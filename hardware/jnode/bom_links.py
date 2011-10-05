#!/bin/sh
cat jnode.html | python2 -c "import sys,re;[sys.stdout.write(re.sub('http://(.*?)<', r'<a href=\"http://\1\">link</a><', line)) for line in sys.stdin]"
