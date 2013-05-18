#!/usr/bin/python
import sys
from makeTick import makeTicks

tickTypes = [
    {'name': 'S32', 'scheme':
        """bbS32  count
           bbU32  opt""" },

    {'name': 'F64', 'scheme':
        """double value
           bbU32  opt""" },
]

makeTicks(tickTypes, "tsTickKPI.h.templ", "tsTickKPI.cpp.templ")
