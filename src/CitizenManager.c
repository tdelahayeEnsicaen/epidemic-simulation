#include "CitizenManager.h"
#include "Process.h"

#include "Citizen.h"
#include "Doctor.h"
#include "Firefigther.h"
#include "Journalist.h"

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
#include <mqueue.h>

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

pthread_mutex_t journalistMutex;

bool isRunning = true;
enum CitizenUpdateStep updateStep = WAIT;
int counter = 0;

#define JOURNALISTS_TO_PRESS 0

int queues[1];

// ----------------- PROCESS INFORMATION ------------------

Process previousProcess =
{
    .input = { .name = CITIZEN_TO_SIM_NAME },
    .output = { .name = SIM_TO_CITIZEN_NAME }
};

const char* getProcessName()
{
    return "CIT";
}

Process* getPreviousProcess()
{
    return &previousProcess;
}

Process* getNextProcesses(int* pSize)
{
    *pSize = 0;
    return NULL;
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
    pthread_mutex_init(&updateMutex, NULL);
    pthread_cond_init(&updateCond, NULL);

    pthread_mutex_init(&updateStepMutex, NULL);
    pthread_cond_init(&updateStepCond, NULL);

    pthread_mutex_init(&journalistMutex, NULL);

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

    queues[JOURNALISTS_TO_PRESS] = mq_open(JOURNALISTS_TO_PRESS_NAME, O_WRONLY);

    return queues[JOURNALISTS_TO_PRESS] != -1;
}

bool update()
{
    pthread_mutex_lock(&updateMutex);
    pthread_cond_broadcast(&updateCond);
    pthread_mutex_unlock(&updateMutex);

    return true;
}

bool destroy()
{
    isRunning = false;

    pthread_cond_broadcast(&updateCond);
    pthread_mutex_unlock(&updateMutex);

    //pthread_cond_destroy(&updateCond);
    pthread_mutex_destroy(&updateMutex);

    //pthread_cond_destroy(&updateStepCond);
    pthread_mutex_destroy(&updateStepMutex);

    pthread_mutex_destroy(&journalistMutex);

    mq_close(queues[JOURNALISTS_TO_PRESS]);

    return true;
}

void waitSignal()
{
    pthread_mutex_lock(&updateMutex);
    pthread_cond_wait(&updateCond, &updateMutex);
    pthread_mutex_unlock(&updateMutex);
}

void nextStep()
{
    pthread_mutex_lock(&updateStepMutex);

    counter++;

    if (counter == CITIZEN_COUNT)
    {
        updateStep = (updateStep + 1) % 6;

        pthread_cond_broadcast(&updateStepCond);
        counter = 0;
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
            pthread_mutex_lock(&journalistMutex);
            sendNews(pJournalist, queues[JOURNALISTS_TO_PRESS]);
            pthread_mutex_unlock(&journalistMutex);
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