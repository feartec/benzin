import sys, struct, re

f = open(sys.argv[1], 'rb')
rlyt = f.read()
f.close()

from ethyl import formats
for (name, definition) in formats.items():
    definition = re.sub(re.compile('//.*$', re.M), '', definition)
    definition = definition.strip()
    definition = re.sub('\n[\n ]+', '\n', definition)
    df = [[j.strip() for j in i.strip().split()] for i in definition.split('\n')]
    formats[name] = df

def pr_dict(data, start=''):
    for a in sorted(data.keys()):
        b = data[a]
        print '%s%s: %s' % (start, str(a), hex(b) if type(b) in (int, long) else repr(b))
types = {
    'u8': 'B',
    'u16': 'H',
    'u32': 'I',
    'u64': 'Q',
    's8': 'b',
    's16': 'h',
    's32': 'i',
    's64': 'q',
    'float': 'f',
    'f32': 'f',
    'double': 'd',
    'f64': 'd',
    'bool': 'c',
    'char': 'c',
}
def parse_var(pos, typ, chunk):
    if not types.has_key(typ):
        raise Exception('What\'s a %s?' % typ)
    fmt = '>' + types[typ]
    size = struct.calcsize(fmt)
    
    return (pos + size), struct.unpack(fmt, chunk[pos:pos + size])[0]
def nullterm(str_plus):
    z = str_plus.find('\0')
    if z != -1:
        return str_plus[:z]
    else:
        return str_plus
def parse_data(chunk, definition, prefix=None, start=0):
    df = formats[definition][:]
    #print df, repr(definition)
    print df
    etc = df[-1][0][:3] == 'etc'
    etcn = etc and df.pop() == ['etc!']

    pos = start
    ret = {}
    for typ, name in df:
        m = re.match('^([^\[]+)\[([a-zA-Z0-9]+)\]$', name)
        if m:
            name = m.group(1)
            size = eval(m.group(2))
            if typ == 'char':
                var = chunk[pos:pos+size].rstrip(chr(0))
                pos += size
            else:
                var = []
                for n in xrange(size):
                    pos, varn = parse_var(pos, typ, chunk)
                    var.append(varn)
        else:
            pos, var = parse_var(pos, typ, chunk)
        if ret.has_key(name):
            try:
                ret[name].append(var)
            except AttributeError:
                ret[name] = [ret[name], var]
        else:
            ret[name] = var
    #print hex(pos)
    if pos > len(chunk):
        raise Exception('too much!')
    if pos < len(chunk):
        if etc and not etcn:
            ret['etc'] = repr(chunk[pos:])
        elif not etc:
            raise Exception('Incomplete handling...')
    if prefix is not None:
        ret2 = {}
        for (k, v) in ret.items():
            ret2[prefix + '.' + k] = v
        ret = ret2
    return (ret, pos) if etcn else ret
        
def iff_to_chunks(z):
    pos = 0
    chunks = []
    while pos < len(z):
        cc = z[pos:pos+4]
        length, = struct.unpack('>I', z[pos+4:pos+8])
        #print cc
        #print hex(length)
        data = z[pos+8:pos+length]
        pos += length
        chunks.append((cc, data))
    return chunks
assert rlyt[0:8] == 'RLYT\xfe\xff\x00\x08'
offs, = struct.unpack('>h', rlyt[0xc:0xe])
ch = iff_to_chunks(rlyt[offs:])
indent = 0
for typ, chunk in ch:
    print '    ' * indent + typ + ' ' + hex(len(chunk))
    vars = None
    if typ in ('pic1', 'pan1', 'bnd1', 'wnd1', 'txt1'):  
        vars, pos = parse_data(chunk, 'pane')
        if typ == 'txt1':
            vars2, pos = parse_data(chunk[pos:], 'text', '~text')
            
            txt = chunk[vars2['~text.name_offs'] - 8:]
            vars2['~text.text'] = unicode(txt, 'utf_16_be').rstrip('\x00')
            vars.update(vars2)
        elif typ == 'pic1':
            vars2, pos = parse_data(chunk, 'pic', prefix='~pic', start=pos)

            vars2['~pic.texcoords'] = []
            for n in xrange(vars2['~pic.num_texcoords']):
                vars2['~pic.texcoords'].append(struct.unpack('>ffffffff', chunk[pos:pos+0x20]))
                pos += 0x20
            assert pos == len(chunk)
            vars.update(vars2)
    elif typ == 'lyt1':
        vars = parse_data(chunk, 'lytheader')
    elif typ == 'grp1':
        vars, pos = parse_data(chunk, 'group')
        vars['subs'] = []
        for n in xrange(vars['numsubs']):
            vars['subs'].append(nullterm(chunk[pos:pos+16]))
            pos += 16
    elif typ in ('txl1', 'fnl1', 'mat1'):
        things = {'txl1': 'textures', 'fnl1': 'files', 'mat1': 'materials'}[typ]
        vars, pos = parse_data(chunk, 'numoffs')
        vars[things] = []
        pos += vars['offs']
        del vars['offs'] # unneeded
        for n in xrange(vars['num']):
            if typ == 'mat1':
                offset, = struct.unpack('>I', chunk[pos:pos+4])
                pos += 4
            else:
                offset, unk = struct.unpack('>II', chunk[pos:pos+8])
                pos += 8
            name = nullterm(chunk[offset - 8:])
            vars[things].append(name if typ == 'mat1' else {'name': name, 'unk': unk})
        if typ == 'mat1':
            materials = vars['materials']
    elif typ in ('grs1', 'pas1'):
        indent += 1
    elif typ in ('gre1', 'pae1'):
        indent -= 1
    else:
        print repr(chunk)
    if vars: pr_dict(vars, '    ' * indent + '    ')
#print hex(len(ch))
