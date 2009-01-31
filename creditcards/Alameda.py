from pyglet import clock, window
from pyglet.gl import *
from pyglet.image import *


import sys
sys.path.append('../Common')
from Struct import Struct
import pywii as wii

def LZ77Decompress(data):
	inp = 0
	dlen = len(data)
	data += '\0' * 16
	
	ret = []
	
	while inp < dlen:
		bitmask = ord(data[inp])
		inp += 1
		
		for i in xrange(8):
			if bitmask & 0x80:
				rep = ord(data[inp])
				repLength = (rep >> 4) + 3
				inp += 1
				repOff = ord(data[inp]) | ((rep & 0x0F) << 8)
				inp += 1
				
				assert repOff <= len(ret)
				
				while repLength > 0:
					ret.append(ret[-repOff - 1])
					repLength -= 1
			else:
				ret.append(data[inp])
				inp += 1
			
			bitmask <<= 1
	
	return ''.join(ret)

class U8(object):
	class U8Header(Struct):
		__endian__ = Struct.BE
		def __format__(self):
			self.Tag = Struct.string(4)
			self.RootNode = Struct.uint32
			self.HeaderSize = Struct.uint32
			self.DataOffset = Struct.uint32
			self.Zeroes = Struct.uint8[0x10]
	
	class U8Node(Struct):
		__endian__ = Struct.BE
		def __format__(self):
			self.Type = Struct.uint16
			self.NameOffset = Struct.uint16
			self.DataOffset = Struct.uint32
			self.Size = Struct.uint32
	
	def __init__(self, data=None):
		self.Files = {}
		
		if data != None:
			self.Unpack(data)
	
	def Unpack(self, data):
		pos = 0
		u8 = self.U8Header()
		u8.unpack(data[pos:pos+len(u8)])
		pos += len(u8)
		
		assert u8.Tag == 'U\xAA8-'
		pos += u8.RootNode - 0x20
		
		root = self.U8Node()
		root.unpack(data[pos:pos+len(root)])
		pos += len(root)
		
		children = []
		for i in xrange(root.Size - 1):
			child = self.U8Node()
			child.unpack(data[pos:pos+len(child)])
			pos += len(child)
			children.append(child)
		
		stringTable = data[pos:pos + u8.DataOffset - len(u8) - root.Size * len(root)]
		pos += len(stringTable)
		
		currentOffset = u8.DataOffset
		
		path = ['.']
		pathDepth = [root.Size - 1]
		for offset,child in enumerate(children):
			name = stringTable[child.NameOffset:].split('\0', 1)[0]
			if child.Type == 0x0100:
				path.append(name)
				pathDepth.append(child.Size-offset-1)
			elif child.Type == 0:
				name = '/'.join(path + [name])
				
				if currentOffset < child.DataOffset:
					diff = child.DataOffset - currentOffset
					assert diff <= 32
					
					pos += diff
					currentOffset += diff
				
				currentOffset += child.Size
				
				self.Files[name] = data[pos:pos+child.Size]
				pos += child.Size
			
			pathDepth[-1] -= 1
			if pathDepth[-1] == 0:
				path = path[:-1]
				pathDepth = pathDepth[:-1]

class IMD5Header(Struct):
	__endian__ = Struct.BE
	def __format__(self):
		self.Tag = Struct.string(4)
		self.Size = Struct.uint32
		self.Zeroes = Struct.uint8[8]
		self.MD5 = Struct.uint8[0x10]

def IMD5(data):
	imd5 = IMD5Header()
	imd5.unpack(data[:len(imd5)])
	assert imd5.Tag == 'IMD5'
	pos = len(imd5)
	
	if data[pos:pos+4] == 'LZ77':
		return LZ77Decompress(data[pos+8:])
	else:
		return data[pos:]

