import os
import sys
import re
from glob import glob

ean_regex = re.compile(r'(\d)-(\d{6})-(\d{6})')

candidates = []
# candidates.extend(glob('/Users/johann/checkout/testsuite/jpg/*.JPG'))
# candidates.extend(glob('/Users/johann/Desktop/testsuite/*.JPG'))
candidates.extend(glob('/Users/johann/Desktop/barcodes/vertical-middle/*.jpg'))

for dirname in sys.argv[1:]:
    for filename in os.listdir(dirname):
        newpath = os.path.join('/Users/johann/Desktop/originals', dirname)
        if not os.path.exists(newpath):
            os.makedirs(newpath)
        newpath = os.path.join(newpath, filename)
        if os.path.exists(newpath):
            # print 'new path exists'
            continue
        match = ean_regex.match(filename)
        if match is None:
            # print 'no ean match'
            continue
        ean = ''.join(match.groups())
        name = filename[16:]
        if name.startswith('IMG'): name = name[3:]
        original = None
        for candidate in candidates:
            if candidate.endswith(name):
                original = candidate
        print dirname, filename, ean, name
        if original:
            print '   ', original
            print '   ', newpath
            os.rename(original, newpath)
