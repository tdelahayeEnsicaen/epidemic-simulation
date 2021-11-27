#include "CitizenManager.h"

#include "Map.h"

#include <pthread.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

pthread_mutex_t updateMutex;
pthread_cond_t updateCond;
pthread_t threads[CITIZEN_COUNT];

pid_t epidemicSimId;

char isRunning = 1;
clock_t start_clock, end;
int counter;

enum Action
{
    INIT,
    UPDATE,
    DESTROY
};

#define SIM_TO_CITIZEN 0
#define CITIZEN_TO_SIM 1

int tubes[2];

int openTube(char* name, int flags)
{
    int tube = open(name, flags);

    if (tube == -1)
    {
        fprintf(stderr, "Failled to open tube: %s\n", name);
        exit(EXIT_FAILURE);
    }

    return tube;
}

void openTubes()
{
    printf("[CIT] Open tubes\n");

    tubes[SIM_TO_CITIZEN] = openTube("./sim_to_citizen", O_RDONLY);
    tubes[CITIZEN_TO_SIM] = openTube("./citizen_to_sim", O_WRONLY);
}

void closeTubes()
{
    printf("[CIT] Close tubes\n");

    close(tubes[SIM_TO_CITIZEN]);
    close(tubes[CITIZEN_TO_SIM]);
}

