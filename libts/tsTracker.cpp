#include "tsTracker.h"

tsTracker::tsTracker()
{
}

void tsTracker::Subscribe(std::string node, bbU64 feedID)
{
    mSubscribers.insert(std::pair<bbU64,std::string>(feedID, node));
}

