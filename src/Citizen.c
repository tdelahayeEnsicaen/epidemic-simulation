#include "Citizen.h"

#include "Utils.h"

// ----------------------- MOVEMENT -----------------------

bool updatePosition(Citizen* pCitizen)
{
    if (!pCitizen->alive)
        return 0;

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
        // TODO move to EpidemicSim
        lockCitizen(pCitizen);
        pCitizen->contamination = pCitizen->contamination > 0.20f ? pCitizen->contamination - 0.20f : 0.0f;
        unlockCitizen(pCitizen);
        break;
    default:
        break;
    }

    lockCitizen(pCitizen);
    pCitizen->contamination += tile.contamination * contaminationRate;
    unlockCitizen(pCitizen);
}

bool canContaminate(const Citizen* pSource)
{
    return pSource->sick && pSource->burned;
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
            pTarget->contamination += CONTAMINATION_INCREMENT;
            unlockCitizen(pTarget);
        }
    }
}

// ----------------------- SICKNESS -----------------------

int computeRiskOfDying(const Citizen* pCitizen)
{
    const Tile tile = getTile(pCitizen->x, pCitizen->y);

    if (tile.type == HOSPITAL)
    {
        return BASE_RISK_OF_DYING / HOSPITAL_DIVISOR;
    }
    
    for (int i = 0; i < DOCTOR_COUNT; i++)
    {
        const Citizen* pDoctor = getCitizen(DOCTOR_OFFSET + i);

        lockCitizen(pDoctor);

        if (pDoctor->alive && pDoctor->x == pCitizen->x && pDoctor->y == pCitizen->y)
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
    if (pCitizen->alive)
    {
        if (!pCitizen->sick && genFloat() < pCitizen->contamination)
        {
            pCitizen->dayOfSickness = 0;
            pCitizen->sick = true;
        }

        if (pCitizen->sick)
        {
            if (pCitizen->dayOfSickness > 5 && genFloat() < computeRiskOfDying(pCitizen))
            {
                pCitizen->alive = false;
            }

            pCitizen->dayOfSickness++;
        }
    }
}