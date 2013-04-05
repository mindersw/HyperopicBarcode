#!/usr/bin/env python

import sys
import os

NAMES = {
    'iphone': 'Apple iPhone',
    'clarifi': 'Apple iPhone with macro lens',
    't650i': 'Sony Ericsson T650i',
    'logicway_success': 'Logic Way GmbH Success',
    'logicway_fail': 'Logic Way GmbH Failure',
    'generator': 'Bitmaps and examples',
}

for dirname in sys.argv[1:]:
    counters = [0] * 14
    total = 0
    digitsname = os.path.join(dirname, 'digits')
    filenames = os.listdir(digitsname)
    filenames.sort()
    for filename in filenames:
        digits = filename[0] + filename[2:8] + filename[9:15]
        recognized = os.readlink(os.path.join(digitsname, filename))
        correct = 0
        for i in range(13):
            if digits[i] == recognized[i]:
                correct += 1
        # print filename, digits, recognized, correct
        counters[correct] += 1
        total += 1
    if len(sys.argv) == 2:
        for i in range(14):
            if counters[i] == 0:
                continue
            print "%2d digits correct: %.1f%% (%d/%d)" % (
                i, 100.0 * counters[i] / total, counters[i], total)
    else:
        print r'%s & %d & %d & %.1f\%% \\' % (
            NAMES.get(dirname[8:], dirname[8:]),
            total, counters[13], 100.0 * counters[13] / total)
