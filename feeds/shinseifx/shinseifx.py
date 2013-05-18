#!/usr/bin/python

import sys, datetime
sys.path.append("../../libts")
sys.path.append("..")
from pytsutil import *
from pyts import *
from BeautifulSoup import BeautifulSoup

groups = [ 'Regular', 'Gold', 'Platinum' ]

def parsePage(sender, page):
    soup = BeautifulSoup(page)

    groupIdx = 0
    list = []
    symbol = ''
    ask = 0.0
    bid = 0.0

    for td in soup.findAll('td', 'font12'):
        s = td.string
        if s:
            s = s.strip(" \t\n\r")

            if s.startswith("Last update :"):
                datestr = s[13:].replace("&nbsp;&nbsp;","-").replace(" ","").replace("\t","").replace("\n","").replace("\r","")
                t = datetime.datetime.strptime(datestr, "%Y/%m/%d-%H:%M")

                for item in list:
                    print t, item[0]+"JPY."+groups[groupIdx][0], item[1], item[2],

                    obj = tsObjID(400, symFromStr( str(item[0]+"JPY."+groups[groupIdx][0]) ))
                    tick = tsTickBidAsk(obj, item[1], item[2], 0)
                    tick.setTime(tsTime(t.year, t.month, t.day, t.hour, t.minute, 0, 0))
                    print tick
                    sender.send(tick)

                groupIdx+=1
                list = []
                if groupIdx==3: return list
            elif '(' in s:
                i=s.index('(')
                symbol = s[i+1:i+4]
                bid = 0.0
                ask = 0.0
            else:
                if not bid:
                    bid=float(s)
                else:
                    ask=float(s)
                    list.append( (symbol, bid, ask) )

sender = tsTickSender()

fetchDir = "data"
mkdir(fetchDir)

replay = len(sys.argv) != 1
pollUrl(sender, "http://sre.shinseibank.com/InterestRateB/FXRate_EN.aspx", 90, fetchDir, replay, parsePage)