class TPL(object):
	class TPLHeader(Struct):
		__endian__ = Struct.BE
		def __format__(self):
			self.Magic = Struct.string(4)
			self.Count = Struct.uint32
			self.Size = Struct.uint32
	
	class TexOffsets(Struct):
		__endian__ = Struct.BE
		def __format__(self):
			self.HeaderOff = Struct.uint32
			self.PaletteOff = Struct.uint32
	
	class TexHeader(Struct):
		__endian__ = Struct.BE
		def __format__(self):
			self.Size = Struct.uint16[2]
			self.Format = Struct.uint32
			self.DataOff = Struct.uint32
			self.Wrap = Struct.uint32[2]
			self.Filter = Struct.uint32[2]
			self.LODBias = Struct.float
			self.EdgeLOD = Struct.uint8
			self.LOD = Struct.uint8[2]
			self.Unpacked = Struct.uint8
	
	class PalHeader(Struct):
		__endian__ = Struct.BE
		def __format__(self):
			self.Count = Struct.uint16
			self.Unpacked = Struct.uint8
			self.Pad = Struct.uint8
			self.Format = Struct.uint32
			self.DataOff = Struct.uint32
	
	def __init__(self, data=None):
		self.Textures = []
		
		if data != None:
			self.Unpack(data)
	
	def Unpack(self, data):
		header = self.TPLHeader()
		header.unpack(data[:len(header)])
		pos = len(header)
		
		assert header.Magic == '\x00\x20\xAF\x30'
		assert header.Size == 0xc
		
		for i in xrange(header.Count):
			offs = self.TexOffsets()
			offs.unpack(data[pos:pos+len(offs)])
			pos += len(offs)
			
			self.Textures.append(self.ParseTexture(data, offs))
	
	def ParseTexture(self, data, offs):
		texHeader = self.TexHeader()
		texHeader.unpack(data[offs.HeaderOff:offs.HeaderOff+len(texHeader)])
		format = texHeader.Format
		self.format = format
		rgba = None
		if format == 0:
			rgba = self.I4(data[texHeader.DataOff:], texHeader.Size)
		elif format == 1:
			rgba = self.I8(data[texHeader.DataOff:], texHeader.Size)
		elif format == 2:
			rgba = self.IA4(data[texHeader.DataOff:], texHeader.Size)
		elif format == 4:
		    rgba = self.RGB565(data[texHeader.DataOff:], texHeader.Size)
		elif format == 5:
			rgba = self.RGB5A3(data[texHeader.DataOff:], texHeader.Size)
		elif format == 14:
			rgba = self.S3TC(data[texHeader.DataOff:], texHeader.Size)
		else:
			print 'Unknown texture format', format
		
		if rgba == None:
			rgba = '\0\0\0\0' * texHeader.Size[0] * texHeader.Size[1]
		
		image = ImageData(texHeader.Size[1], texHeader.Size[0], 'RGBA', rgba)
		print format
		return image
	
	def I4(self, data, (y, x)):
		out = [0 for i in xrange(x * y)]
		outp = 0
		inp = 0
		for i in xrange(0, y, 8):
			for j in xrange(0, x, 8):
				ofs = 0
				for k in xrange(8):
					off = min(x - j, 8)
					for sub in xrange(0, off, 2):
						texel = ord(data[inp])
						high, low = texel >> 4, texel & 0xF
						out[outp + ofs + sub] = (high << 4) | (high << 20) | (high << 12) | 0xFF<<24
						
						if sub + 1 < off:
							out[outp + ofs + sub + 1] = (low << 4) | (low << 20) | (low << 12) | 0xFF<<24
						
						inp += 1
					
					ofs += x
					inp += (8 - off) / 2
				outp += off
			outp += x * 7
		
		return ''.join(Struct.uint32(p) for p in out)
	
	def I8(self, data, (y, x)):
		out = [0 for i in xrange(x * y)]
		outp = 0
		inp = 0
		for i in xrange(0, y, 4):
			for j in xrange(0, x, 4):
				ofs = 0
				for k in xrange(4):
					off = min(x - j, 4)
					for sub in xrange(off):
						texel = ord(data[inp])
						out[outp + ofs + sub] = (texel << 24) | (texel << 16) | (texel << 8) | 0xFF
						inp += 1
					
					ofs += x
					inp += 4 - off * 2
				outp += off
			outp += x * 3
		
		return ''.join(Struct.uint32(p) for p in out)
		
	def RGB565(self, data, (y, x)):
		out = [0 for i in xrange(x * y)]
		outp = 0
		inp = 0
		for i in xrange(0, y, 4):
			for j in xrange(0, x, 4):
				ofs = 0
				for k in xrange(4):
					off = min(x - j, 4)
					for sub in xrange(off):
						texel = ord(data[inp])
						#out[outp + ofs + sub] = (texel << 24) | (texel << 16) | (texel << 8) | 0xFF
						b = (texel&0x1f)<<3
						g = ((texel>>5)&0x3f)<<2
						r = ((texel>>11)&0x1f)<<3
						b |= b>>5
						g |= g>>6
						r |= r>>5
						if (outp + ofs + sub) < (x*y):
							out[outp + ofs + sub] = (0xff<<24) | (b<<16) | (g<<8) | r
						inp += 1
					
					ofs += x
					inp += 4 - off * 2
				outp += off
			outp += x * 3
		
		return ''.join(Struct.uint32(p) for p in out)
	
	def IA4(self, data, (y, x)):
		out = [0 for i in xrange(x * y)]
		outp = 0
		inp = 0
		for i in xrange(0, y, 4):
			for j in xrange(0, x, 8):
				ofs = 0
				for k in xrange(4):
					off = min(x - j, 8)
					for sub in xrange(off):
						texel = ord(data[inp])
						alpha, inte = texel >> 4, texel & 0xF
						if (outp + ofs + sub) < (x*y):
							out[outp + ofs + sub] = (inte << 4) | (inte << 12) | (inte << 20) | (alpha << 28)
						inp += 1
					
					ofs += x
					inp += 8 - off
				outp += off
			outp += x * 3
		
		return ''.join(Struct.uint32(p) for p in out)
	
	def RGB5A3(self, data, (y, x)):
		out = [0 for i in xrange(x * y)]
		outp = 0
		inp = 0
		for i in xrange(0, y, 4):
			for j in xrange(0, x, 4):
				ofs = 0
				for k in xrange(4):
					off = min(x - j, 4)
					for sub in xrange(off):
						texel = Struct.uint16(data[inp:inp + 2], endian='>')
						if texel & 0x8000:
							p  = ((texel >> 10) & 0x1F) << 3
							p |= ((texel >> 5) & 0x1F) << 11
							p |= ( texel       & 0x1F) << 19
							p |= 0xFF<<24
						else:
							p  = ((texel >> 12) & 0x07) << 29
							p |= ((texel >>  8) & 0x0F) << 4
							p |= ((texel >>  4) & 0x0F) << 12
							p |=  (texel        & 0x0F) << 20
						out[outp + ofs + sub] = p
						inp += 2
					
					ofs += x
					inp += (4 - off) * 2
				outp += off
			outp += x * 3
		
		return ''.join(Struct.uint32(p) for p in out)

	def unpack_rgb565(self,texel):
		b = (texel&0x1f)<<3
		g = ((texel>>5)&0x3f)<<2
		r = ((texel>>11)&0x1f)<<3
		b |= b>>5
		g |= g>>6
		r |= r>>5
		return (0xff<<24) | (b<<16) | (g<<8) | r
	def icolor(self,a,b,fa,fb,fc):
		c = 0
		for i in xrange(0,32,8):
			xa = (a>>i)&0xff
			xb = (b>>i)&0xff
			xc = min(255,max(0,int((xa*fa + xb*fb)/fc)))
			c |= xc<<i
		return c
	
	def S3TC(self, data, (y, x)):
		out = [0 for i in xrange(x * y)]
		TILE_WIDTH = 8
		TILE_HEIGHT = 8
		inp = 0
		outp = 0
		for i in xrange(0, y, TILE_HEIGHT):
			for j in xrange(0, x, TILE_WIDTH):
				maxw = min(x - j,TILE_WIDTH)
				for k in xrange(2):
					for l in xrange(2):
						rgb = [0,0,0,0]
						texel1 = Struct.uint16(data[inp:inp + 2], endian='>')
						texel2 = Struct.uint16(data[inp + 2:inp + 4], endian='>')
						rgb[0] = self.unpack_rgb565(texel1)
						rgb[1] = self.unpack_rgb565(texel2)
						
						if texel1 > texel2:
							rgb[2] = self.icolor (rgb[0], rgb[1], 2, 1, 3) | 0xff000000
							rgb[3] = self.icolor (rgb[1], rgb[0], 2, 1, 3) | 0xff000000
						else:
							rgb[2] = self.icolor (rgb[0], rgb[1], 0.5, 0.5, 1) | 0xff000000
							rgb[3] = 0
						
						# color selection (00, 01, 10, 11)
						cm = map(ord,data[inp+4:inp+8])
						ofs = l*4
						for n in range(4):
							if (ofs + outp)<(x*y):
								# one row (4 texels)
								if maxw > (0 + l*4):
									out[ofs + 0 + outp] = rgb[(cm[n] & 0xc0) >> 6];
								if maxw > (1 + l*4):
									out[ofs + 1 + outp] = rgb[(cm[n] & 0x30) >> 4];
								if maxw > (2 + l*4):
									out[ofs + 2 + outp] = rgb[(cm[n] & 0x0c) >> 2];
								if maxw > (3 + l*4):
									out[ofs + 3 + outp] = rgb[(cm[n] & 0x03) >> 0];
							ofs += x
						inp += 8
					outp += x * 4
				outp += maxw - x * 8
			outp += x * (TILE_HEIGHT - 1)
		
		return ''.join(Struct.uint32(p) for p in out)

