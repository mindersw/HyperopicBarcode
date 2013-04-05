#!/usr/bin/env python

import os
import sys

inverse = False
requested = 13
if len(sys.argv) == 2:
    requested = int(sys.argv[1])
if len(sys.argv) == 3 and '-n' in sys.argv[1]:
    requested = int(sys.argv[2])
    inverse = True

filenames = os.listdir("digits")
filenames.sort()
for filename in filenames:
    digits = filename[0] + filename[2:8] + filename[9:15]
    recognized = os.readlink("digits/" + filename)
    correct = 0
    for i in range(13):
        if digits[i] == recognized[i]:
            correct += 1
    if ((correct == requested and not inverse) or
        (correct != requested and inverse)):
        print filename
