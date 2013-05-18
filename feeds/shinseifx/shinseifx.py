#!/usr/bin/python

import sys, datetime
from BeautifulSoup import BeautifulSoup
sys.path.append("../../libts")
from pyts import *
sys.path.append("..")
from pytsutil import *

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

fetchDir = "data"
try:
    os.mkdir(fetchDir)
except:
    pass

replay = len(sys.argv) != 1
loop = 0

sender = tsTickSender()

if replay:
    for root, dirs, files in os.walk(fetchDir):
        for name in files:
            print loop, name
            page = open(os.path.join(root, name)).read()
            parsePage(sender, page)
            loop += 1
else:
    poll = HttpPoll("http://sre.shinseibank.com/InterestRateB/FXRate_EN.aspx")
    while True:
        page = poll.pollHttp()
        if page: parsePage(sender, page)
        time.sleep(90)
