#include "Firefigther.h"

#include "Utils.h"

#include <memory.h>

void burnDeadBody(const Citizen* pFirefighter)
{
    for (uint8_t i = 0; i < CITIZEN_COUNT; i++)
    {
        Citizen* pCitizen = getCitizen(i);

        lockCitizen(pCitizen);

        if (pCitizen->status == DEAD && pCitizen->x == pFirefighter->x && pCitizen->y == pFirefighter->y)
        {
            pCitizen->status = BURNED;
        }

        unlockCitizen(pCitizen);
    }
}

void injectPulverisator(Citizen* pFirefighter, float value)
{
    float storage;
    memcpy(&storage, pFirefighter->data, sizeof(float));

    storage = min(storage + value, PULVERISATOR_CAPACITY);

    memcpy(pFirefighter->data, &storage, sizeof(float));
}

float extractPulverisator(Citizen* pFirefighter, float value)
{
    float storage;
    float result;
    memcpy(&storage, pFirefighter->data, sizeof(float));

    if (storage > value)
    {
        result = value;
        storage -= value;
    }
    else
    {
        result = storage;
        storage = 0.0f;
    }
    
    memcpy(pFirefighter->data, &storage, sizeof(float));

    return result;
}

void decontaminate(Citizen* pFirefighter)
{
    if (pFirefighter->status >= DEAD)
        return;

    float remaining = extractPulverisator(pFirefighter, PULVERISATOR_BY_TURN);

    for (uint8_t i=0; i < CITIZEN_COUNT && remaining > 0.0f; i++)
    {
        if (i == pFirefighter->id)
            continue;

        Citizen* pCitizen = getCitizen(i);

        lockCitizen(pCitizen);

        if (pCitizen->status < DEAD && pCitizen->x == pFirefighter->x && pCitizen->y == pFirefighter->y)
        {
            float remove = min(pCitizen->contamination, min(remaining, PULVERISATOR_BY_CITIZEN));

            pCitizen->contamination -= remove;
            remaining -= remove;
        }

        unlockCitizen(pCitizen);
    }

    if (remaining > 0.0f)
    {
        const Tile tile = getTile(pFirefighter->x, pFirefighter->y);

        float remove = min(tile.contamination, min(remaining, PULVERISATOR_BY_TILE));

        increaseTileContamination(pFirefighter->x, pFirefighter->y, -remove);
        remaining -= remove;
    }

    injectPulverisator(pFirefighter, remaining);
}