#!/usr/bin/env python

import sys
import re
import os

UPC_REGEX = re.compile(r'(\d\d\d\d\d\d)-(\d\d\d\d\d\d)[-_\.](.*)')
EAN_REGEX = re.compile(r'(\d)(\d\d\d\d\d\d)(\d\d\d\d\d\d)[-_\.](.*)')


def rename(filename, parts):
    newname = '-'.join(parts)
    print filename, '=>', newname
    os.rename(filename, newname)


for filename in sys.argv:
    match = UPC_REGEX.match(filename)
    if match:
        parts = list(match.groups())
        parts[0] = parts[0].replace('-', '')
        parts[1] = parts[1].replace('-', '')
        parts.insert(0, '0')
        rename(filename, parts)
        continue
    match = EAN_REGEX.match(filename)
    if match:
        rename(filename, match.groups())
        continue
