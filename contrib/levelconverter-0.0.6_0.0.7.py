#!/usr/bin/python

import sys

if len(sys.argv) != 2:
    print "Usage: %s FILENAME" % sys.argv[0]
    sys.exit()

print ";; Converting:", sys.argv[1]

f = open(sys.argv[1], 'r')
mycontent = f.readlines()

content = []

for i in mycontent:
    content.append(i[:-1])

name       = content[0]
theme      = content[1]
time       = content[2]
music      = content[3]
background = content[4]
red        = content[5]
green      = content[6]
blue       = content[7]
width      = content[8]
height     = "15"
gravity    = content[9]

print "(supertux-level"
print "  (name \"%s\")" % name
print "  (theme \"%s\")" % theme
print "  (music \"%s\")" % music
print "  (background \"%s\")" % background
print "  (bkgd_red   %s)" % red
print "  (bkgd_green %s)" % green
print "  (bkgd_blue  %s)" % blue
print "  (time %s)" % time
print "  (width %s)" % width
print "  (gravity %s)" % gravity
print "  (tilemap "
for i in content[10:]:
    for x in i:
        sys.stdout.write("%3d" % ord(x))
        sys.stdout.write(" ")
    print ""
print "   )"
print ")"

# EOF #
