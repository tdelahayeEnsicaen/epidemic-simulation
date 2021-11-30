#include "EpidemicSim.h"
#include "Process.h"

#include "Map.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

FILE* pFile;

#define SIM_TO_CITIZEN 0
#define CITIZEN_TO_SIM 1
#define SIM_TO_TIMER 2
#define TIMER_TO_SIM 3

int tubes[4];

void openTubes()
{
    printf("[EPI] Open tubes\n");

    tubes[SIM_TO_CITIZEN] = openTube(SIM_TO_CITIZEN_NAME, O_WRONLY, true);
    tubes[CITIZEN_TO_SIM] = openTube(CITIZEN_TO_SIM_NAME, O_RDONLY, true);

    tubes[SIM_TO_TIMER] = openTube(SIM_TO_TIMER_NAME, O_WRONLY, true);
    tubes[TIMER_TO_SIM] = openTube(TIMER_TO_SIM_NAME, O_RDONLY, true);
}

void closeTubes()
{
    printf("[EPI] Close tubes\n");

    closeTube(SIM_TO_CITIZEN_NAME, tubes[SIM_TO_CITIZEN], true);
    closeTube(CITIZEN_TO_SIM_NAME, tubes[CITIZEN_TO_SIM], true);

    closeTube(SIM_TO_TIMER_NAME, tubes[SIM_TO_TIMER], true);
    closeTube(TIMER_TO_SIM_NAME, tubes[TIMER_TO_SIM], true);
}

int main()
{
    printf("[EPI] Start\n");

    openTubes();

    bool isRunning = true;

    while (isRunning)
    {
        enum ProcessAction action = readAction(tubes[TIMER_TO_SIM]);
        bool result;

        switch(action)
        {
        case INIT:
            printf("[EPI] Initialization\n");
            initSimulation();
            printf("[EPI] Running\n");

            result = sendAction(tubes[SIM_TO_CITIZEN], tubes[CITIZEN_TO_SIM], INIT);

            saveMap(pFile);
            break;

        case UPDATE:
            printf("[EPI] Update\n");
            updateSimulation();

            result = sendAction(tubes[SIM_TO_CITIZEN], tubes[CITIZEN_TO_SIM], UPDATE);

            saveMap(pFile);
            break;

        case DESTROY:
            result = sendAction(tubes[SIM_TO_CITIZEN], tubes[CITIZEN_TO_SIM], DESTROY);

            printf("[EPI] Destroy\n");
            destroySimulation();

            isRunning = false;
            break;

        default:
            printf("[EPI] Error: invalid action %d\n", action);
            result = false;
            break;
        }

        sendResult(tubes[SIM_TO_TIMER], result);
    }

    return 0;
}

void initSimulation()
{
    pFile = fopen("evolution.txt", "w");

    if (!pFile)
    {
        printf("Cannot open file\n");
        exit(EXIT_FAILURE);
    }

    createMap();
}

void propagateContamination(Tile src, int xDest, int yDest);

void updateSimulation()
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            const Tile src = getTile(x, y);
            
            if (src.type == WASTELAND)
            {
                propagateContamination(src, x+1, y+1);
                propagateContamination(src, x+1, y+0);
                propagateContamination(src, x+1, y-1);

                propagateContamination(src, x+0, y+1);
                propagateContamination(src, x+0, y-1);

                propagateContamination(src, x-1, y+1);
                propagateContamination(src, x-1, y+0);
                propagateContamination(src, x-1, y-1);
            }
        }
    }
}

void propagateContamination(Tile src, int xDest, int yDest)
{
    Tile dest = getTile(xDest, yDest);

    if (dest.type != WASTELAND || src.contamination <= dest.contamination)
        return;

    if (rand() % 100 >= 15)
        return;

    const float diff = src.contamination - dest.contamination;

    setTileContamination(xDest, yDest, dest.contamination + (diff * (0.01f + 0.19f * (float)rand() / RAND_MAX) ));
}

void destroySimulation()
{
    destroyMap();
    closeTubes();
}