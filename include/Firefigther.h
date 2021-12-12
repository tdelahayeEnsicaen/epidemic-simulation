#ifndef FIREFIGHTER_H
#define FIREFIGHTER_H

#include "Map.h"

#define PULVERISATOR_CAPACITY 10.0f
/**
 * @brief Amount of pulverisator that can be used by a firefighter by turn
 */
#define PULVERISATOR_BY_TURN 1.0f
/**
 * @brief Amount of pulverisator that can be used on a single citizen by turn
 */
#define PULVERISATOR_BY_CITIZEN 0.20f
/**
 * @brief Amount of pulverisator that can be used on a single tile by turn
 */
#define PULVERISATOR_BY_TILE 0.20f

/**
 * @brief Burn all dead bodies on the same tile as the given firefigther
 * 
 * @param pFirefighter 
 */
void burnDeadBody(const Citizen* pFirefighter);

void injectPulverisator(Citizen* pFirefighter, float value);

float extractPulverisator(Citizen* pFirefighter, float value);

void decontaminate(Citizen* pFirefighter);

#endif