#include "EpidemicSim.h"

#include "Map.h"

#include <pthread.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

FILE* pFile;

#define SIM_TO_CITIZEN 0
#define CITIZEN_TO_SIM 1
#define SIM_TO_TIMER 2
#define TIMER_TO_SIM 3

int tubes[4];

enum Action
{
    INIT,
    UPDATE,
    DESTROY
};

int openTube(char* name, int flags)
{
    unlink(name);

    int tube = mkfifo(name, 0644);

    if (tube == -1)
    {
        fprintf(stderr, "Failled to create tube: %s\n", name);
        exit(EXIT_FAILURE);
    }

    tube = open(name, flags);

    if (tube == -1)
    {
        fprintf(stderr, "Failled to open tube: %s\n", name);
        exit(EXIT_FAILURE);
    }

    return tube;
}

void openTubes()
{
    printf("[EPI] Open tubes\n");

    tubes[SIM_TO_CITIZEN] = openTube("./sim_to_citizen", O_WRONLY);
    tubes[CITIZEN_TO_SIM] = openTube("./citizen_to_sim", O_RDONLY);

    tubes[SIM_TO_TIMER] = openTube("./sim_to_timer", O_WRONLY);
    tubes[TIMER_TO_SIM] = openTube("./timer_to_sim", O_RDONLY);
}

void closeTubes()
{
    printf("[EPI] Close tubes\n");

    close(tubes[SIM_TO_CITIZEN]);
    close(tubes[CITIZEN_TO_SIM]);

    unlink("./sim_to_citizen");
    unlink("./citizen_to_sim");

    close(tubes[SIM_TO_TIMER]);
    close(tubes[TIMER_TO_SIM]);

    unlink("./sim_to_timer");
    unlink("./timer_to_sim");
}

int main()
{
    printf("[EPI] Start\n");

    openTubes();

    bool isRunning = true;

    while (isRunning)
    {
        enum Action action;
        bool result;
        read(tubes[TIMER_TO_SIM], &action, sizeof(enum Action));

        switch(action)
        {
        case INIT:
            printf("[EPI] Initialization\n");
            initSimulation();
            printf("[EPI] Running\n");
            result = true;
            break;

        case UPDATE:
            printf("[EPI] Update\n");
            saveMap(pFile);

            write(tubes[SIM_TO_CITIZEN], &action, sizeof(enum Action));

            updateSimulation();

            read(tubes[CITIZEN_TO_SIM], &result, sizeof(bool));
            break;

        case DESTROY:
            printf("[EPI] Destroy\n");
            destroySimulation();
            result = true;
            isRunning = false;
            break;

        default:
            printf("[EPI] Error: invalid action %d\n", action);
            result = false;
            break;
        }

        write(tubes[SIM_TO_TIMER], &result, sizeof(bool));
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

    enum Action action = INIT;
    write(tubes[SIM_TO_CITIZEN], &action, sizeof(enum Action));

    bool result;
    read(tubes[CITIZEN_TO_SIM], &result, sizeof(bool));
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
    enum Action action = DESTROY;
    write(tubes[SIM_TO_CITIZEN], &action, sizeof(enum Action));

    bool result;
    read(tubes[CITIZEN_TO_SIM], &result, sizeof(bool));

    destroyMap();
    closeTubes();
}