class ValueFader(object):
	def __init__(self, Type, parent, triplets, loop):
		self.Type = Type
		self.Parent = parent
		self.Triplets = triplets
		self.Loop = loop
		self.Reset()
	def Reset(self):
		self.Pos = 0
		self.Value = self.Triplets[0][1]
		self.Framecount = 0
		self.Frame = 0
		self.Complete = False
	def Do(self):
		cframe, cval, cacc = self.Triplets[self.Pos]
		if self.Frame >= cframe and self.Pos < len(self.Triplets)-1:
			nframe = cframe
			nval = cval
			nacc = cacc
			while nframe == cframe and self.Pos < len(self.Triplets)-1:
				cframe, cval, cacc = self.Triplets[self.Pos]
				self.Value = cval
				self.Pos += 1
				if self.Pos == len(self.Triplets) - 1:
					print 'ZOMG'
				nframe, nval, nacc = self.Triplets[self.Pos]
			
			print "Starting ValueFader(%s) for %s:"%(self.Type,self.Parent.Name),cframe,cval,cacc,nframe,nval,nacc
			self.Framecount = nframe - cframe
			if self.Framecount > 0:
				self.dValue = (nval - cval)/self.Framecount
			else:
				self.Value = nval
		if self.Framecount > 0:
			self.Framecount -= 1
			self.Value += self.dValue
		elif self.Pos == len(self.Triplets) - 1:
			if self.Loop:
				self.Reset()
			else:
				self.Complete = True
		
		self.Frame += 1
		return self.Value
		

