#include "Journalist.h"
#include <mqueue.h>
#include <stdio.h>
#include <string.h>

void sendNews(const Citizen* pJournalist, int queue)
{
    sendDeadCount(queue);
    sendAvgContamination(queue);
    sendContaminedCitizens(queue);
    sendJournalistContamination(pJournalist, queue);
}

void sendDeadCount(int queue)
{
    uint8_t buffer[2 * sizeof(uint8_t)] = { DEAD_COUNT, 0 };

    for (uint8_t i = 0; i < CITIZEN_COUNT; i++)
    {
        const Citizen* pCitizen = getCitizen(i);

        lockCitizen(pCitizen);
        buffer[1] += pCitizen->status >= DEAD;
        unlockCitizen(pCitizen);
    }

    mq_send(queue, (char*)buffer, 2 * sizeof(uint8_t), 10);
}

void sendAvgContamination(int queue)
{
    uint8_t buffer[sizeof(uint8_t) + sizeof(float)] = { AVERAGE_CONTAMINATION };
    float* cont = (float*) (buffer + 1);

    *cont = 0.0f;

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            *cont += getTile(x, y).contamination;
        }
    }
    
    *cont /= MAP_WIDTH * MAP_HEIGHT;

    mq_send(queue, (char*)buffer, sizeof(uint8_t) + sizeof(float), 5);
}

void sendContaminedCitizens(int queue)
{
    uint8_t buffer[2 * sizeof(uint8_t)] = { CONTAMINED_CITIZENS, 0 };

    for (uint8_t i = 0; i < CITIZEN_COUNT; i++)
    {
        const Citizen* pCitizen = getCitizen(i);

        lockCitizen(pCitizen);
        buffer[1] += pCitizen->contamination > 0.0f;
        unlockCitizen(pCitizen);
    }

    mq_send(queue, (char*)buffer, 2 * sizeof(uint8_t), 2);
}

void sendJournalistContamination(const Citizen* pJournalist, int queue)
{
    uint8_t buffer[sizeof(uint8_t) + sizeof(float)] = { JOURNALIST_CONTAMINATION };
    float* cont = (float*) (buffer + 1);

    *cont = pJournalist->contamination;
    
    mq_send(queue, (char*)buffer, sizeof(uint8_t) + sizeof(float), 1);
}