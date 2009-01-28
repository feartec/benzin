import sys, struct, re
from ethyl import *

f = open(sys.argv[1], 'rb')
rlyt = f.read()
f.close()

assert rlyt[0:8] == 'RLYT\xfe\xff\x00\x08'
offs, = struct.unpack('>h', rlyt[0xc:0xe])
ch = iff_to_chunks(rlyt[offs:])
indent = 0
for typ, chunk in ch:
    print '    ' * indent + typ + ' ' + hex(len(chunk))
    vars = None
    if typ in ('pic1', 'pan1', 'bnd1', 'wnd1', 'txt1'):  
        vars, pos = parse_data(chunk, 'pane')
        assert chunk[:pos] == unparse_data(vars, 'pane') # for testing
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
