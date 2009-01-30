import sys, struct, re
from ethyl import *

f = open(sys.argv[1], 'r')
chunks = eval(f.read())
f.close()


rlyt = 'RLYT\xfe\xff\x00\x08\x00\x00\x00\x00' # size later
rlyt += struct.pack('>hh', 0x10, len(chunks))

materials = None

for c in chunks:
    if isinstance(c, basestring):
        typ = c
        chunk = ''
    else:
        typ, vars = c
        if typ == 'lyt1':
            chunk = unparse_data(vars, 'lytheader')
        elif typ == 'grp1':
            vars['numsubs'] = len(vars['subs'])
            chunk = unparse_data(vars, 'group')
            for sub in vars['subs']:
                chunk += unnullterm(sub, 16)
        elif typ in ('txl1', 'fnl1', 'mat1'):
            things = {'txl1': 'textures', 'fnl1': 'files', 'mat1': 'materials'}[typ]
            ls = vars[things]
            chunk = struct.pack('>hh', len(ls), 0)
            strings = ''
            offss = []
            for thing in ls:
                name = untv(thing['name'] if typ != 'mat1' else thing)
                offss.append(len(strings))
                strings += name
            for n, thing in enumerate(ls):
                chunk += struct.pack('>I', offss[n] + (12 if typ == 'mat1' else 0) + (4 if typ == 'mat1' else 8) * len(ls))
                if typ != 'mat1':
                    chunk += struct.pack('>I', thing['unk'])
            chunk += strings
            if typ == 'mat1':
                materials = ls
        elif typ in ('pic1', 'pan1', 'bnd1', 'wnd1', 'txt1'):  
            chunk = unparse_data(vars, 'pane')
            if typ == 'txt1':
                vars['~text.name_offs'] = 0x74
                vars['~text.mat_off'] = materials.index(vars['~text.material'])
                blob = untv(unicode(vars['~text.text']).encode('utf_16_be'))
                vars['~text.len1'] = vars['~text.len2'] = len(blob)
                chunk += unparse_data(vars, 'text', prefix='~text')
                chunk += blob
            elif typ == 'pic1':
                vars['~pic.mat_off'] = materials.index(vars['~pic.material'])
                vars['~pic.num_texcoords'] = len(vars['~pic.texcoords'])
                chunk += unparse_data(vars, 'pic', prefix='~pic')
                for tc in vars['~pic.texcoords']:
                    chunk += struct.pack('>ffffffff', *tc)
            elif typ != 'pan1':
                failz
        else:
            raise Exception('Unhandled type %s' % typ)
    rlyt += typ + struct.pack('>I', len(chunk) + 8) + chunk


rlyt = rlyt[:8] + struct.pack('>I', len(rlyt)) + rlyt[0xc:]


g = open(sys.argv[2], 'wb')
g.write(rlyt)
g.close()