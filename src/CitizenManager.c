#include "CitizenManager.h"
#include "Process.h"

#include "Citizen.h"
#include "Doctor.h"
#include "Firefigther.h"

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

enum CitizenUpdateStep
{
    POSITION,
    TILE_CONTAMINATION,
    CITIZEN_CONTAMINATION,
    SICKNESS,
    SPECIAL_ACTION,
    WAIT
};

pthread_mutex_t updateMutex;
pthread_cond_t updateCond;
pthread_t threads[CITIZEN_COUNT];

pthread_mutex_t updateStepMutex;
pthread_cond_t updateStepCond;

char isRunning = 1;
enum CitizenUpdateStep updateStep = WAIT;
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

    pthread_mutex_init(&updateStepMutex, NULL);
    pthread_cond_init(&updateStepCond, NULL);

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

    //pthread_cond_destroy(&updateStepCond);
    pthread_mutex_destroy(&updateStepMutex);
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

int c = 0;

void nextStep()
{
    pthread_mutex_lock(&updateStepMutex);

    c++;

    if (c == CITIZEN_COUNT)
    {
        updateStep = (updateStep + 1) % 6;
        printf("[CIT] NEXT STEP: %d\n", updateStep);

        pthread_cond_broadcast(&updateStepCond);
        c = 0;
    }
    else
    {
        pthread_cond_wait(&updateStepCond, &updateStepMutex);
    }

    pthread_mutex_unlock(&updateStepMutex);
}

void *ordinaryPeopleHandler(void* pArg)
{
    Citizen* pCitizen = pArg;
    bool hasMoved;
    float contamination;
    Tile tile;

    while (isRunning)
    {
        switch (updateStep)
        {
        case POSITION:
            hasMoved = updatePosition(pCitizen);
            contamination = pCitizen->contamination;
            tile = getTile(pCitizen->x, pCitizen->y);
            break;

        case TILE_CONTAMINATION:
            exchangeContaminationWithTile(pCitizen, contamination, tile, hasMoved);
            break;

        case CITIZEN_CONTAMINATION:
            propagateContaminationToCitizens(pCitizen);
            break;

        case SICKNESS:
            updateSickness(pCitizen);
            break;

        case SPECIAL_ACTION:

            break;

        case WAIT:
            waitSignal();
            break;
        
        default:
            printf("Invalid Citizen update step: %d\n", updateStep);
            break;
        }

        nextStep();
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
        switch (updateStep)
        {
        case POSITION:
            hasMoved = updatePosition(pDoctor);
            contamination = pDoctor->contamination;
            tile = getTile(pDoctor->x, pDoctor->y);
            break;

        case TILE_CONTAMINATION:
            exchangeContaminationWithTile(pDoctor, contamination, tile, hasMoved);
            break;

        case CITIZEN_CONTAMINATION:
            propagateContaminationToCitizens(pDoctor);
            break;

        case SICKNESS:
            updateSickness(pDoctor);
            break;

        case SPECIAL_ACTION:
            updateDoctor(pDoctor, tile);
            break;

        case WAIT:
            waitSignal();
            break;
        
        default:
            printf("Invalid Citizen update step: %d\n", updateStep);
            break;
        }

        nextStep();
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
        switch (updateStep)
        {
        case POSITION:
            hasMoved = updatePosition(pFirefighter);
            contamination = pFirefighter->contamination;
            tile = getTile(pFirefighter->x, pFirefighter->y);
            break;

        case TILE_CONTAMINATION:
            exchangeContaminationWithTile(pFirefighter, contamination, tile, hasMoved);
            break;

        case CITIZEN_CONTAMINATION:
            propagateContaminationToCitizens(pFirefighter);
            break;

        case SICKNESS:
            updateSickness(pFirefighter);
            break;

        case SPECIAL_ACTION:
            if (hasMoved)
            {
                burnDeadBody(pFirefighter);

                const Tile tile = getTile(pFirefighter->x, pFirefighter->y);

                // TODO move to epidemic sim
                if (tile.type == FIRE_STATION)
                {
                    injectPulverisator(pFirefighter, 10.0f);
                }
            }

            decontaminate(pFirefighter);
            break;

        case WAIT:
            waitSignal();
            break;
        
        default:
            printf("Invalid Citizen update step: %d\n", updateStep);
            break;
        }

        nextStep();
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
        switch (updateStep)
        {
        case POSITION:
            hasMoved = updatePosition(pJournalist);
            contamination = pJournalist->contamination;
            tile = getTile(pJournalist->x, pJournalist->y);
            break;

        case TILE_CONTAMINATION:
            exchangeContaminationWithTile(pJournalist, contamination, tile, hasMoved);
            break;

        case CITIZEN_CONTAMINATION:
            propagateContaminationToCitizens(pJournalist);
            break;

        case SICKNESS:
            updateSickness(pJournalist);
            break;

        case SPECIAL_ACTION:

            break;

        case WAIT:
            waitSignal();
            break;
        
        default:
            printf("Invalid Citizen update step: %d\n", updateStep);
            break;
        }

        nextStep();
    }

    return NULL;
}