import sys, struct, re
from ethyl import *

f = open(sys.argv[1], 'r')
chunks = eval(f.read())
f.close()

assert len(chunks) == 0 # :/

rlan = 'RLAN\xfe\xff\x00\x08\x00\x00\x00\x00' # size later
rlan += struct.pack('>hh', 0x10, len(chunks))

rlan = rlan[:8] + struct.pack('>I', len(rlan)) + rlan[0xc:]


g = open(sys.argv[2], 'wb')
g.write(rlan)
g.close()