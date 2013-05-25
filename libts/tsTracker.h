#ifndef tsTRACKER_H
#define tsTRACKER_H

#include "tsdef.h"
#include <map>

class tsTracker
{
    typedef std::multimap<bbU64, std::string> SubscriberMap;
    SubscriberMap mSubscribers;

public:
    tsTracker();
    void Subscribe(std::string node, bbU64 feedID);
    void Announce(bbU64 feedID);
};

#endif

