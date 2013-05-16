#!/usr/bin/python
import sys
from makeTick import makeTicks

tickTypes = [
    {'name': 'Price', 'scheme':
        """double price
           bbU32  opt""" },

    {'name': 'Volume', 'scheme':
        """bbU64  volume
           bbU32  opt""" },

    {'name': 'PriceVolume', 'scheme':
        """double price
           bbU64  volume
           bbU32  opt""" },

    {'name': 'Bid', 'scheme':
        """double price
           bbU64  volume
           bbU32  opt""" },

    {'name': 'Ask', 'scheme':
        """double price
           bbU64  volume
           bbU32  opt""" },

    {'name': 'BidAsk', 'scheme':
        """double priceBid
           double priceAsk
           bbU32  opt""" },
]

makeTicks(tickTypes, "tsTickFinance.h.templ", "tsTickFinance.cpp.templ")
