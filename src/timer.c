#include "Process.h"

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#define ARG_COUNT 2
#define USAGE_FORMAT "%s [Period(in seconds)]\n"

int period;

pthread_mutex_t updateMutex;
pthread_cond_t updateCond;

// ----------------- PROCESS INFORMATION ------------------

Process previousProcess =
{
    .input = { .name = TIMER_TO_SIM_NAME },
    .output = { .name = SIM_TO_TIMER_NAME }
};

const char* getProcessName()
{
    return "TIMER";
}

Process* getPreviousProcess()
{
    return &previousProcess;
}

Process* getNextProcesses(int* pSize)
{
    *pSize = 0;
    return NULL;
}

// ------------------ PROCESS LIFE CYCLE ------------------

void tick(int sig)
{
    sig = sig;

    pthread_mutex_lock(&updateMutex);
    pthread_cond_broadcast(&updateCond);
    pthread_mutex_unlock(&updateMutex);

    alarm(period);
}

bool initialize()
{
    struct sigaction alarm_action;

    memset(&alarm_action, 0, sizeof(struct sigaction));

    alarm_action.sa_handler = &tick;

    pthread_mutex_init(&updateMutex, NULL);
    pthread_cond_init(&updateCond, NULL);

    if (sigaction(SIGALRM, &alarm_action, NULL) != -1)
    {
        alarm(period);
        return true;
    }
    else
    {
        return false;
    }
}

bool update()
{
    pthread_mutex_lock(&updateMutex);
    pthread_cond_wait(&updateCond, &updateMutex);
    pthread_mutex_unlock(&updateMutex);

    return true;
}

bool destroy()
{
    //pthread_cond_destroy(&updateCond);
    pthread_mutex_destroy(&updateMutex);

    return true;
}

bool parseArguments(int argc, char const *argv[])
{
    if (argc != ARG_COUNT)
    {
        fprintf(stderr, USAGE_FORMAT, argv[0]);
        return false;
    }

    sscanf(argv[1], "%d", &period);

    return true;
}
