#ifndef DOCTOR_H
#define DOCTOR_H

#include "Map.h"

#include <stdbool.h>

/**
 * @brief Maximum number of care pocket that a doctor can carry
 */
#define MAX_CARE_POCKET 10

/**
 * @brief Find the sick citizen with the highest contamination rate at the 
 * given position
 * 
 * @param x 
 * @param y 
 * 
 * @return sick citizen or NULL if no sick citizen is present 
 */
Citizen* findPatient(int x, int y);

/**
 * @brief Update doctor
 * 
 * The doctor will find a patient on his tile and heal him if he has a care 
 * pocket.
 * 
 * If a doctor is sick he cannot heal patient but he can heal himself if he
 * has a care pocket and he has been ill for less than 10 days.
 * 
 * A doctor doesn't need care pocket if he is in an hospital.
 * 
 * @param pDoctor 
 * @param tile 
 */
void updateDoctor(Citizen* pDoctor, Tile tile);

#endif