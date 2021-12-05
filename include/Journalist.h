#ifndef JOURNALIST_H
#define JOURNALIST_H

#include "Map.h"

typedef enum
{
    DEAD_COUNT,
    AVERAGE_CONTAMINATION,
    CONTAMINED_CITIZENS,
    JOURNALIST_CONTAMINATION
} NewsType;

void sendNews(const Citizen* pJournalist, int queue);

void sendDeadCount(int queue);

void sendAvgContamination(int queue);

void sendContaminedCitizens(int queue);

void sendJournalistContamination(const Citizen* pJournalist, int queue);

#endif