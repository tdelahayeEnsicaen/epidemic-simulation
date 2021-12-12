#include "Map.h"
#include "Utils.h"

#include "Doctor.h"

#include <memory.h>
#include <time.h>
#include <stdlib.h>

#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>

struct Map
{
    Tile tiles[MAP_WIDTH][MAP_HEIGHT];
    Citizen citizens[CITIZEN_COUNT];
};

struct Map* pMap;
int fd;

sem_t* tiles_semaphore;
sem_t* citizen_semaphores[CITIZEN_COUNT];

void genTileMap();
void genCitizens();

void createMap()
{
    fd = shm_open("/map", O_RDWR | O_CREAT, 0666);
    ftruncate(fd, sizeof(struct Map));

    pMap = mmap(NULL, sizeof(struct Map), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    sem_unlink("/tiles");
    tiles_semaphore = sem_open("/tiles", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 1);

    char name[15];

    for (int i = 0; i < CITIZEN_COUNT; i++)
    {
        sprintf(name, "/citizen_%d", i);
        sem_unlink(name);
        citizen_semaphores[i] = sem_open(name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 1);
    }

    genTileMap();
    genCitizens();
}

void loadMap()
{
    srand(time(NULL));
    fd = shm_open("/map", O_RDWR, 0666);

    if (fd == -1)
    {
        printf("ERROR Cannot open shared memory\n");
        return;
    }

    pMap = mmap(NULL, sizeof(struct Map), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    tiles_semaphore = sem_open("/tiles", O_RDWR);

    char name[15];

    for (int i = 0; i < CITIZEN_COUNT; i++)
    {
        sprintf(name, "/citizen_%d", i);
        citizen_semaphores[i] = sem_open(name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 1);
    }
}

void destroyMap()
{
    sem_destroy(tiles_semaphore);

    for (int i = 0; i < CITIZEN_COUNT; i++)
    {
        sem_destroy(citizen_semaphores[i]);
    }

    sem_unlink("/tiles");

    char name[12];

    for (int i = 0; i < CITIZEN_COUNT; i++)
    {
        sprintf(name, "/citizen_%d", i);
        sem_unlink(name);
    }

    munmap(pMap, sizeof(struct Map));
    close(fd);
    shm_unlink("/map");
}

void genTileMap()
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            pMap->tiles[x][y].type = WASTELAND;
            pMap->tiles[x][y].contamination = 0.0f;
        }
    }
    
    pMap->tiles[3][3].type = HOSPITAL;
    pMap->tiles[6][0].type = FIRE_STATION;
    pMap->tiles[0][6].type = FIRE_STATION;

    int houseToAdd = HOUSE_COUNT;
    srand(time(NULL));

    while (houseToAdd != 0)
    {
        const int x = rand() % MAP_WIDTH;
        const int y = rand() % MAP_HEIGHT;
        Tile* pTile = &pMap->tiles[x][y];

        if (pTile->type == WASTELAND)
        {
            pTile->type = HOUSE;
            houseToAdd--;
        }
    }

    int wastelandToContaminate = WASTELAND_COUNT / 10;

    while (wastelandToContaminate != 0)
    {
        const int x = rand() % MAP_WIDTH;
        const int y = rand() % MAP_HEIGHT;
        Tile* pTile = &pMap->tiles[x][y];

        if (pTile->type == WASTELAND && pTile->contamination == 0.0f)
        {
            pTile->contamination = 0.2f + ((float) rand() / (float) RAND_MAX * 0.2f);
            wastelandToContaminate--;
        }
    }
}

void createCitizenWithPosition(uint8_t id, CitizenType type, uint8_t x, uint8_t y, void* data)
{
    pMap->citizens[id].id = id;
    pMap->citizens[id].type = type;
    pMap->citizens[id].x = x;
    pMap->citizens[id].y = y;
    pMap->citizens[id].status = HEALTHY;
    pMap->citizens[id].wantToEnterHospital = false;
    pMap->citizens[id].hasContaminationDetector = (type == FIREFIGHTER);
    pMap->citizens[id].dayOfSickness = 0;
    pMap->citizens[id].contamination = 0.0f;

    memcpy(pMap->citizens[id].data, data, 4);
}

void createCitizen(uint8_t id, CitizenType type, void* data)
{
    createCitizenWithPosition(id, type, 255, 255, data);
}

