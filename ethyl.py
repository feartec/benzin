import sys, struct, re
formats = {
    'pane': '''
            u8 flags_1          // 8
            u8 flags_2          // 9
            u8 alpha_1          // a
            u8 alpha_2          // b
            char name[0x18]     // c
            float x             // 24
            float y             // 28
            float z             // 2c
            float pane_unk1     // 30
            float pane_unk2     // 34
            float angle         // 38
            float xmag          // 3c
            float ymag          // 40
            float width         // 44
            float height        // 48
            etc!
        ''',
    'text': '''
            u16 unk1        // 4c
            u16 unk2        // 4e
            u16 mat_off     // 50
            u16 unk3        // 52
            u8 unk4         // 54
            u8 unk5[3]      // 55
            u32 name_offs   // 58
            u32 unk6        // 5c
            u32 unk7        // 60
            float unk8      // 64
            float unk9      // 68
            float unka      // 6c
            float unkb      // 70
            etc!
        ''',
    'pic': '''
            u32 unk1        // 4c
            u32 unk2        // 50
            u32 unk3        // 54
            u32 unk4        // 58
            u16 mat_off     // 5c
            u8 num_texcoords// 5e, should be < 8
            u8 padding      // 5f
            etc!
        ''',
    'group': '''
            char name[16]
            u16 numsubs
            u16 unk
            etc!
        ''',
        
    'numoffs': '''
            u16 num
            u16 offs
            etc!
        ''',
    'lytheader': '''
            u32 a
            u32 b
            u32 c
        ''',

}

for (dname, definition) in formats.items():
    definition = re.sub(re.compile('//.*$', re.M), '', definition)
    definition = definition.strip()
    definition = re.sub('\n[\n ]+', '\n', definition)
    df = [[j.strip() for j in i.strip().split()] for i in definition.split('\n')]
    df = []
    etc = etcn = False
    for i in definition.split('\n'):
        i = i.strip()
        if i[:3] == 'etc':
            etc = True
            if i == 'etc!':
                etcn = True
            continue
        typ, name = [j.strip() for j in i.split()]
        m = re.match('^([^\[]+)\[([a-zA-Z0-9]+)\]$', name)
        if m:
            name = m.group(1)
            size = eval(m.group(2))
            df.append((typ, (name, size)))
        else:
            df.append((typ, name))
    formats[dname] = df, etc, etcn
    
    

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
    df, etc, etcn = formats[definition]
    #print df, repr(definition)
    #print df

    pos = start
    ret = {}
    for typ, name in df:
        try:
            name, size = name
        except:
            pos, var = parse_var(pos, typ, chunk)
        else:
            if typ == 'char':
                var = chunk[pos:pos+size].rstrip(chr(0))
                pos += size
            else:
                var = []
                for n in xrange(size):
                    pos, varn = parse_var(pos, typ, chunk)
                    var.append(varn)
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
def unparse_var(typ, var):
    if not types.has_key(typ):
        raise Exception('What\'s a %s?' % typ)
    return struct.pack('>' + types[typ], var)
def unparse_data(vars, definition, prefix=None):
    df, etc, etcn = formats[definition]
    ret = ''
    for typ, name in df:
        size = None
        try:
            name, size = name
        except:
            pass
        if prefix is not None:
            name = re.sub('^' + re.quote(prefix) + '\.', '', name)
        var = vars[name]
        if size is None:
            ret += unparse_var(typ, var)
        else:
            if typ == 'char':
                ret += var.ljust(size, '\0')
            else:
                assert len(var) == size
                for v in var:
                    ret += unparse_var(typ, v)
    return ret
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