#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// MAP SIZE

#define MAP_WIDTH 7
#define MAP_HEIGHT 7

// TILE TYPES

typedef enum
{
    HOSPITAL,
    FIRE_STATION,
    HOUSE,
    WASTELAND
} TileType;

// TILE COUNT

#define HOSPITAL_COUNT 1
#define FIRE_STATION_COUNT 2
#define HOUSE_COUNT 12
#define WASTELAND_COUNT ((MAP_WIDTH*MAP_HEIGHT) - HOUSE_COUNT - FIRE_STATION_COUNT - HOSPITAL_COUNT)

// CITIZEN TYPES

typedef enum
{
    ORDINARY_PEOPLE,
    DOCTOR,
    FIREFIGHTER,
    JOURNALIST
} CitizenType;

// CITIZEN STATUS

typedef enum
{
    HEALTHY,
    SICK,
    DEAD,
    BURNED
} CitizenStatus;

// CITIZEN OFFSET

#define DOCTOR_OFFSET 0
#define FIREFIGTHER_OFFSET (DOCTOR_OFFSET + DOCTOR_COUNT)
#define JOURNALIST_OFFSET (FIREFIGTHER_OFFSET + FIREFIGHTER_COUNT)
#define ORDINARY_PEOPLE_OFFSET (JOURNALIST_OFFSET + JOURNALIST_COUNT)

// CITIZEN COUNT

#define ORDINARY_PEOPLE_COUNT 25
#define DOCTOR_COUNT 4
#define FIREFIGHTER_COUNT 6
#define JOURNALIST_COUNT 2
#define CITIZEN_COUNT (ORDINARY_PEOPLE_COUNT + DOCTOR_COUNT + FIREFIGHTER_COUNT + JOURNALIST_COUNT) 

typedef struct
{
    // TileType
    uint8_t type;
    float contamination;
} Tile;

typedef struct 
{
    uint8_t id;
    // CitizenType
    uint8_t type;

    uint8_t x, y;
    // CitizenStatus
    uint8_t status;
    bool wantToEnterHospital;
    bool hasContaminationDetector;
    
    uint8_t dayOfSickness;
    float contamination;
    uint8_t data[4];
} Citizen;

void createMap();

void loadMap();

void destroyMap();

// ---------- TILES -----------

uint8_t getMaximumCapacity(TileType tileType);

TileType getTileType(int x, int y);

Tile getTile(int x, int y);

void increaseTileContamination(int x, int y, float increment);

// --------- CITIZENS ---------

const char* getCitizenTypeName(CitizenType type);

void lockCitizen(const Citizen* pCitizen);

void unlockCitizen(const Citizen* pCitizen);

Citizen* getCitizen(uint8_t id);

uint32_t getCitizenCount(int x, int y);

bool canAccess(Citizen* pCitizen, int x, int y);

/**
 * @brief Try to move the given citizen to the destination tile.
 * 
 * A citizen cannot enter destination tile if the tile is full.
 * 
 * A citizen can enter a fire station only if there is a firefighter or if he 
 * is one.
 * 
 * @param pCitizen
 * @param xDest 
 * @param yDest 
 * @return true if the method success false else
 */
bool moveCitizen(Citizen* pCitizen, int xDest, int yDest);

// -------- SAVE ---------

void saveMap(FILE* pFile);

#endif