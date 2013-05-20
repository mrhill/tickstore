#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys, datetime
sys.path.append("../../libts")
sys.path.append("..")
from pyts import *
from pytsutil import *

latestFilename = "latest.txt"

def parsePage(sender, page):
	global latest, latestFilename
	for line in page.split("\n"):
		fields = line.replace('"','').strip("\n\r").split(",")
		try:
			date = datetime.datetime.strptime(fields[0], "%Y/%m/%d")
			if date <= latest: continue
			latest = date
			priceClose = float(fields[1])
			priceOpen = float(fields[2])
			priceHi = float(fields[3])
			priceLo = float(fields[4])
		except ValueError:
			continue

		obj = tsObjID(29, symFromStr("^N225"))
		tick = tsTickRecap(obj, priceOpen, priceHi, priceLo, priceClose, 0, 0, 0)
		tick.setTime(tsTime(date.year, date.month, date.day, 0, 0, 0, 0))
		print tick
		sender.send(tick)

	latestFile = open(latestFilename,"w")
	latestFile.write(latest.isoformat())
	latestFile.close()


sender = tsTickSender()

fetchDir = "data"
mkdir(fetchDir)

latest = datetime.datetime(1860,1,1)
try:
	print "Reading", latestFilename, "...",
	latestFile = open(latestFilename)
	latest = datetime.datetime.strptime(latestFile.read(), "%Y-%m-%dT%H:%M:%S" )
	latestFile.close()
except IOError:
	pass
except ValueError:
	pass
print "skipping until", latest

replay = len(sys.argv) != 1

pollUrl(sender, "http://indexes.nikkei.co.jp/nkave/historical/nikkei_stock_average_daily_en.csv", 3600, fetchDir, replay, parsePage)