int main()
{
    printf("[CIT] Start\n");

    openTubes();

    while (isRunning)
    {
        enum Action action;
        bool result;
        read(tubes[SIM_TO_CITIZEN], &action, sizeof(enum Action));

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

        write(tubes[CITIZEN_TO_SIM], &result, sizeof(bool));
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
        int* pCitizenId = (int*) malloc(sizeof(int));
        *pCitizenId = i;

        Citizen citizen = getCitizen(i);

        switch (citizen.type)
        {
        case ORDINARY_PEOPLE:
            pthread_create(&threads[i], NULL, ordinaryPeopleHandler, pCitizenId);
            break;

        case DOCTOR:
            pthread_create(&threads[i], NULL, doctorHandler, pCitizenId);
            break;

        case FIREFIGHTER:
            pthread_create(&threads[i], NULL, firefighterHandler, pCitizenId);
            break;

        case JOURNALIST:
            pthread_create(&threads[i], NULL, journalistHandler, pCitizenId);
            break;
        
        default:
            printf("Error invalid type %d\n", citizen.type);
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

#define CITIZEN_MOVE_PROBABILITY 40

typedef struct 
{
    int x;
    int y;
} Point;

const Point DIRECTIONS[8] = { { +1, +1 }, { +1, +0 }, { +1, -1 }, { +0, +1 }, { +0, -1 }, { -1, +1 }, { -1, +0 }, { -1, -1 } };

void genDirList(Point dest[])
{
    memset(dest, 0, sizeof(Point) * 8);

    for (int i = 0; i < 8; i++)
    {
        int random = rand() % (8 - i);
        int destIndex;

        for (destIndex = 0; random != 0 || dest[destIndex].x != 0 || dest[destIndex].y != 0; destIndex++)
        {
            if (dest[destIndex].x == 0 && dest[destIndex].y == 0)
            {
                random--;
            }
        }

        dest[destIndex] = DIRECTIONS[i];
    }
}

int tryRandomMove(int citizenId)
{
    Point directions[8];
    genDirList(directions);

    const Citizen citizen = getCitizen(citizenId);

    for (int i=0; i < 8; i++)
    {
        if (moveCitizen(citizenId, citizen.x + directions[i].x, citizen.y + directions[i].y))
            return 1;
    }

    return 0;
}

char updateCitizenPosition(int citizenId)
{
    float contaminationRate;
    Citizen citizen;
    Tile tile;

    citizen = getCitizen(citizenId);

    if (!citizen.alive)
        return 0;

    char hasMoved;

    if (rand() % 100 < CITIZEN_MOVE_PROBABILITY && tryRandomMove(citizenId))
    {
        contaminationRate = 0.02f;

        citizen = getCitizen(citizenId);
        tile = getTile(citizen.x, citizen.y);

        switch (tile.type)
        {
        case HOSPITAL:
            setTileContamination(citizen.x, citizen.y, tile.contamination + (citizen.contamination * 0.01f / 4.0f));
            break;

        case FIRE_STATION:
            break;
        
        default:
            setTileContamination(citizen.x, citizen.y, tile.contamination + (citizen.contamination * 0.01f));
            break;
        }

        hasMoved = 1;
    }
    else
    {
        contaminationRate = 0.05f;

        citizen = getCitizen(citizenId);
        tile = getTile(citizen.x, citizen.y);

        hasMoved = 0;
    }

    if (citizen.type == FIREFIGHTER)
    {
        contaminationRate /= 10.0f;
    }

    setCitizenContamination(citizenId, citizen.contamination + (tile.contamination * contaminationRate));

    return hasMoved;
}

#define BASE_RISK_OF_DYING 0.05f
#define HOSPITAL_DIVISOR 4.0f
#define DOCTOR_DIVISOR 2.0f

float getRiskOfDying(Citizen citizen)
{
    const Tile tile = getTile(citizen.x, citizen.y);

    if (tile.type == HOSPITAL)
    {
        return BASE_RISK_OF_DYING / HOSPITAL_DIVISOR;
    }
    
    for (int i = 0; i < DOCTOR_COUNT; i++)
    {
        const Citizen doctor = getCitizen(i);

        if (doctor.alive && doctor.x == citizen.x && doctor.y == citizen.y)
        {
            return BASE_RISK_OF_DYING / DOCTOR_DIVISOR;
        }
    }
    
    return BASE_RISK_OF_DYING;
}

void updateCitizenSickness(int citizenId)
{
    Citizen citizen = getCitizen(citizenId);
    const Tile tile = getTile(citizen.x, citizen.y);

    if (citizen.alive)
    {
        if (tile.type == FIRE_STATION)
        {
            citizen.contamination = citizen.contamination > 0.20f ? citizen.contamination - 0.20f : 0.0f;
            setCitizenContamination(citizenId, citizen.contamination);
        }

        if (!citizen.sick && ((float) rand() / RAND_MAX) < citizen.contamination)
        {
            setCitizenDayOfSickness(citizenId, 0);
            setCitizenSick(citizenId, 1);
        }

        if (citizen.sick)
        {
            if (citizen.dayOfSickness > 5 && ((float)rand() / RAND_MAX) < getRiskOfDying(citizen))
            {
                setCitizenAlive(citizenId, 0);
            }

            setCitizenDayOfSickness(citizenId, citizen.dayOfSickness+1);
        }
    }
    
    if (!citizen.burned && citizen.sick && ((float)rand() / RAND_MAX) < 0.10f)
    {
        for (int i = 0; i < CITIZEN_COUNT; i++)
        {
            const Citizen other = getCitizen(i);

            if (other.alive && other.x == citizen.x && other.y == citizen.y)
            {
                if (other.type == FIREFIGHTER && ((float)rand() / RAND_MAX) < 0.30f)
                {
                    setCitizenContamination(i, citizen.contamination + 0.01f);
                }
                else
                {
                    setCitizenContamination(i, citizen.contamination + 0.01f);
                }
            }
        }
    }

    if (!citizen.burned && citizen.sick && tile.type == WASTELAND && ((float)rand() / RAND_MAX) < 0.01f)
    {
        for (int i = 0; i < CITIZEN_COUNT; i++)
        {
            const Citizen other = getCitizen(i);

            if (other.alive && other.x != citizen.x && other.y != citizen.y)
            {
                if (abs(other.x - citizen.x) <= 1 && abs(other.y - citizen.y) <= 1)
                {
                    const Tile otherTile = getTile(other.x, other.y);

                    if (otherTile.type == WASTELAND)
                    {
                        if (other.type == FIREFIGHTER && ((float)rand() / RAND_MAX) < 0.30f)
                        {
                            setCitizenContamination(i, citizen.contamination + 0.01f);
                        }
                        else
                        {
                            setCitizenContamination(i, citizen.contamination + 0.01f);
                        }
                    }
                }
            }
        }
    }
}

void *ordinaryPeopleHandler(void* pArg)
{
    int* pCitizenId = pArg;

    while (isRunning)
    {
        waitSignal();

        updateCitizenPosition(*pCitizenId);
        updateCitizenSickness(*pCitizenId);
    }

    free(pCitizenId);

    return NULL;
}

char findPatient(int* pPatient, int x, int y)
{
    float contamination = 0.0f;
    char found = 0;

    for (int i=0; i < CITIZEN_COUNT; i++)
    {
        const Citizen citizen = getCitizen(i);

        if (citizen.sick && citizen.x == x && citizen.y == y && contamination < citizen.contamination)
        {
            *pPatient = i;
            found = 1;
        }
    }
    return found;
}

void *doctorHandler(void* pArg)
{
    int* pDoctorId = pArg;

    while (isRunning)
    {
        waitSignal();

        updateCitizenPosition(*pDoctorId);
        updateCitizenSickness(*pDoctorId);

        Citizen doctor = getCitizen(*pDoctorId);
        Tile tile = getTile(doctor.x, doctor.y);
        int patient;

        if (tile.type == HOSPITAL)
        {
            setCitizenData(*pDoctorId, 10);
            doctor.data = 10;
        }

        if (doctor.sick)
        {
            if (doctor.dayOfSickness < 10)
            {
                if (tile.type == HOSPITAL)
                {
                    setCitizenSick(*pDoctorId, 0);
                }
                else if (doctor.data > 0)
                {
                    setCitizenData(*pDoctorId, doctor.data-1);
                    setCitizenSick(*pDoctorId, 0);
                }
            }
        }
        else if (findPatient(&patient, doctor.x, doctor.y))
        {
            if (tile.type == HOSPITAL)
            {
                setCitizenSick(patient, 0);
            }
            else if (doctor.data > 0)
            {
                setCitizenData(*pDoctorId, doctor.data-1);
                setCitizenSick(patient, 0);
            }
        }
    }

    free(pDoctorId);

    return NULL;
}

void burnDeadbody(int firefighterId)
{
    const Citizen firefighter = getCitizen(firefighterId);

    for (int i=0; i < CITIZEN_COUNT; i++)
    {
        const Citizen citizen = getCitizen(i);

        if (!citizen.alive && !citizen.burned && citizen.x == firefighter.x && citizen.y == firefighter.y)
        {
            setCitizenBurned(i, 1);
            printf("Burn\n");
        }
    }
}

void decontaminate(int firefighterId)
{
    const Citizen firefighter = getCitizen(firefighterId);

    if (!firefighter.alive)
        return;

    // TODO fix float int problem
    int remainingPulverisator = firefighter.data < 100 ? firefighter.data : 100;
    const int a = remainingPulverisator;

    for (int i=0; i < CITIZEN_COUNT; i++)
    {
        if (i == firefighterId)
            continue;

        const Citizen citizen = getCitizen(i);

        if (citizen.alive && citizen.x == firefighter.x && citizen.y == firefighter.y)
        {
            int canRemove = remainingPulverisator > 20 ? 20 : remainingPulverisator;

            if (canRemove > citizen.contamination * 100.0f)
            {
                remainingPulverisator -= citizen.contamination * 100.0f;
                setCitizenContamination(i, 0.0f);
            }
            else
            {
                remainingPulverisator -= canRemove;
                setCitizenContamination(i, citizen.contamination - (canRemove / 100.0f));
            }
        }
    }

    if (remainingPulverisator > 0)
    {
        const Tile tile = getTile(firefighter.x, firefighter.y);

        int canRemove = remainingPulverisator > 20 ? 20 : remainingPulverisator;

        if (canRemove > tile.contamination * 100.0f)
        {
            remainingPulverisator -= tile.contamination * 100.0f;
            setTileContamination(firefighter.x, firefighter.y, 0.0f);
        }
        else
        {
            remainingPulverisator -= canRemove;
            setTileContamination(firefighter.x, firefighter.y, tile.contamination - (canRemove / 100.0f));
        }
    }

    setCitizenData(firefighterId, firefighter.data - (a - remainingPulverisator));
}

void *firefighterHandler(void* pArg)
{
    int* pCitizenId = pArg;

    while (isRunning)
    {
        waitSignal();

        char hasMoved = updateCitizenPosition(*pCitizenId);
        updateCitizenSickness(*pCitizenId);

        if (hasMoved)
        {
            burnDeadbody(*pCitizenId);

            const Citizen firefighter = getCitizen(*pCitizenId);
            const Tile tile = getTile(firefighter.x, firefighter.y);

            if (tile.type == FIRE_STATION)
            {
                setCitizenData(*pCitizenId, 1000);
            }
        }

        decontaminate(*pCitizenId);
    }

    free(pCitizenId);

    return NULL;
}

void *journalistHandler(void* pArg)
{
    int* pCitizenId = pArg;

    while (isRunning)
    {
        waitSignal();

        updateCitizenPosition(*pCitizenId);
        updateCitizenSickness(*pCitizenId);
    }

    free(pCitizenId);

    return NULL;
}