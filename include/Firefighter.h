#ifndef FIREFIGHTER_H
#define FIREFIGHTER_H

#include "Map.h"

#define PULVERISATOR_BY_TURN 1.0f

#define PULVERISATOR_BY_CITIZEN 0.20f

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