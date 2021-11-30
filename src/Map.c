#include "Map.h"

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

    tiles_semaphore = sem_open("/tiles", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 1);

    char name[12];

    for (int i = 0; i < CITIZEN_COUNT; i++)
    {
        sprintf(name, "/citizen_%d", i);
        citizen_semaphores[i] = sem_open("/citizens", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 1);
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
        printf("ERROR\n");
        return;
    }

    pMap = mmap(NULL, sizeof(struct Map), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    tiles_semaphore = sem_open("/tiles", O_RDWR);

    char name[12];

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

void createCitizen(int id, char type, int x, int y, char data[4])
{
    pMap->citizens[id].type = type;
    pMap->citizens[id].alive = 1;
    pMap->citizens[id].sick = 0;
    pMap->citizens[id].burned = 0;
    pMap->citizens[id].dayOfSickness = 0;
    pMap->citizens[id].contamination = 0.0f;

    memcpy(pMap->citizens[id].data, data, 4);

    moveCitizen(&pMap->citizens[id], x, y);
}

void genCitizens()
{
    srand(time(NULL));

    // TODO Should required people be created before others ?

    // DOCTORS

    char data[4] = { 5, 0, 0, 0 };

    createCitizen(0, DOCTOR, 3, 3, data);

    for (int i = DOCTOR_OFFSET + 1; i < (DOCTOR_OFFSET + DOCTOR_COUNT); i++)
    {
        const int x = rand() % MAP_WIDTH;
        const int y = rand() % MAP_HEIGHT;

        createCitizen(i, DOCTOR, x, y, data); // TODO if in hospital set to 10
    }

    // FIRE FIGTHERS

    float pulverisator = 10.0f;
    memcpy(data, &pulverisator, sizeof(float));

    createCitizen(4, FIREFIGHTER, 6, 0, data);

    createCitizen(5, FIREFIGHTER, 0, 6, data);

    pulverisator = 5.0f;
    memcpy(data, &pulverisator, sizeof(float));

    for (int i = FIREFIGTHER_OFFSET + 2; i < (FIREFIGTHER_OFFSET + FIREFIGHTER_COUNT); i++)
    {
        const int x = rand() % MAP_WIDTH;
        const int y = rand() % MAP_HEIGHT;

        createCitizen(i, FIREFIGHTER, x, y, data);
    }

    // JOURNALISTS

    memset(data, 0, 4);

    for (int i = JOURNALIST_OFFSET; i < (JOURNALIST_OFFSET + JOURNALIST_COUNT); i++)
    {
        const int x = rand() % MAP_WIDTH;
        const int y = rand() % MAP_HEIGHT;

        createCitizen(i, JOURNALIST, x, y, data);
    }

    // ORDINARY PEOPLE

    for (int i = ORDINARY_PEOPLE_OFFSET; i < (ORDINARY_PEOPLE_OFFSET + ORDINARY_PEOPLE_COUNT); i++)
    {
        const int x = rand() % MAP_WIDTH;
        const int y = rand() % MAP_HEIGHT;

        createCitizen(i, ORDINARY_PEOPLE, x, y, data);
    }
}

int getMaximumCapacity(char tileType)
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

char getTileType(int x, int y)
{
    char type;

    sem_wait(tiles_semaphore);

    type = pMap->tiles[x][y].type;

    sem_post(tiles_semaphore);

    return type;
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

    pMap->tiles[x][y].contamination += increment;

    sem_post(tiles_semaphore);
}

char* getCitizenTypeName(char type)
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

Citizen* getCitizen(int id)
{
    return &pMap->citizens[id];
}

int getCitizenCount(int x, int y)
{
    int count = 0;

    for (int i = 0; i < CITIZEN_COUNT; i++)
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

bool canAccess(const Citizen* pCitizen, int x, int y)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
        return false;

    const Tile tile = getTile(x, y);
    const int citizenCount = getCitizenCount(x, y);

    if (getMaximumCapacity(tile.type) == citizenCount)
    {
        return false;
    }

    switch(tile.type)
    {
    case HOSPITAL:
        return pCitizen->sick || pCitizen->type == DOCTOR || pCitizen->type == FIREFIGHTER;

    case FIRE_STATION:
        if (pCitizen->type == FIREFIGHTER)
            return true;

        for (int i = FIREFIGTHER_OFFSET; i < (FIREFIGTHER_OFFSET + FIREFIGHTER_COUNT); i++)
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
        return true;

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

void saveMap(FILE* pFile)
{
    for (int i = 0; i < CITIZEN_COUNT; i++)
    {
        const Citizen* pCitizen = getCitizen(i);

        lockCitizen(pCitizen);

        if (pCitizen->type == FIREFIGHTER)
        {
            float data;
            memcpy(&data, pCitizen->data, sizeof(float));

            fprintf(pFile, "%s X=%d Y=%d Alive=%d Sick=%d DOS=%d Cont=%.2f Data=%f\n", 
            getCitizenTypeName(pCitizen->type),
            pCitizen->x,
            pCitizen->y,
            pCitizen->alive,
            pCitizen->sick,
            pCitizen->dayOfSickness,
            pCitizen->contamination,
            data);
        }
        else
        {
            fprintf(pFile, "%s X=%d Y=%d Alive=%d Sick=%d DOS=%d Cont=%.2f Data=%d\n", 
            getCitizenTypeName(pCitizen->type),
            pCitizen->x,
            pCitizen->y,
            pCitizen->alive,
            pCitizen->sick,
            pCitizen->dayOfSickness,
            pCitizen->contamination,
            pCitizen->data[0]);
        }

        unlockCitizen(pCitizen);
    }

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            fprintf(pFile, "%d ", getCitizenCount(x, y));
        }

        fprintf(pFile, "\n");
    }

    sem_wait(tiles_semaphore);

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            fprintf(pFile, "%.2f ", pMap->tiles[x][y].contamination);
        }

        fprintf(pFile, "\n");
    }

    sem_post(tiles_semaphore);

    fflush(pFile);
}