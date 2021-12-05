#include "Doctor.h"

Citizen* findPatient(int x, int y)
{
    float contamination = 0.0f;
    Citizen* pPatient = NULL;

    for (uint8_t i=0; i < CITIZEN_COUNT; i++)
    {
        Citizen* pCandidate = getCitizen(i);

        lockCitizen(pCandidate);

        if (pCandidate->status == SICK && pCandidate->x == x && pCandidate->y == y && pCandidate->contamination > contamination)
        {
            contamination = pCandidate->contamination;
            pPatient = pCandidate;
        }

        unlockCitizen(pCandidate);
    }
    
    return pPatient;
}

void updateDoctor(Citizen* pDoctor, Tile tile)
{
    lockCitizen(pDoctor);
    CitizenStatus status = pDoctor->status;
    uint8_t dayOfSickness = pDoctor->dayOfSickness;
    unlockCitizen(pDoctor);

    if (status >= DEAD)
        return;

    if (tile.type == HOSPITAL)
    {
        pDoctor->data[DAY_OUT_OF_HOSPITAL] = 2;
    }
    else if (pDoctor->data[DAY_OUT_OF_HOSPITAL] > 0)
    {
        pDoctor->data[DAY_OUT_OF_HOSPITAL]--;
    }

    Citizen* pPatient;

    if (status == SICK)
    {
        pPatient = dayOfSickness < 10 ? pDoctor : NULL;
    }
    else
    {
        pPatient = findPatient(pDoctor->x, pDoctor->y);
    }

    if (pPatient)
    {
        if (tile.type == HOSPITAL)
        {
            lockCitizen(pPatient);
            pPatient->status = HEALTHY;
            unlockCitizen(pPatient);
        }
        else if (pDoctor->data[CARE_POCKET_INDEX] > 0)
        {
            lockCitizen(pPatient);
            pDoctor->data[CARE_POCKET_INDEX]--;
            pPatient->status = HEALTHY;
            unlockCitizen(pPatient);
        }
    }
}