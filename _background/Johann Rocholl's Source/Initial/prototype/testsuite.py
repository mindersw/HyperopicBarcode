#! /usr/bin/env python

import os
import sys

def rmdir(dirname):
    if not os.path.exists(dirname):
        return
    for root, dirs, files in os.walk(dirname, topdown=False):
        for name in files:
            os.remove(os.path.join(root, name))
        for name in dirs:
            os.rmdir(os.path.join(root, name))
    os.rmdir(dirname)


for dirname in os.listdir('testsuite'):
    if dirname.startswith('.'):
        continue
    if (len(sys.argv) > 1 and
        not dirname in sys.argv and
        not 'testsuite/%s' % dirname in sys.argv and
        not 'testsuite/%s/' % dirname in sys.argv):
        continue
    print dirname
    rmdir('output')
    rmdir('digits')
    os.mkdir('output')
    os.mkdir('digits')
    error = os.system('build/picture testsuite/%s/*' % dirname)
    if error:
        print "failed with exit status", error
        sys.exit(error)
    rmdir('results/%s' % dirname)
    if not os.path.exists('results/%s' % dirname):
        os.makedirs('results/%s' % dirname)
    os.rename('output', 'results/%s/output' % dirname)
    os.rename('digits', 'results/%s/digits' % dirname)