class Object(object):
	def __init__(self, name):
		self.Frame = 0
		self.rlvc = None
		self.rlpa = None
		self.rlmc = None
		self.rlts = None
		self.Name = name
		self.Alpha = 255
		self.Animations = 0
	def RLPA(self, triplets, loop):
		if len(triplets) > 1:
			if self.rlpa != None:
				self.Animations -= 1
			self.rlpa = ValueFader("RLPA", self, triplets, loop)
			self.Animations += 1
	def RLVC(self, triplets, loop):
		if len(triplets) > 1:
			if self.rlvc != None:
				self.Animations -= 1
			self.rlvc = ValueFader("RLVC", self, triplets, loop)
			self.Animations += 1
	def RLTS(self, triplets, loop):
		if len(triplets) > 1:
			if self.rlts != None:
				self.Animations -= 1
			self.rlts = ValueFader("RLTS", self, triplets, loop)
			self.Animations += 1
	def RLMC(self, triplets, loop):
		if len(triplets) > 1:
			if self.rlmc != None:
				self.Animations -= 1
			self.rlmc = ValueFader("RLMC", self, triplets, loop)
			self.Animations += 1
	def Animate(self):
		if self.rlpa is not None:
			self.Coords[1] = self.rlpa.Do()
			if self.rlpa.Complete:
				self.rlpa = None
				self.Animations -= 1
		if self.rlvc is not None:
			self.Alpha = self.rlvc.Do()
			if self.rlvc.Complete:
				self.rlvc = None
				self.Animations -= 1
		if self.rlts is not None:
			self.Coords[1] = self.rlts.Do()
			if self.rlts.Complete:
				self.rlts = None
				self.Animations -= 1
		if self.rlmc is not None:
			self.Alpha = self.rlmc.Do()
			if self.rlmc.Complete:
				self.rlmc = None
				self.Animations -= 1
	def __str__(self):
		return self.Name
	
	def AnimDone(self):
		return self.Animations == 0

