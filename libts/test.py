#!/usr/bin/python

from pyts import *

sender = tsTickSender()
tick = tsTick()
sender.send(tick)
