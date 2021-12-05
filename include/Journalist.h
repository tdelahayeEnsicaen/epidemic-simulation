#ifndef JOURNALIST_H
#define JOURNALIST_H

enum NewsType
{
    DEAD_COUNT,
    AVERAGE_CONTAMINATION,
    CONTAMINED_CITIZENS,
    JOURNALIST_CONTAMINATION
};

#include "Map.h"

void sendNews(const Citizen* pJournalist, int queue);

#endif