class Pane(Object):
	def __init__(self, name, coords=None):
		Object.__init__(self, name)
		self.Coords = coords
		self.Children = []
		self.Enabled = True
	def Add(self, child):
		self.Children.append(child)
	
	def AnimDone(self):
		if not Object.AnimDone(self):
			return False
		
		for child in self.Children:
			if not child.AnimDone():
				return False
		
		return True

class Picture(Object):
	def __init__(self, name, material, coords):
		Object.__init__(self, name)
		self.Material = material
		self.Coords = coords
		self.Enabled = True

class Brlyt(object):
	class BrlytHeader(Struct):
		__endian__ = Struct.BE
		def __format__(self):
			self.Magic = Struct.string(4)
			self.Unk = Struct.uint32
			self.Size = Struct.uint32
			self.UnkCount = Struct.uint16
			self.AtomCount = Struct.uint16
	
	class BrlytAtom(Struct):
		__endian__ = Struct.BE
		def __format__(self):
			self.FourCC = Struct.string(4)
			self.Size = Struct.uint32
	
	def __init__(self, archive, data):
		self.Archive = archive
		self.Textures = []
		self.Materials = []
		self.RootPane = None
		self.RootPaneName = None
		self.Objects = {}
		self.PanePath = []
		self.PaneId = 0
		self.Language = "ENG"
		
		if data != None:
			self.Unpack(data)
	
	def Unpack(self, data):
		pos = 0
		header = self.BrlytHeader()
		header.unpack(data[:len(header)])
		pos += len(header)
		
		assert header.Magic == 'RLYT'
		
		for i in xrange(header.AtomCount):
			atom = self.BrlytAtom()
			atom.unpack(data[pos:pos+len(atom)])
			
			atomdata = data[pos:pos+atom.Size]
			if atom.FourCC == 'txl1':
				self.TXL1(atomdata)
			elif atom.FourCC == 'mat1':
				self.MAT1(atomdata)
			elif atom.FourCC == 'pan1':
				self.PAN1(atomdata)
			elif atom.FourCC == 'pas1':
				self.PAS1(atomdata)
			elif atom.FourCC == 'pae1':
				self.PAE1(atomdata)
			elif atom.FourCC == 'pic1':
				self.PIC1(atomdata)
			elif atom.FourCC == "grp1":
				self.GRP1(atomdata)
			else:
				print "Unknown FOURCC:",atom.FourCC
				wii.chexdump(atomdata)
			
			pos += atom.Size
	
	def TXL1(self, data):
		pos = 8
		texCount = Struct.uint16(data[pos:pos+2], endian='>')
		pos += 4
		
		for i in xrange(texCount):
			fnOff = Struct.uint32(data[pos:pos+4], endian='>')
			pos += 8
			
			fn = data[fnOff+0xC:].split('\0', 1)[0]
			tex = TPL(self.Archive.Files['./arc/timg/' + fn]).Textures[0]
			self.Textures.append((fn, tex))
			print fn
	
	def ApplyMask(self, image, mask):
		print "Making mask:",image,mask
		print image.width,image.height,mask.width,mask.height
		if image.height != mask.height or image.width != mask.width:
			raise ValueError("Mask dimensions must be equal to mask dimensions")
		newdata = [0 for x in xrange(image.height * image.width * 4)]
		
		for pix in xrange(image.height * image.width):
			newdata[pix*4 + 0] = image.data[pix*4 + 0]
			newdata[pix*4 + 1] = image.data[pix*4 + 1]
			newdata[pix*4 + 2] = image.data[pix*4 + 2]
			newdata[pix*4 + 3] = mask.data[pix*4 + 0]
		
		return  ImageData(image.width, image.height, 'RGBA', ''.join(newdata))
	
	def MAT1(self, data):
		pos = 8
		matCount = Struct.uint16(data[pos:pos+2], endian='>')
		pos += 4
		
		for i in xrange(matCount):
			nameOff = Struct.uint32(data[pos:pos+4], endian='>')
			pos += 4
			
			name = data[nameOff:].split('\0', 1)[0]
			wii.chexdump(data[nameOff:nameOff+0x60])
			texid = Struct.uint16(data[nameOff + 0x40:nameOff + 0x42], endian='>')
			texid2 = Struct.uint16(data[nameOff + 0x44:nameOff + 0x46], endian='>')
			colorA = [Struct.uint16(data[nameOff + 0x1c + x:nameOff + 0x20 + x], endian='>')/255.0 for x in range(0,8,2)]
			colorB = [Struct.uint16(data[nameOff + 0x22 + x:nameOff + 0x24 + x], endian='>')/255.0 for x in range(0,8,2)]
			flags = Struct.uint32(data[nameOff + 0x3c:nameOff + 0x40], endian='>')
			
			numtex = 1 # Hardcoded for her pleasure #(flags>>8) & 0xf #guess
			if numtex == 1:
				img = self.Textures[texid]
				tex = img[1].create_texture(Texture)
				print 'Material 0x%02x (%s) maps to texture %s' % (i, name, img[0])
			elif numtex == 2:
				img = self.Textures[texid]
				alpha = self.Textures[texid2]
				combo = self.ApplyMask(img[1],alpha[1])
				tex = combo.create_texture(Texture)
				print 'Material 0x%02x (%s) maps to texture %s and mask %s' % (i, name, img[0], alpha[0])
			else:
				print "Bad num textures: %d"%numtex
			
			self.Materials.append((name, tex, (colorA,colorB)))
			
	
	def PAN1(self, data):
		wii.chexdump(data)
		name =  data[0xC:].split('\0', 1)[0]
		x = Struct.float(data[0x24:0x28], endian='>')
		y = Struct.float(data[0x28:0x2C], endian='>')
		a = Struct.float(data[0x3C:0x40], endian='>')
		b = Struct.float(data[0x40:0x44], endian='>')
		xs = Struct.float(data[0x44:0x48], endian='>')
		ys = Struct.float(data[0x48:0x4C], endian='>')
		coords = [x, y, xs, ys]
		print 'Pane %s:' % name, coords+[a,b]
		self.CurPane = Pane(name, coords)
	
	def PAS1(self, data):
		if self.CurPane is None:
			self.CurPane = Pane("_pane%d" % self.PaneId)
			self.PaneId += 1
		if self.RootPane is None:
			self.RootPane = self.CurPane
		else:
			assert len(self.PanePath) != 0
			self.PanePath[-1].Add(self.CurPane)
		self.PanePath.append(self.CurPane)
		self.Objects[self.CurPane.Name] = self.CurPane
		print "Pane start:",'.'.join(map(str,self.PanePath))
		self.CurPane = None
	
	def PAE1(self, data):
		print "Pane end:",'.'.join(map(str,self.PanePath))
		self.PanePath = self.PanePath[:-1]
	
	def PIC1(self, data):
		wii.chexdump(data)
		name = data[0xC:].split('\0', 1)[0]
		mat = Struct.uint16(data[0x5C:0x5E], endian='>')
		mat = self.Materials[mat]
		xs = Struct.float(data[0x44:0x48], endian='>')
		ys = Struct.float(data[0x48:0x4C], endian='>')
		x = Struct.float(data[0x24:0x28], endian='>')
		y = Struct.float(data[0x28:0x2C], endian='>')
		print 'Picture %s maps to material %s' % (name, mat[0])
		print '\t%fx%f (%f, %f)' % (xs, ys, x, y)
		
		p=Picture(name, mat, [x, y, xs, ys])
		self.PanePath[-1].Add(p)
		self.Objects[name] = p
	def GRP1(self, data):
		wii.chexdump(data)
		if len(data) < 0x1c:
			pass
		lang = data[0x8:0x18].split('\0', 1)[0]
		nitems = Struct.uint16(data[0x18:0x1a], endian='>')
		p = 0x1c
		items = []
		for i in xrange(nitems):
			items.append(data[p:].split('\0', 1)[0])
			p += 0x10
		for i in items:
			if lang != self.Language:
				self.Objects[i].Enabled = False
			else:
				self.Objects[i].Enabled = True

