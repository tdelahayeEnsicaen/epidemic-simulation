#ifndef CITIZEN_MANAGER_H
#define CITIZEN_MANAGER_H

void waitSignal();

void nextStep();

void *ordinaryPeopleHandler(void* pArg);

void *doctorHandler(void* pArg);

void *firefighterHandler(void* pArg);

void *journalistHandler(void* pArg);

#endif