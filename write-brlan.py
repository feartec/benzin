import sys, struct, re
from ethyl import *

CAT_COORDS  = 0x1000000
CAT_2_UNK   = 0x2000000 # alpha and stuff
COORD_X     = 0
COORD_Y     = 1

f = open(sys.argv[1], 'r')
data = eval(f.read())
f.close()

anims = data['anims']
framesize = data['framesize']
flags = data['flags']

# Assumptions: No timgs, only one pai1
ab = ''
_b = ''
_os = ''
i = 0
# No, by the time you ask, I won't understand this either
for animtype, elname, animcat, animents in anims:
    cs = []
    for byte1, cookies in animents:
        byte0 = 0
        byte2 = 2
        byte3 = 0
        c = struct.pack('>BBBBHHI', byte0, byte1, byte2, byte3, len(cookies), 0, 0xc)
        for cookie in cookies:
            c += struct.pack('>fff', *cookie)
        cs.append(c)
    assert len(animtype) == 4
    a = animtype + chr(len(animents)) + '\0\0\0'
    csb = ''
    for c in cs:
        a += struct.pack('>I', 8 + 4*len(cs) + len(csb)) # offset
        csb += c
    a += csb
    _ = unnullterm(elname, 0x14) + struct.pack('>II', animcat, len(ab) + 0x1c*(len(anims) - i))
    _os += struct.pack('>I', 0x14  + 4*len(anims) + len(_b))
    ab += a
    _b += _
    i += 1
    
pai = struct.pack('>hbbhhI', framesize, flags, 0, 0, len(anims), 
0x14) + _os + _b + ab


rlan = 'RLAN\xfe\xff\x00\x08\x00\x00\x00\x00' # size later
rlan += struct.pack('>hh', 0x10, 1)

rlan = rlan[:8] + struct.pack('>I', len(rlan)) + rlan[0xc:]

rlan += 'pai1' + struct.pack('>I', len(pai)+8) + pai

g = open(sys.argv[2], 'wb')
g.write(rlan)
g.close()