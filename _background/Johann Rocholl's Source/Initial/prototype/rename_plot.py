#! /usr/bin/env python

import os
import sys

oldname = sys.argv[1].split('.')[0]
newname = sys.argv[2].split('.')[0]

os.rename(oldname + '.dat', newname + '.dat')
if os.path.exists(oldname + '.pdf'):
    os.rename(oldname + '.pdf', newname + '.pdf')

newfile = file(newname + '.plt', 'w')
for line in file(oldname + '.plt').readlines():
    newfile.write(line.replace(oldname, newname))
newfile.close()
