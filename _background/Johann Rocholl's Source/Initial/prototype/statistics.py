#!/usr/bin/env python

import sys
import os

GROUPS = """
iphone            Apple iPhone
clarifi           Apple iPhone with macro lens
t650i             Sony Ericsson T650i
logicway_success  PDAbar correct
logicway_fail     PDAbar not found
""".strip().splitlines()

GROUP_NAMES = []
DESCRIPTIONS = {}
for line in GROUPS:
    parts = line.split()
    name = parts[0]
    description = ' '.join(parts[1:])
    GROUP_NAMES.append(name)
    DESCRIPTIONS[name] = description


def count(digitsname):
    """
    Read symlinks and count results in a digits folder.
    """
    counters = [0] * 14
    if not os.path.exists(digitsname):
        return counters
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
    return counters


def summary_table(dirname):
    """
    Produce LaTeX table with one line for each GROUPS folder in
    dirname, and a summary line below.
    """
    print r'  \begin{tabular}{|l|r|r|r|}'
    print r'    \hline'
    print r'    %-30s & Total & Correct & Success rate \\' % 'Image source'
    print r'    \hline'
    format = r'    %-30s &%4d &%4d & %.1f\%% \\'
    summary = [0] * 14
    for name in GROUP_NAMES:
        digitsname = os.path.join(dirname, name, 'digits')
        if not os.path.exists(digitsname):
            continue
        counters = count(digitsname)
        for index, counter in enumerate(counters):
            summary[index] += counter
        correct = counters[13]
        total = sum(counters)
        print format % (DESCRIPTIONS[name], total, correct,
                        100.0 * correct / total)
    correct = summary[13]
    total = sum(summary)
    print r'    \hline'
    print format % ("All test images", total, correct,
                    100.0 * correct / total)
    print r'    \hline'
    print r'  \end{tabular}'
    print r'  \caption{Test results for five sets of test images}'
    print r'  \label{table:results_testsuite}'


def zoom_table(dirnames):
    """
    Produce LaTeX table with zoom results on one line for each GROUPS
    folder in dirname, and a summary line below.
    """
    description_format = r'    %-30s'
    percentage_format = r'& %5.1f\%%'
    print r'  \begin{tabular}{|l%s|}' % ('|r' * len(dirnames))
    print r'    \hline'
    dirnames.sort()
    dirnames.reverse()
    print description_format % 'Maximum width in pixels',
    summary_correct = {}
    summary_total = {}
    for dirname in dirnames:
        ext = dirname.split('.')[1]
        number = int(ext.split('/')[0])
        print '&%6d' % number,
        summary_correct[dirname] = 0
        summary_total[dirname] = 0
    print r'\\'
    print r'    \hline'
    for name in GROUP_NAMES:
        print description_format % DESCRIPTIONS[name],
        for dirname in dirnames:
            digitsname = os.path.join(dirname, name, 'digits')
            counters = count(digitsname)
            correct = counters[13]
            total = sum(counters)
            if total:
                print percentage_format % (100.0 * correct / total),
            else:
                print '&    -   ',
            summary_correct[dirname] += correct
            summary_total[dirname] += total
        print r'\\'
    print r'    \hline'
    print description_format % "All test images",
    for dirname in dirnames:
        correct = summary_correct[dirname]
        total = summary_total[dirname]
        print percentage_format % (100.0 * correct / total),
    print r'\\'
    print r'    \hline'
    print r'  \end{tabular}'
    print r'  \caption{Test results for different resolutions}'
    print r'  \label{table:results_resolutions}'


def partially_correct_table(dirname):
    """
    Produce LaTeX table with partially correct results.
    """
    print r'  \begin{tabular}{|l' + 4 * '|r' + '|}'
    print r'    \hline'
    print r'    Number of correct digits & $0\dots3$ & $4\dots7$ & $8\dots11$ & 13 \\'
    print r'    \hline'
    format = r'    %-30s' + 4 * ' & %.1f\%%' + r' \\'
    summary = [0] * 14
    for name in GROUP_NAMES:
        digitsname = os.path.join(dirname, name, 'digits')
        if not os.path.exists(digitsname):
            continue
        counters = count(digitsname)
        for index, counter in enumerate(counters):
            summary[index] += counter
        total = sum(counters)
        print format % (DESCRIPTIONS[name],
                        100.0 * sum(counters[0:4]) / total,
                        100.0 * sum(counters[4:8]) / total,
                        100.0 * sum(counters[8:12]) / total,
                        100.0 * counters[13] / total)
    print r'    \hline'
    counters = summary
    total = sum(counters)
    print format % ('All test images',
                    100.0 * sum(counters[0:4]) / total,
                    100.0 * sum(counters[4:8]) / total,
                    100.0 * sum(counters[8:12]) / total,
                    100.0 * counters[13] / total)
    print r'    \hline'
    print r'  \end{tabular}'
    print r'  \caption{Percentages of partially correct results}'
    print r'  \label{table:results_partial}'


if __name__ == '__main__':
    print r'\begin{table}[htp]'
    print r'  \centering'
    if len(sys.argv) == 1:
        summary_table('results')
    if len(sys.argv) == 2:
        if sys.argv[1] == '--partial':
            partially_correct_table('results')
        else:
            summary_table(sys.argv[1])
    if len(sys.argv) > 2:
        zoom_table(sys.argv[1:])
    print r'\end{table}'