class Brlan(object):
	class BrlanHeader(Struct):
		__endian__ = Struct.BE
		def __format__(self):
			self.Magic = Struct.string(4)
			self.Unk = Struct.uint32
			self.Size = Struct.uint32
			self.AnimCount = Struct.uint16
			self.AtomCount = Struct.uint16
	
	class BrlanAtom(Struct):
		__endian__ = Struct.BE
		def __format__(self):
			self.FourCC = Struct.string(4)
			self.Size = Struct.uint32
	
	def __init__(self, data, brlyt, loop=False):
		self.Loop = loop
		self.brlyt = brlyt
		if data != None:
			self.Unpack(data)
	
	def Unpack(self, data):
		header = self.BrlanHeader()
		header.unpack(data[:len(header)])
		pos = len(header)
		
		assert header.Magic == 'RLAN'
		
		for i in xrange(header.AtomCount):
			atom = self.BrlanAtom()
			atom.unpack(data[pos:pos+len(atom)])
			atomdata = data[pos:pos+atom.Size]
			pos += atom.Size
			
			if atom.FourCC == 'pai1':
				self.PAI1(atomdata)
	
	class PAI1Header(Struct):
		__endian__ = Struct.BE
		def __format__(self):
			self.UnkOff = Struct.uint16
			self.Unk = Struct.uint16
			self.Count = Struct.uint32
			self.Off = Struct.uint32
	
	def PAI1(self, data):
		pos = 8
		header = self.PAI1Header()
		header.unpack(data[pos:pos+len(header)])
		pos = header.Off
		
		for i in xrange(header.Count):
			off = nameOff = Struct.uint32(data[pos:pos+4], endian='>')
			pos += 4
			
			name = data[off:].split('\0', 1)[0]
			
			off += 0x14
			numAtoms = ord(data[off])
			off += 4
			
			print name
			
			for i in xrange(numAtoms):
				sub = Struct.uint32(data[off:off+4], endian='>')
				off += 4
				fourcc = data[nameOff+sub:nameOff+sub+4]
				
				#if fourcc == 'RLTS': 
				#	print 'RLTS unknown'
				#else:
				self.Atom(name, fourcc, data[nameOff+sub:])
	
	def Atom(self, name, fourcc, data):
		print '\t' + fourcc
		
		count = Struct.uint16(data[0x10:0x12], endian='>')
		
		# file('rlat/%s.%s' % (name, fourcc), 'wb').write(data[:0x18])
		triplets = []
		
		# Calculate offset to triplets (added by dasda).
		off = Struct.uint16(data[4:6], endian='<') * 8 + 16;
		
		for i in xrange(count):
			af = Struct.float(data[off:off+4], endian='>')
			bf = Struct.float(data[off+4:off+8], endian='>')
			cf = Struct.float(data[off+8:off+0xC], endian='>')
			
			triplets.append((af, bf, cf))
			off += 0xC
		
		if fourcc == 'RLPA':
			self.brlyt.Objects[name].RLPA(triplets, self.Loop)
		elif fourcc == 'RLVC':
			self.brlyt.Objects[name].RLVC(triplets, self.Loop)
		elif fourcc == 'RLTS':
			self.brlyt.Objects[name].RLTS(triplets, self.Loop)
		elif fourcc == 'RLMC':
			self.brlyt.Objects[name].RLMC(triplets, self.Loop)
		else:
			for trip in triplets:
				print '\t\tTriplet: %f %f %f' % trip
	
