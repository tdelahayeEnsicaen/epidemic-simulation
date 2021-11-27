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
sem_t* citizens_semaphore;

void genTileMap();
void genCitizens();

void createMap()
{
    fd = shm_open("/map", O_RDWR | O_CREAT, 0666);
    ftruncate(fd, sizeof(struct Map));

    pMap = mmap(NULL, sizeof(struct Map), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    tiles_semaphore = sem_open("/tiles", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 1);
    citizens_semaphore = sem_open("/citizens", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 1);

    genTileMap();
    genCitizens();
}

void loadMap()
{
    fd = shm_open("/map", O_RDWR, 0666);

    if (fd == -1)
    {
        printf("ERROR\n");
        return;
    }

    pMap = mmap(NULL, sizeof(struct Map), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    tiles_semaphore = sem_open("/tiles", O_RDWR);
    citizens_semaphore = sem_open("/citizens", O_RDWR);
}

void destroyMap()
{
    sem_destroy(tiles_semaphore);
    sem_destroy(citizens_semaphore);

    sem_unlink("/tiles");
    sem_unlink("/citizens");

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

#define DOCTOR_OFFSET 0
#define FIREFIGTHER_OFFSET (DOCTOR_OFFSET + DOCTOR_COUNT)
#define JOURNALIST_OFFSET (FIREFIGTHER_OFFSET + FIREFIGHTER_COUNT)
#define ORDINARY_PEOPLE_OFFSET (JOURNALIST_OFFSET + JOURNALIST_COUNT)

void createCitizen(int id, char type, int x, int y, int data)
{
    pMap->citizens[id].type = type;
    pMap->citizens[id].alive = 1;
    pMap->citizens[id].sick = 0;
    pMap->citizens[id].burned = 0;
    pMap->citizens[id].dayOfSickness = 0;
    pMap->citizens[id].contamination = 0.0f;
    pMap->citizens[id].data = data;

    moveCitizen(id, x, y);
}

void genCitizens()
{
    srand(time(NULL));

    // TODO Should required people be created before others ?

    // DOCTORS

    createCitizen(0, DOCTOR, 3, 3, 5);

    for (int i = DOCTOR_OFFSET + 1; i < (DOCTOR_OFFSET + DOCTOR_COUNT); i++)
    {
        const int x = rand() % MAP_WIDTH;
        const int y = rand() % MAP_HEIGHT;

        createCitizen(i, DOCTOR, x, y, 5); // TODO if in hospital set to 10

        moveCitizen(i, x, y);
    }

    // FIRE FIGTHERS

    createCitizen(4, FIREFIGHTER, 6, 0, 1000);

    createCitizen(5, FIREFIGHTER, 0, 6, 1000);

    for (int i = FIREFIGTHER_OFFSET + 2; i < (FIREFIGTHER_OFFSET + FIREFIGHTER_COUNT); i++)
    {
        const int x = rand() % MAP_WIDTH;
        const int y = rand() % MAP_HEIGHT;

        createCitizen(i, FIREFIGHTER, x, y, 500);
    }

    // JOURNALISTS

    for (int i = JOURNALIST_OFFSET; i < (JOURNALIST_OFFSET + JOURNALIST_COUNT); i++)
    {
        const int x = rand() % MAP_WIDTH;
        const int y = rand() % MAP_HEIGHT;

        createCitizen(i, JOURNALIST, x, y, 0);
    }

    // ORDINARY PEOPLE

    for (int i = ORDINARY_PEOPLE_OFFSET; i < (ORDINARY_PEOPLE_OFFSET + ORDINARY_PEOPLE_COUNT); i++)
    {
        const int x = rand() % MAP_WIDTH;
        const int y = rand() % MAP_HEIGHT;

        createCitizen(i, ORDINARY_PEOPLE, x, y, 0);
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

void setTileContamination(int x, int y, float contamination)
{
    sem_wait(tiles_semaphore);

    pMap->tiles[x][y].contamination = contamination;

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

Citizen getCitizen(int index)
{
    Citizen citizen;

    sem_wait(citizens_semaphore);

    citizen = pMap->citizens[index];

    sem_post(citizens_semaphore);

    return citizen;
}

int getCitizenCount(int x, int y)
{
    int count = 0;

    for (int i = 0; i < CITIZEN_COUNT; i++)
    {
        sem_wait(citizens_semaphore);
        if (pMap->citizens[i].x == x && pMap->citizens[i].y == y)
            count++;
        sem_post(citizens_semaphore);
    }

    return count;
}

int canAccess(Citizen citizen, int x, int y)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
        return 0;

    const Tile tile = getTile(x, y);
    const int citizenCount = getCitizenCount(x, y);

    if (getMaximumCapacity(tile.type) == citizenCount)
    {
        return 0;
    }

    switch(tile.type)
    {
    case HOSPITAL:
        return citizen.sick || citizen.type == DOCTOR || citizen.type == FIREFIGHTER;

    case FIRE_STATION:
        if (citizen.type == FIREFIGHTER)
            return 1;

        for (int i = FIREFIGTHER_OFFSET; i < (FIREFIGTHER_OFFSET + FIREFIGHTER_COUNT); i++)
        {
            const Citizen firefighter = getCitizen(i);

            if (firefighter.x == x && firefighter.y == y)
                return 1;
        }

        return 0;

    case HOUSE:
        return 1;

    case WASTELAND:
        return 1;

    default:
        return 0;
    }
}

int moveCitizen(int citizenId, int xDest, int yDest)
{
    const Citizen citizen = getCitizen(citizenId);

    if (citizen.x == xDest && citizen.y == yDest)
        return 1;

    if (canAccess(citizen, xDest, yDest))
    {
        sem_wait(citizens_semaphore);
        pMap->citizens[citizenId].x = xDest;
        pMap->citizens[citizenId].y = yDest;
        sem_post(citizens_semaphore);
        return 1;
    }

    return 0;
}

void setCitizenContamination(int citizenId, float contamination)
{
    if (contamination < 0.0f)
        contamination = 0.0f;
    else if (contamination > 1.0f)
        contamination = 1.0f;

    sem_wait(citizens_semaphore);
    pMap->citizens[citizenId].contamination = contamination;
    sem_post(citizens_semaphore);
}

void setCitizenSick(int citizenId, char sick)
{
    sem_wait(citizens_semaphore);
    pMap->citizens[citizenId].sick = sick;
    sem_post(citizens_semaphore);
}

void setCitizenBurned(int citizenId, char burned)
{
    sem_wait(citizens_semaphore);
    pMap->citizens[citizenId].burned = burned;
    sem_post(citizens_semaphore);
}

void setCitizenDayOfSickness(int citizenId, char dayCount)
{
    sem_wait(citizens_semaphore);
    pMap->citizens[citizenId].dayOfSickness = dayCount;
    sem_post(citizens_semaphore);
}

void setCitizenAlive(int citizenId, char alive)
{
    sem_wait(citizens_semaphore);
    pMap->citizens[citizenId].alive = alive;
    sem_post(citizens_semaphore);
}

void setCitizenData(int citizenId, int data)
{
    sem_wait(citizens_semaphore);
    pMap->citizens[citizenId].data = data;
    sem_post(citizens_semaphore);
}

void saveMap(FILE* pFile)
{
    sem_wait(citizens_semaphore);

    for (int i = 0; i < CITIZEN_COUNT; i++)
    {
        fprintf(pFile, "%s X=%d Y=%d Alive=%d Sick=%d DOS=%d Cont=%.2f Data=%d\n", 
            getCitizenTypeName(pMap->citizens[i].type),
            pMap->citizens[i].x,
            pMap->citizens[i].y,
            pMap->citizens[i].alive,
            pMap->citizens[i].sick,
            pMap->citizens[i].dayOfSickness,
            pMap->citizens[i].contamination,
            pMap->citizens[i].data);
    }

    sem_post(citizens_semaphore);

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