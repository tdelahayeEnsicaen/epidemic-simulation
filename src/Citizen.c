#include "Citizen.h"

#include "Utils.h"

#include <stdlib.h>

// ----------------------- MOVEMENT -----------------------

bool updatePosition(Citizen* pCitizen)
{
    if (pCitizen->status >= DEAD)
        return false;

    Tile tile = getTile(pCitizen->x, pCitizen->y);

    if (tile.type == HOSPITAL)
    {
        pCitizen->wantToEnterHospital = false;

        if (pCitizen->status == HEALTHY)
        {  
            // check if a sick citizen want to enter in hospital
            for (uint8_t i = 0; i < CITIZEN_COUNT; i++)
            {
                Citizen* pOther = getCitizen(i);
                lockCitizen(pOther);

                if (pOther->wantToEnterHospital && (pOther->x != pCitizen->x || pOther->y != pCitizen->y))
                {
                    uint8_t tempX = pCitizen->x;
                    uint8_t tempY = pCitizen->y;

                    pCitizen->x = pOther->x;
                    pCitizen->y = pOther->y;

                    pOther->x = tempX;
                    pOther->y = tempY;

                    return true;
                }
                unlockCitizen(pOther);
            }
        }
    }

    if (genFloat() < CITIZEN_MOVE_PROBABILITY)
    {
        Point directions[8];
        genDirList(directions);

        for (int i=0; i < 8; i++)
        {
            if (moveCitizen(pCitizen, pCitizen->x + directions[i].x, pCitizen->y + directions[i].y))
                return true;
        }
    }

    return false;
}

// -------------------- CONTAMINATION ---------------------

void exchangeContaminationWithTile(Citizen* pCitizen, float contamination, Tile tile, bool hasMoved)
{
    if (hasMoved)
    {
        increaseTileContamination(pCitizen->x, pCitizen->y, contamination * CONTAMINATION_SENT);
    }

    float contaminationRate;

    if (hasMoved)
    {
        contaminationRate = CONTAMINATION_RECEIVED_ON_MOVEMENT;
    }
    else
    {
        contaminationRate = CONTAMINATION_RECEIVED_WITHOUT_MOVEMENT;
    }
    
    if (pCitizen->type == FIREFIGHTER)
    {
        contaminationRate *= FIREFIGHTER_MODIFIER;
    }

    switch (tile.type)
    {
    case HOSPITAL:
        contaminationRate *= HOSPITAL_CONTAMINATION_MODIFIER;
        break;
    case FIRE_STATION:
        contaminationRate *= FIRE_STATION_CONTAMINATION_MODIFIER;
        break;
    default:
        break;
    }

    lockCitizen(pCitizen);
    pCitizen->contamination = min(pCitizen->contamination + tile.contamination * contaminationRate, 1.0f);
    unlockCitizen(pCitizen);
}

bool canContaminate(const Citizen* pSource)
{
    switch (pSource->status)
    {
    case SICK:
    case DEAD:
        return true;
    
    case HEALTHY:
    case BURNED:
    default:
        return false;
    }
}

bool canContaminateTarget(const Citizen* pSource, const Citizen* pTarget)
{
    bool canContaminate;

    if (pSource == pTarget)
    {
        canContaminate = false;
    }
    else if (pSource->x == pTarget->x && pSource->y == pTarget->y)
    {
        canContaminate = genFloat() < NEARBY_CONTAMINATION_CHANCE;
    }
    else if (abs(pSource->x - pTarget->x) <= 1 && abs(pSource->y - pTarget->y) <= 1)
    {
        const Tile sourceTile = getTile(pSource->x, pSource->y);
        const Tile targetTile = getTile(pTarget->x, pTarget->y);

        if (sourceTile.type == WASTELAND && targetTile.type == WASTELAND)
        {
            canContaminate = genFloat() < DISTANT_CONTAMINATION_CHANCE;
        }
        else
        {
            canContaminate = false;
        }
    }
    else
    {
        canContaminate = false;
    }
    
    if (canContaminate && pTarget->type == FIREFIGHTER)
    {
        canContaminate = genFloat() >= FIREFIGHTER_IMMUNIZATION_CHANCE;
    }

    return canContaminate;
}

void propagateContaminationToCitizens(const Citizen* pSource)
{
    if (!canContaminate(pSource))
        return;

    for (int i = 0; i < CITIZEN_COUNT; i++)
    {
        Citizen* pTarget = getCitizen(i);

        if (canContaminateTarget(pSource, pTarget))
        {
            lockCitizen(pTarget);
            pTarget->contamination = min(pTarget->contamination + CONTAMINATION_INCREMENT, 1.0f);
            unlockCitizen(pTarget);
        }
    }
}

// ----------------------- SICKNESS -----------------------

float computeRiskOfDying(const Citizen* pCitizen)
{
    if (getTileType(pCitizen->x, pCitizen->y) == HOSPITAL)
    {
        return BASE_RISK_OF_DYING / HOSPITAL_DIVISOR;
    }

    if (pCitizen->type == DOCTOR)
        return BASE_RISK_OF_DYING / DOCTOR_DIVISOR;
    
    for (uint8_t i = 0; i < DOCTOR_COUNT; i++)
    {
        const Citizen* pDoctor = getCitizen(DOCTOR_OFFSET + i);

        lockCitizen(pDoctor);

        if (pDoctor->status < DEAD && pDoctor->x == pCitizen->x && pDoctor->y == pCitizen->y)
        {
            unlockCitizen(pDoctor);
            return BASE_RISK_OF_DYING / DOCTOR_DIVISOR;
        }

        unlockCitizen(pDoctor);
    }
    
    return BASE_RISK_OF_DYING;
}

void updateSickness(Citizen* pCitizen)
{
    lockCitizen(pCitizen);
    CitizenStatus status = pCitizen->status;
    unlockCitizen(pCitizen);

    switch (status)
    {
    case HEALTHY:
        if (genFloat() < pCitizen->contamination)
        {
            lockCitizen(pCitizen);
            pCitizen->dayOfSickness = 0;
            pCitizen->status = SICK;
            unlockCitizen(pCitizen);
        }
        break;

    case SICK:
        lockCitizen(pCitizen);
        if (pCitizen->dayOfSickness > 5 && genFloat() < computeRiskOfDying(pCitizen))
        {
            pCitizen->status = DEAD;
        }

        pCitizen->dayOfSickness++;
        unlockCitizen(pCitizen);
    
    default:
        break;
    }
}