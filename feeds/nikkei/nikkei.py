#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys, datetime
sys.path.append("../../libts")
sys.path.append("..")
from pyts import *
from pytsutil import *

def parsePage(sender, page):
	global latest
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

sender = tsTickSender()

fetchDir = "data"
mkdir(fetchDir)

latest = datetime.datetime(1860,1,1)
replay = len(sys.argv) != 1

pollUrl(sender, "http://indexes.nikkei.co.jp/nkave/historical/nikkei_stock_average_daily_en.csv", 10, fetchDir, replay, parsePage)
