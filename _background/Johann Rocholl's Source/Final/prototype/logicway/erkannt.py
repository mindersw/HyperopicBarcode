import os
import re

regex = re.compile(r"""
picture.php.index=(\d+)"\s+alt.*?
Code:&nbsp;.*?
(\d+)
""", re.VERBOSE | re.DOTALL)
# \?index=(\d+)" alt.*?
# Code:&nbsp;.*?

input = open('erkannt.html', 'r').read()
for index, code in regex.findall(input):
    oldname = "logicway_erkannt/%s.jpg" % index
    newname = "logicway_erkannt/%s-%s-%s-%s.jpg" % (
        code[0], code[1:7], code[7:13], index)
    if not os.path.exists(oldname):
        continue
    if os.path.exists(newname):
        continue
    os.rename(oldname, newname)
