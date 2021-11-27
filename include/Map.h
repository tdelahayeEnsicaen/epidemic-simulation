#ifndef MAP_H
#define MAP_H

#include <stdio.h>

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
    char type;
    char x, y;
    char alive;
    char sick;
    char burned;
    char dayOfSickness;
    float contamination;
    int data;
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

void setTileContamination(int x, int y, float contamination);

// --------- CITIZENS ---------

char* getCitizenTypeName(char type);

Citizen getCitizen(int index);

/**
 * @brief Return the number of citizen at the given position
 * 
 * @param x 
 * @param y 
 * @return number of citizen
 */
int getCitizenCount(int x, int y);

int canAccess(Citizen citizen, int x, int y);

/**
 * @brief Try to move the given citizen to the destination tile.
 * This method fails if the destination tile is full or if the destination tile
 * is a fire station and no firefighter is present and the citizen isn't a 
 * firefighter.
 * 
 * @param citizenId
 * @param xDest 
 * @param yDest 
 * @return 1 if the method success 0 else
 */
int moveCitizen(int citizenId, int xDest, int yDest);

void setCitizenContamination(int citizenId, float contamination);

void setCitizenSick(int citizenId, char sick);

void setCitizenBurned(int citizenId, char burned);

void setCitizenDayOfSickness(int citizenId, char dayCount);

void setCitizenAlive(int citizenId, char alive);

void setCitizenData(int citizenId, int data);

// -------- SAVE ---------

void saveMap(FILE* pFile);

#endif