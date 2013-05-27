#!/usr/bin/python

import sys

infile = open(sys.argv[1], "rb")
while True:
	buf = infile.read(4)
	if len(buf)==0: sys.exit(0)
	if len(buf)!=4:
		print "error1 at offset 0x%X" % infile.tell()
		sys.exit(1)

	size = ord(buf[2])
	tt = ord(buf[0])
	print "%d-0x%X(0x%X)" % (tt,size,infile.tell()-4),

	buf = infile.read(size-4);
	if size-4 != len(buf):
		print "error2 at offset 0x%X" % infile.tell()
		sys.exit(1)

