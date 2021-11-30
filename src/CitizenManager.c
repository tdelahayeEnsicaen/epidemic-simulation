#include "CitizenManager.h"
#include "Process.h"

#include "Citizen.h"
#include "Doctor.h"
#include "Firefighter.h"

#include <pthread.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

pthread_mutex_t updateMutex;
pthread_cond_t updateCond;
pthread_t threads[CITIZEN_COUNT];

char isRunning = 1;
clock_t start_clock, end;
int counter;

#define SIM_TO_CITIZEN 0
#define CITIZEN_TO_SIM 1

int tubes[2];

void openTubes()
{
    printf("[CIT] Open tubes\n");

    tubes[SIM_TO_CITIZEN] = openTube(SIM_TO_CITIZEN_NAME, O_RDONLY, false);
    tubes[CITIZEN_TO_SIM] = openTube(CITIZEN_TO_SIM_NAME, O_WRONLY, false);
}

void closeTubes()
{
    printf("[CIT] Close tubes\n");

    closeTube(SIM_TO_CITIZEN_NAME, tubes[SIM_TO_CITIZEN], false);
    closeTube(CITIZEN_TO_SIM_NAME, tubes[CITIZEN_TO_SIM], false);
}

int main()
{
    printf("[CIT] Start\n");

    openTubes();

    while (isRunning)
    {
        enum ProcessAction action = readAction(tubes[SIM_TO_CITIZEN]);
        bool result;

        switch (action)
        {
        case INIT:
            printf("[CIT] Initialization\n");
            initCitizens();
            printf("[CIT] Running\n");
            result = true;
            break;

        case UPDATE:
            printf("[CTI] Update\n");
            updateCitizens();
            result = true;
            break;

        case DESTROY:
            printf("[CIT] Destroy\n");
            destroyCitizens();
            isRunning = false;
            result = true;
            break;
        
        default:
            printf("[CTI] Error: invalid action %d\n", action);
            result = false;
            break;
        }

        sendResult(tubes[CITIZEN_TO_SIM], result);
    }

    closeTubes();

    return 0;
}

void initCitizens()
{
    pthread_mutex_init(&updateMutex, NULL);
    pthread_cond_init(&updateCond, NULL);

    loadMap();

    for (int i = 0; i < CITIZEN_COUNT; i++)
    {
        Citizen* pCitizen = getCitizen(i);

        switch (pCitizen->type)
        {
        case ORDINARY_PEOPLE:
            pthread_create(&threads[i], NULL, ordinaryPeopleHandler, pCitizen);
            break;

        case DOCTOR:
            pthread_create(&threads[i], NULL, doctorHandler, pCitizen);
            break;

        case FIREFIGHTER:
            pthread_create(&threads[i], NULL, firefighterHandler, pCitizen);
            break;

        case JOURNALIST:
            pthread_create(&threads[i], NULL, journalistHandler, pCitizen);
            break;
        
        default:
            printf("Error invalid type %d\n", pCitizen->type);
            break;
        }
    }
}

void destroyCitizens()
{
    //pthread_cond_destroy(&updateCond);
    pthread_mutex_destroy(&updateMutex);
}

void updateCitizens()
{
    pthread_mutex_lock(&updateMutex);

    if (counter == CITIZEN_COUNT)
    {
        double time = ((double) (end - start_clock)) / CLOCKS_PER_SEC * 1000.0;
        printf("Citizens update execution time: %.2f\n", time);
    }
    else
    {
        printf("Citizens update took too many time\n");
    }
    
    start_clock = clock();
    counter = 0;

    pthread_cond_broadcast(&updateCond);
    pthread_mutex_unlock(&updateMutex);
}

void waitSignal()
{
    pthread_mutex_lock(&updateMutex);

    counter++;

    if (counter == CITIZEN_COUNT)
        end = clock();

    pthread_cond_wait(&updateCond, &updateMutex);
    pthread_mutex_unlock(&updateMutex);
}

void *ordinaryPeopleHandler(void* pArg)
{
    Citizen* pCitizen = pArg;
    bool hasMoved;
    float contamination;
    Tile tile;

    while (isRunning)
    {
        waitSignal();

        hasMoved = updatePosition(pCitizen);
        contamination = pCitizen->contamination;
        tile = getTile(pCitizen->x, pCitizen->y);

        exchangeContaminationWithTile(pCitizen, contamination, tile, hasMoved);

        propagateContaminationToCitizens(pCitizen);

        updateSickness(pCitizen);
    }

    return NULL;
}

void *doctorHandler(void* pArg)
{
    Citizen* pDoctor = pArg;
    bool hasMoved;
    float contamination;
    Tile tile;

    while (isRunning)
    {
        waitSignal();

        hasMoved = updatePosition(pDoctor);
        contamination = pDoctor->contamination;
        tile = getTile(pDoctor->x, pDoctor->y);

        exchangeContaminationWithTile(pDoctor, contamination, tile, hasMoved);

        propagateContaminationToCitizens(pDoctor);

        updateSickness(pDoctor);

        updateDoctor(pDoctor, tile);
    }

    return NULL;
}

void *firefighterHandler(void* pArg)
{
    Citizen* pFirefighter = pArg;
    bool hasMoved;
    float contamination;
    Tile tile;

    while (isRunning)
    {
        waitSignal();

        hasMoved = updatePosition(pFirefighter);
        contamination = pFirefighter->contamination;
        tile = getTile(pFirefighter->x, pFirefighter->y);

        exchangeContaminationWithTile(pFirefighter, contamination, tile, hasMoved);

        propagateContaminationToCitizens(pFirefighter);

        updateSickness(pFirefighter);

        if (hasMoved)
        {
            burnDeadbody(pFirefighter);

            const Tile tile = getTile(pFirefighter->x, pFirefighter->y);

            // TODO move to epidemic sim
            if (tile.type == FIRE_STATION)
            {
                injectPulverisator(pFirefighter, 10.0f);
            }
        }

        decontaminate(pFirefighter);
    }

    return NULL;
}

void *journalistHandler(void* pArg)
{
    Citizen* pJournalist = pArg;
    bool hasMoved;
    float contamination;
    Tile tile;

    while (isRunning)
    {
        waitSignal();

        hasMoved = updatePosition(pJournalist);
        contamination = pJournalist->contamination;
        tile = getTile(pJournalist->x, pJournalist->y);

        exchangeContaminationWithTile(pJournalist, contamination, tile, hasMoved);

        propagateContaminationToCitizens(pJournalist);

        updateSickness(pJournalist);
    }

    return NULL;
}