class BannerWindow(window.Window):
	def on_resize(self, width, height):
		glViewport(0, 0, width, height)
		glMatrixMode(GL_PROJECTION)
		glLoadIdentity()
		glOrtho(-width/2, width/2, -height/2, height/2, -1, 1)
		glMatrixMode(GL_MODELVIEW)


class Renderer(object):
	def __init__(self):
		self.Width, self.Height = 640, 480
		self.Window = BannerWindow(self.Width, self.Height)
		self.Window.set_exclusive_mouse(False)
		self.Brlyt = None
		
		glClearColor(0.0, 0.0, 0.0, 0.0)
		glClearDepth(1.0)
		glDepthFunc(GL_LEQUAL)
		glEnable(GL_DEPTH_TEST)
		
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA)
		glEnable(GL_BLEND)
		
		glEnable(GL_TEXTURE_2D)
		
		clock.set_fps_limit(60)
	
	def Render(self, item, wireframe=False):
		item.Animate()
		if not item.Enabled:
			return
		if isinstance(item, Pane):
			if item.Coords is not None:
				x, y, xs, ys = item.Coords
			else:
				x = y = 0
				xs = ys = 1
			glPushMatrix()
			glTranslatef(x, y, 0)
			for child in item.Children:
				self.Render(child,wireframe)
			glPopMatrix()
		elif isinstance(item, Picture):
			
			mat = item.Material
			texture, colors = mat[1:]
			x, y, xs, ys = item.Coords
			xc, yc = x-(xs/2),y-(ys/2)
			
			if not wireframe:
				glBindTexture(texture.target, texture.id)
				col = list(colors[0])
				col[3] = col[3] * item.Alpha / 255.0
				glColor4f(*col)
				texture.blit(xc,yc+ys,0,xs,-ys)
				glBindTexture(texture.target, 0)
			else:
				
				glColor3f(1,0,0)
				glBegin(GL_LINE_STRIP)
				glVertex2f(xc,yc)
				glVertex2f(xc+xs,yc)
				glVertex2f(xc+xs,yc+ys)
				glVertex2f(xc,yc+ys)
				glVertex2f(xc,yc)
				glColor3f(1,1,1)
				glEnd()
				pass
	
	def MainLoop(self, loop):
		while not self.Window.has_exit:
			self.Window.dispatch_events()
			self.Window.clear()
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
			glLoadIdentity()
			glColor4f(1.0, 1.0, 1.0, 1.0)
			
			self.Render(self.Brlyt.RootPane,False)
			#self.Render(self.Brlyt.RootPane,True)
			
			self.Window.flip()
			
			if not loop and self.Brlyt.RootPane.AnimDone():
				return True

