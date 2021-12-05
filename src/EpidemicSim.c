#include "EpidemicSim.h"
#include "Process.h"
#include "DataCollector.h"

#include "Utils.h"

#include "Firefigther.h"
#include "Doctor.h"

// ----------------- PROCESS INFORMATION ------------------

Process nextProcesses[] = 
{
    // PRESS AGENCY
    {
        .input = { .name = SIM_TO_PRESS_NAME },
        .output = { .name = PRESS_TO_SIM_NAME }
    },
    // TIMER
    {
        .input = { .name = SIM_TO_TIMER_NAME },
        .output = { .name = TIMER_TO_SIM_NAME }
    },
    // CITIZEN MANAGER
    {
        .input = { .name = SIM_TO_CITIZEN_NAME },
        .output = { .name = CITIZEN_TO_SIM_NAME }
    }
};

const char* getProcessName()
{
    return "EPI";
}

Process* getPreviousProcess()
{
    return NULL;
}

Process* getNextProcesses(int* pSize)
{
    *pSize = 3;
    return nextProcesses;
}

// ------------------ PROCESS LIFE CYCLE ------------------

bool parseArguments(int argc, char const *argv[])
{
    argc = argc;
    argv = argv;
    return true;
}

bool initialize()
{
    createMap();
    initDataCollector();

    return true;
}

const Point PROPAGATION_DIRECTIONS[8] = 
{ 
    { +1, +1 }, { +1, +0 }, 
    { +1, -1 }, { +0, +1 }, 
    { +0, -1 }, { -1, +1 }, 
    { -1, +0 }, { -1, -1 } 
};

bool update()
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            const Tile src = getTile(x, y);

            switch (src.type)
            {
            case WASTELAND:
                for (uint8_t i = 0; i < 8; i++)
                {
                    propagateContamination(src, x + PROPAGATION_DIRECTIONS[i].x, y + PROPAGATION_DIRECTIONS[i].y);
                }
                break;

            case FIRE_STATION:
                for (uint8_t id = 0; id < CITIZEN_COUNT; id++)
                {
                    Citizen* pCitizen = getCitizen(id);

                    if (pCitizen->x == x && pCitizen->y == y)
                    {
                        pCitizen->contamination = max(pCitizen->contamination - 0.20f, 0.0f);
                        pCitizen->hasContaminationDetector = true;

                        if (pCitizen->type == FIREFIGHTER)
                        {
                            injectPulverisator(pCitizen, 10.0f);
                        }
                    }
                }
                break;

            case HOSPITAL:
                for (uint8_t id = 0; id < CITIZEN_COUNT; id++)
                {
                    Citizen* pCitizen = getCitizen(id);

                    if (pCitizen->type == DOCTOR && pCitizen->x == x && pCitizen->y == y)
                    {
                        pCitizen->data[CARE_POCKET_INDEX] = MAX_CARE_POCKET;
                    }
                }
                break;
            
            default:
                break;
            }
        }
    }

    updateDataCollector();

    return true;
}

void propagateContamination(Tile src, int xDest, int yDest)
{
    if (xDest < 0 || xDest >= MAP_WIDTH || yDest < 0 || yDest >= MAP_HEIGHT)
        return;

    Tile dest = getTile(xDest, yDest);

    if (dest.type != WASTELAND || src.contamination <= dest.contamination)
        return;

    if (genFloat() > 0.15f)
        return;

    const float diff = src.contamination - dest.contamination;

    increaseTileContamination(xDest, yDest, diff * (0.01f + 0.19f * genFloat()));
}

bool destroy()
{
    destroyMap();
    destroyDataCollector();

    return true;
}