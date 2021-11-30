#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <stdbool.h>

// MAP SIZE

#define MAP_WIDTH 7
#define MAP_HEIGHT 7

// TILE TYPES

#define HOSPITAL 0
#define FIRE_STATION 1
#define HOUSE 2
#define WASTELAND 3

// TILE COUNT

#define HOSPITAL_COUNT 1
#define FIRE_STATION_COUNT 2
#define HOUSE_COUNT 12
#define WASTELAND_COUNT ((MAP_WIDTH*MAP_HEIGHT) - HOUSE_COUNT - FIRE_STATION_COUNT - HOSPITAL_COUNT)

// CITIZEN TYPES

#define ORDINARY_PEOPLE 0
#define DOCTOR 1
#define FIREFIGHTER 2
#define JOURNALIST 3

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
    char type;
    float contamination;
} Tile;

typedef struct 
{
    char id;
    char type;
    char x, y;
    bool alive;
    bool sick;
    bool burned;
    char dayOfSickness;
    float contamination;
    char data[4];
} Citizen;

void createMap();

void loadMap();

void destroyMap();

// ---------- TILES -----------

/**
 * @brief Return the maximum number of people who can stand on the given tile 
 * type
 * 
 * @param tileType 
 * @return a positive integer
 */
int getMaximumCapacity(char tileType);

char getTileType(int x, int y);

Tile getTile(int x, int y);

void increaseTileContamination(int x, int y, float increment);

// --------- CITIZENS ---------

char* getCitizenTypeName(char type);

void lockCitizen(const Citizen* pCitizen);

void unlockCitizen(const Citizen* pCitizen);

Citizen* getCitizen(int id);

/**
 * @brief Return the number of citizen at the given position
 * 
 * @param x 
 * @param y 
 * 
 * @return number of citizen
 */
int getCitizenCount(int x, int y);

bool canAccess(const Citizen* pCitizen, int x, int y);

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
 * @return 1 if the method success 0 else
 */
bool moveCitizen(Citizen* pCitizen, int xDest, int yDest);

// -------- SAVE ---------

void saveMap(FILE* pFile);

#endif