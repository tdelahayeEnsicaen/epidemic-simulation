#include "Journalist.h"
#include <mqueue.h>
#include <stdio.h>
#include <string.h>

void sendNews(const Citizen* pJournalist, int queue)
{
    uint8_t contCount = 0;
    uint8_t deadCount = 0;

    float cont = 0.0f;

    for (uint8_t i = 0; i < CITIZEN_COUNT; i++)
    {
        const Citizen* pCitizen = getCitizen(i);

        lockCitizen(pCitizen);

        contCount += pCitizen->contamination > 0.0f;
        deadCount += pCitizen->status >= DEAD;

        unlockCitizen(pCitizen);
    }

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            cont += getTile(x, y).contamination;
        }
    }
    
    cont /= MAP_WIDTH * MAP_HEIGHT;

    uint8_t msg[10];

    msg[0] = (uint8_t)DEAD_COUNT;
    msg[1] = deadCount;

    mq_send(queue, (char*)msg, 2, 10);

    msg[0] = (uint8_t)AVERAGE_CONTAMINATION;
    memcpy(msg + 1, &cont, sizeof(float));

    mq_send(queue, (char*)msg, 1 + sizeof(float), 5);

    msg[0] = (uint8_t)CONTAMINED_CITIZENS;
    msg[1] = contCount;

    mq_send(queue, (char*)msg, 2, 2);

    msg[0] = (uint8_t)JOURNALIST_CONTAMINATION;
    memcpy(msg + 1, &pJournalist->contamination, sizeof(float));

    mq_send(queue, (char*)msg, 1 + sizeof(float), 1);
}