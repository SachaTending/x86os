from sys import argv
from os import unlink

for i in argv[1:]:
    try: unlink(i)
    except: pass