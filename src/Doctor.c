#include "Doctor.h"

Citizen* findPatient(int x, int y)
{
    float contamination = 0.0f;
    Citizen* pPatient = NULL;

    for (int i=0; i < CITIZEN_COUNT; i++)
    {
        Citizen* pCandidate = getCitizen(i);

        if (pCandidate->alive && pCandidate->sick && pCandidate->x == x && pCandidate->y == y && pCandidate->contamination > contamination)
        {
            pPatient = pCandidate;
        }
    }
    
    return pPatient;
}

void updateDoctor(Citizen* pDoctor, Tile tile)
{
    // TODO move to epidemic simulation
    if (tile.type == HOSPITAL)
    {
        lockCitizen(pDoctor);
        pDoctor->data[0] = MAX_CARE_POCKET;
        unlockCitizen(pDoctor);
    }

    Citizen* pPatient;

    lockCitizen(pDoctor);

    if (pDoctor->sick)
    {
        pPatient = pDoctor->dayOfSickness < 10 ? pDoctor : NULL;
        unlockCitizen(pDoctor);
    }
    else
    {
        unlockCitizen(pDoctor);
        pPatient = findPatient(pDoctor->x, pDoctor->y);
    }

    if (pPatient)
    {
        if (tile.type == HOSPITAL)
        {
            lockCitizen(pPatient);
            pPatient->sick = false;
            unlockCitizen(pPatient);
        }
        else if (pDoctor->data[0] > 0)
        {
            lockCitizen(pPatient);
            pDoctor->data[0]--;
            pPatient->sick = false;
            unlockCitizen(pPatient);
        }
    }
}