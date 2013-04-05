#!/bin/sh
# Run this script to generate a complete RSS feed:
# ./rss.sh > rss.xml

TITLE='Prototype changelog'
DESCRIPTION='Automatically generated from bzr log.'
URL=http://johann.rocholl.net/prototype/
LANGUAGE=en-us

cat <<EOF
<?xml version="1.0"?>
<rss version="2.0">
<channel>
<title>$TITLE</title>
<link>$URL</link>
<description>$DESCRIPTION</description>
<language>$LANGUAGE</language>
<generator>bzr-log-rss v0.1</generator>
EOF

bzr log --verbose --revision -20.. --rss | sed "s|<link>|<link>$URL|"

cat <<EOF
</channel>
</rss>
EOF