class Alameda(object):
	class IMETHeader(Struct):
		__endian__ = Struct.BE
		def __format__(self):
			self.Zeroes = Struct.uint8[0x40]
			self.IMET = Struct.string(4)
			self.Fixed = Struct.uint8[8]
			self.Sizes = Struct.uint32[3]
			self.Flag1 = Struct.uint32
			self.Names = Struct.string(0x2A<<1, encoding='utf-16-be', stripNulls=True)[7]
			self.Zeroes = Struct.uint8[0x348]
			self.Crypto = Struct.uint8[0x10]
	
	def __init__(self, fn, type='icon'):
		renderer = Renderer()
		
		fp = file(fn, 'rb')
		
		imet = self.IMETHeader()
		imet.unpack(fp.read(len(imet)))
		if imet.IMET != 'IMET':
			fp.seek(0x40)
			imet.unpack(fp.read(len(imet)))
			assert imet.IMET == 'IMET'
		
		print 'English title: %s' % imet.Names[1]
		
		root = U8(fp.read())
		if type == 'icon':
			banner = U8(IMD5(root.Files['./meta/icon.bin']))
			brlyt = Brlyt(banner, banner.Files['./arc/blyt/icon.brlyt'])
			startAnim = Brlan(banner.Files['./arc/anim/icon.brlan'], brlyt, loop=True)
			loop = True
		else:
			banner = U8(IMD5(root.Files['./meta/banner.bin']))
			brlyt = Brlyt(banner, banner.Files['./arc/blyt/banner.brlyt'])
			startAnim = Brlan(banner.Files['./arc/anim/banner_Start.brlan'], brlyt, loop=False)
			loop = False
		
		renderer.Brlyt = brlyt
		if renderer.MainLoop(loop) and type == 'banner':
			loopAnim = Brlan(banner.Files['./arc/anim/banner_Loop.brlan'], brlyt, loop=True)
			renderer.MainLoop(True)

if __name__=='__main__':
	Alameda(*sys.argv[1:])
