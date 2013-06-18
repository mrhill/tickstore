#!/usr/bin/python

from pyts import *

sender = tsTickSender()

sender.setLogin(0x1, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");

tick = tsTick()
sender.send(tick)

objid = tsObjID(0x42, 0xDEADDEADB000B000)
tick = tsTickPriceVolume(objid)

for i in range(100):
	tick.volume = i
	sender.send(tick)
