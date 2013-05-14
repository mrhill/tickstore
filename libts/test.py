#!/usr/bin/python

from pyts import *

sender = tsTickSender()
tick = tsTick()
sender.send(tick)

objid = tsObjID(0x42, 0xDEADDEADB000B000)
tick = tsTickPriceVolume(objid)

for i in range(100):
	tick.volume = i
	sender.send(tick)
