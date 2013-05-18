#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys, datetime
from BeautifulSoup import BeautifulSoup
sys.path.append("../../libts")
from pyts import *
sys.path.append("..")
from pytsutil import *

def parsePage(sender, page):

	for line in page.split("\n"):
		fields = line.replace('"','').strip("\n\r").split(",")
		try:
			date = datetime.datetime.strptime(fields[0], "%Y/%m/%d")
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


fetchDir = "data"
try:
    os.mkdir(fetchDir)
except:
    pass

sender = tsTickSender()

replay = len(sys.argv) != 1
loop = 0
if replay:
    for root, dirs, files in os.walk(fetchDir):
        for name in files:
            print loop, name
            page = open(os.path.join(root, name)).read()
            parsePage(sender, page)
            loop+=1
else:
	poll = HttpPoll("http://indexes.nikkei.co.jp/nkave/historical/nikkei_stock_average_daily_en.csv", "csv")
	while True:
	    page = poll.pollHttp()
	    if page: parsePage(sender, page)
	    time.sleep(3600)
