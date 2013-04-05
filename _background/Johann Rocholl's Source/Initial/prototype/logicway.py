#! /usr/bin/env python

import os

IMPORT_PATH = '/Users/johann/Downloads/www.images'
originals = os.listdir(IMPORT_PATH)

dirname = 'logicway_fail'
for filename in os.listdir(dirname):
    if len(filename) < 16:
        continue
    shortname = filename[16:]
    if shortname not in originals:
        shortname = shortname.upper()
        if shortname not in originals:
            print filename, shortname, 'not found'
            continue
    oldpath = os.path.join(IMPORT_PATH, shortname)
    newpath = os.path.join('/Users/johann/Desktop/originals',
                           dirname, filename)
    os.rename(oldpath, newpath)
