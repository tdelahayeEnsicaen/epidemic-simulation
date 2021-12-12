#ifndef CITIZEN_H
#define CITIZEN_H

#include "Map.h"

#include <stdbool.h>

// ----------------------- MOVEMENT -----------------------

/**
 * @brief Chance for a citizen to move at each turn
 */
#define CITIZEN_MOVE_PROBABILITY 0.40f

/**
 * @brief Update citizen position.
 * 
 * If the citizen if dead then this method will do nothing and return false.
 * 
 * The citizen has a 40% chance of moving on an adjacent tile at each turn. If
 * the citizen cannot access to any adjacent tile this method will return false.
 * 
 * @param pCitizen Citizen to update
 * 
 * @return true if the citizen has moved
 */
bool updatePosition(Citizen* pCitizen);

// -------------------- CONTAMINATION ---------------------

/**
 * @brief Percent of citizen contamination send to the tile when entering.
 */
#define CONTAMINATION_SENT 0.01f
/**
 * @brief Percent of tile contamination send to the citizen when entering on
 * the tile.
 */
#define CONTAMINATION_RECEIVED_ON_MOVEMENT 0.02f
/**
 * @brief Percent of tile contamination send to the citizen when staying on the
 * tile.
 */
#define CONTAMINATION_RECEIVED_WITHOUT_MOVEMENT 0.05f
/**
 * @brief Percent of incoming contamination received by a firefigther.
 */
#define FIREFIGHTER_MODIFIER 0.10f
/**
 * @brief Percent of incoming contamination received when citizen is in hospital.
 */
#define HOSPITAL_CONTAMINATION_MODIFIER 0.25f
/**
 * @brief Percent of incoming contamination received when citizen is in fire 
 * station.
 */
#define FIRE_STATION_CONTAMINATION_MODIFIER 0.00f
/**
 * @brief Citizen contamination decrement when on fire station
 */
#define FIRE_STATION_CONTAMINATION_DECREMENT 0.20f

/**
 * @brief Exchange contamination between the citizen and the tile.
 * 
 * Tile contamination will increased by 1% of citizen contamination if citizen
 * has moved since the last turn.
 * 
 * Citizen contamination will increased by 2% of tile contamination if he has
 * moved since the last turn and else by 5% of tile contamination.
 * 
 * Firefighter only received 10% of incoming contamination.
 * 
 * When the citizen is in hospital he only received 25% of incoming 
 * contamination.
 * 
 * When the citizen is in fire station he only received 0% of incoming 
 * contamination.
 * 
 * @param pCitizen 
 * @param contamination Citizen contamination level
 * @param tile 
 * @param hasMoved True if the citizen has moved since the last turn
 */
void exchangeContaminationWithTile(Citizen* pCitizen, float contamination, Tile tile, bool hasMoved);

/**
 * @brief Chance for a sick person or his dead body (if not burned) to 
 * contaminate a citizen in the same place.
 */
#define NEARBY_CONTAMINATION_CHANCE 0.10f
/**
 * @brief Chance for a sick person or his dead body (if not burned) who is on a
 * wasteland tile to contaminate a citizen on adjacent wasteland tile.
 */
#define DISTANT_CONTAMINATION_CHANCE 0.01f
/**
 * @brief Chance for a firefighter to avoid a contamination.
 */
#define FIREFIGHTER_IMMUNIZATION_CHANCE 0.70f
/**
 * @brief Contamination increment applied to a citizen when contaminate by an 
 * other citizen
 */
#define CONTAMINATION_INCREMENT 0.01f

/**
 * @brief Check if the given citizen can contaminate other citizens
 * 
 * A citizen can contaminate if he is sick or dead and his dead body was not
 * burned
 * 
 * @param pSource 
 * 
 * @return true if the citizen can contaminate
 */
bool canContaminate(const Citizen* pSource);

/**
 * @brief Check if a source citizen can contaminate a target citizen.
 * 
 * Source citizen has a 10% chance of infecting target citizen if in the 
 * same place.
 * If source and target citizens are on wasteland tiles source citizen has
 * a 1% chance of infecting target citizen.
 * 
 * Firefighter has a 70% chance of been immune to a contamination.
 * 
 * Source citizen cannot contaminate himself.
 * 
 * @param pSource Contamination source
 * @param pDest Contamination target
 * 
 * @return true if source citizen can contaminate target citizen
 */
bool canContaminateTarget(const Citizen* pSource, const Citizen* pTarget);

/**
 * @brief Propagate the contamination of a source citizen
 * 
 * If source citizen cannot contaminate this method will do nothing else then
 * this method will try to propagate the source citizen to others citizens.
 * 
 * @param pSource Contamination source
 * 
 * @see canContaminate
 * @see canContaminateTarget
 */
void propagateContaminationToCitizens(const Citizen* pSource);

// ----------------------- SICKNESS -----------------------

/**
 * @brief Base risk for a citizen to die of his illness at each turn
 */
#define BASE_RISK_OF_DYING 0.05f
/**
 * @brief Divisor applied to the risk of dying when the sick citizen is in a 
 * hospital
 */
#define HOSPITAL_DIVISOR 4.0f
/**
 * @brief Divisor applied to the risk of death when a doctor is on the same
 * tile as a sick citizen
 */
#define DOCTOR_DIVISOR 2.0f

/**
 * @brief Compute the probability that the citizen dies of his illness
 * 
 * A citizen has a 5% risk of dying of his disease at each turn divided by 4 in
 * a hospital and by 2 when a doctor is present
 * 
 * @param pCitizen Sick citizen
 *  
 * @return float 
 */
float computeRiskOfDying(const Citizen* pCitizen);

void updateSickness(Citizen* pCitizen);

#endif