void genCitizens()
{
    srand(time(NULL));

    // PREPOSITIONED PEOPLE

    uint8_t emptyData[4] = { 0, 0, 0, 0 };
    uint8_t doctorData[4] = { 5, 0, 0, 0 };

    float inStationPulverisator = 10.0f;
    float outStationPulverisator = 5.0f;

    createCitizenWithPosition(0, DOCTOR, 3, 3, doctorData);

    createCitizenWithPosition(4, FIREFIGHTER, 6, 0, &inStationPulverisator);
    createCitizenWithPosition(5, FIREFIGHTER, 0, 6, &inStationPulverisator);

    // DOCTORS

    for (uint8_t i = DOCTOR_OFFSET + 1; i < (DOCTOR_OFFSET + DOCTOR_COUNT); i++)
    {
        createCitizen(i, DOCTOR, doctorData);
    }

    // FIRE FIGTHERS

    for (uint8_t i = FIREFIGTHER_OFFSET + 2; i < (FIREFIGTHER_OFFSET + FIREFIGHTER_COUNT); i++)
    {
        createCitizen(i, FIREFIGHTER, &outStationPulverisator);
    }

    // JOURNALISTS

    for (uint8_t i = JOURNALIST_OFFSET; i < (JOURNALIST_OFFSET + JOURNALIST_COUNT); i++)
    {
        createCitizen(i, JOURNALIST, emptyData);
    }

    // ORDINARY PEOPLE

    for (uint8_t i = ORDINARY_PEOPLE_OFFSET; i < (ORDINARY_PEOPLE_OFFSET + ORDINARY_PEOPLE_COUNT); i++)
    {
        createCitizen(i, ORDINARY_PEOPLE, emptyData);
    }

    for (uint8_t id = 0; id < CITIZEN_COUNT; id++)
    {
        if (pMap->citizens[id].x != 255)
            continue;

        uint8_t x;
        uint8_t y;

        do
        {
            x = rand() % MAP_WIDTH;
            y = rand() % MAP_HEIGHT;
        } 
        while (!moveCitizen(&pMap->citizens[id], x, y));
    }
}

uint8_t getMaximumCapacity(TileType tileType)
{
    switch (tileType)
    {
    case HOSPITAL: return 12;
    case FIRE_STATION: return 8;
    case HOUSE: return 6;
    case WASTELAND: return 16;
    
    default:
        exit(EXIT_FAILURE);
    }
}

TileType getTileType(int x, int y)
{
    // Tile type doesn't change so don't lock semaphore
    return pMap->tiles[x][y].type;
}

Tile getTile(int x, int y)
{
    Tile tile;

    sem_wait(tiles_semaphore);

    tile = pMap->tiles[x][y];

    sem_post(tiles_semaphore);

    return tile;
}

void increaseTileContamination(int x, int y, float increment)
{
    sem_wait(tiles_semaphore);

    pMap->tiles[x][y].contamination = min(pMap->tiles[x][y].contamination + increment, 1.0f);

    sem_post(tiles_semaphore);
}

const char* getCitizenTypeName(CitizenType type)
{
    switch (type)
    {
    case ORDINARY_PEOPLE:
        return "ORDINARY";

    case DOCTOR:
        return "DOCTOR";

    case FIREFIGHTER:
        return "FIREFIGHTER";

    case JOURNALIST:
        return "JOURNALIST";
    
    default:
        return "ERROR";
    }
}

void lockCitizen(const Citizen* pCitizen)
{
    sem_wait(citizen_semaphores[pCitizen->id]);
}

void unlockCitizen(const Citizen* pCitizen)
{
    sem_post(citizen_semaphores[pCitizen->id]);
}

Citizen* getCitizen(uint8_t id)
{
    return &pMap->citizens[id];
}

uint32_t getCitizenCount(int x, int y)
{
    uint32_t count = 0;

    for (uint8_t i = 0; i < CITIZEN_COUNT; i++)
    {
        const Citizen* pCitizen = getCitizen(i);

        lockCitizen(pCitizen);

        if (pCitizen->x == x && pCitizen->y == y)
        {
            count++;
        }

        unlockCitizen(pCitizen);
    }

    return count;
}

bool canAccess(Citizen* pCitizen, int x, int y)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
        return false;

    const Tile tile = getTile(x, y);
    const uint32_t citizenCount = getCitizenCount(x, y);

    if (getMaximumCapacity(tile.type) == citizenCount)
    {
        if (tile.type == HOSPITAL && pCitizen->status == SICK)
        {
            lockCitizen(pCitizen);
            pCitizen->wantToEnterHospital = true;
            unlockCitizen(pCitizen);
        }

        return false;
    }

    lockCitizen(pCitizen);
    const bool wantToEnterHospital = pCitizen->wantToEnterHospital;
    unlockCitizen(pCitizen);

    if (wantToEnterHospital && tile.type != HOSPITAL)
    {
        return false;
    }

    switch(tile.type)
    {
    case HOSPITAL:
        return pCitizen->status == SICK || (pCitizen->type == DOCTOR && pCitizen->data[DAY_OUT_OF_HOSPITAL] == 0) || pCitizen->type == FIREFIGHTER;

    case FIRE_STATION:
        if (pCitizen->type == FIREFIGHTER)
            return true;

        for (uint8_t i = FIREFIGTHER_OFFSET; i < (FIREFIGTHER_OFFSET + FIREFIGHTER_COUNT); i++)
        {
            const Citizen* pFirefighter = getCitizen(i);

            if (pFirefighter->x == x && pFirefighter->y == y)
                return true;
        }

        return false;

    case HOUSE:
    case WASTELAND:
        return true;

    default:
        return false;
    }
}

bool moveCitizen(Citizen* pCitizen, int xDest, int yDest)
{
    if (pCitizen->x == xDest && pCitizen->y == yDest)
        return false;

    if (canAccess(pCitizen, xDest, yDest))
    {
        lockCitizen(pCitizen);

        pCitizen->x = xDest;
        pCitizen->y = yDest;

        unlockCitizen(pCitizen);

        return true;
    }

    return false;
}