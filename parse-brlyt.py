import sys, struct, re
from ethyl import *

f = open(sys.argv[1], 'rb')
rlyt = f.read()
f.close()

assert rlyt[0:8] == 'RLYT\xfe\xff\x00\x08'
offs, nel = struct.unpack('>hh', rlyt[0xc:0x10])
ch = iff_to_chunks(rlyt[offs:])

assert len(ch) == nel

indent = 0
for typ, chunk in ch:
    print '    ' * indent + typ + ' ' + hex(len(chunk))
    vars = None
    if typ in ('pic1', 'pan1', 'bnd1', 'wnd1', 'txt1'):  
        vars, pos = parse_data(chunk, 'pane')
        assert chunk[:pos] == unparse_data(vars, 'pane') # for testing
        if typ == 'txt1':
            vars2, pos = parse_data(chunk[pos:], 'text', '~text')
            os = vars2['~text.name_offs'] - 8
            txt = chunk[os:os + vars2['~text.len2']]
            vars2['~text.text'] = unicode(txt, 'utf_16_be').rstrip('\x00')
            vars2['~text.material'] = materials[vars2['~text.mat_off']]
            vars.update(vars2)
            #print vars['~text.cnt2'], len(vars2['~text.text']), len(txt), repr(txt)
        elif typ == 'pic1':
            vars2, pos = parse_data(chunk, 'pic', prefix='~pic', start=pos)
            vars2['~pic.material'] = materials[vars2['~pic.mat_off']]
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
    elif typ in ('txl1', 'fnl1'):
        things = {'txl1': 'textures', 'fnl1': 'files'}[typ]
        vars, pos = parse_data(chunk, 'numoffs')
        vars[things] = []
        pos += vars['offs']
        bpos = pos
        del vars['offs'] # unneeded
        for n in xrange(vars['num']):
            offset, unk = struct.unpack('>II', chunk[pos:pos+8])
            name = nullterm(chunk[offset + bpos:])
            vars[things].append({'name': name, 'unk': unk})
            pos += 8
    elif typ == 'mat1':
        vars, pos = parse_data(chunk, 'numoffs')
        vars['materials'] = []
        pos += vars['offs']
        bpos = pos
        del vars['offs']
        materials = []
        for n in xrange(vars['num']):
            offset, = struct.unpack('>I', chunk[pos:pos+4])
            mat, mpos = parse_data(chunk, 'material', start = offset - 8)
            #vars['materials'].append(mat)
            flags = mat['flags']
            materials.append(mat['name'])
            assert mpos == 0x40 + offset - 8
        
            
            #print hex(struct.unpack('>I', chunk[mpos:mpos+4])[0])
            
            # I believe material is:
            # 0x40, followed by
            # 4 * flags[28-31], followed by
            
            
            mat['texref'], mpos = get_array(chunk, mpos, bit_extract(flags, 28, 31), 4, 'texref')
            
            # 0x14 * flags[24-27], followed by
            
            mat['ua2'], mpos = get_array(chunk, mpos, bit_extract(flags, 24, 27), 0x14, 'ua2')
            
            # 4*flags[20-23], followed by
            
            mat['ua3'], mpos = get_array(chunk, mpos, bit_extract(flags, 20, 23), 4, '4b')
            
            # 4 * flags[6]
            
            mat['ua4'], mpos = get_opt(chunk, mpos, bit_extract(flags, 6), 4, '4b')
            
            # 4 * flags[4]
            
            mat['ua5'], mpos = get_opt(chunk, mpos, bit_extract(flags, 4), 4, '4b')
            
            # 4 * flags[19]
            
            mat['ua6'], mpos = get_opt(chunk, mpos, bit_extract(flags, 19), 4, '4b')
            
            # 0x14 * flags[17-18]
            
            mat['ua7'], mpos = get_array(chunk, mpos, bit_extract(flags, 17, 18), 0x14, 'ua7')
            
            # 4 * flags[14-16]
            
            mat['ua8'], mpos = get_array(chunk, mpos, bit_extract(flags, 14, 16), 4, '4b')
            
            # 0x10 * flags[9-13]
            
            mat['ua8'], mpos = get_array(chunk, mpos, bit_extract(flags, 9, 13), 0x10, '10b')
            
            # 4 * flags[8], these are bytes btw
            
            mat['uaa'], mpos = get_opt(chunk, mpos, bit_extract(flags, 8), 4, '4b')
            
            # 4 * flags[7]
            
            mat['uab'], mpos = get_opt(chunk, mpos, bit_extract(flags, 7), 4, '4b')

            if n < vars['num'] - 1:
                next_offset, = struct.unpack('>I', chunk[pos+4:pos+8])
                if next_offset - 8 != mpos:
                    mat['~_insane'] = next_offset - 8 - mpos # Extra shit we didn't parse :(

            vars['materials'].append(mat)
            pos += 4
    elif typ in ('grs1', 'pas1'):
        indent += 1
    elif typ in ('gre1', 'pae1'):
        indent -= 1
    else:
        print 'Unhandled type %s' % typ
        print repr(chunk)
    if vars: pr_dict(vars, '    ' * indent + '    ')
#print hex(len(ch))
