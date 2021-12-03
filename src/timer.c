#include "Process.h"

#include <stdlib.h>

#include <stdio.h>
#include <signal.h>

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define ARG_COUNT 2
#define USAGE_FORMAT "%s [Period(in seconds)]\n"

int turnCounter = 0;
int period;

#define SIM_TO_TIMER 0
#define TIMER_TO_SIM 1

int tubes[2];

void openTubes()
{
    printf("[TIM] Open tubes\n");

    tubes[SIM_TO_TIMER] = openTube(SIM_TO_TIMER_NAME, O_RDONLY);
    tubes[TIMER_TO_SIM] = openTube(TIMER_TO_SIM_NAME, O_WRONLY);
}

void closeTubes()
{
    printf("[TIM] Close tubes\n");

    close(tubes[SIM_TO_TIMER]);
    close(tubes[TIMER_TO_SIM]);
}

void tick(int sig)
{
    printf("[TIM] Update %d/%d\n", turnCounter+1, TURN_LIMIT);
    sig = sig;

    enum ProcessAction action = UPDATE;
    write(tubes[TIMER_TO_SIM], &action, sizeof(action));

    bool result;
    read(tubes[SIM_TO_TIMER], &result, sizeof(bool));
    
    turnCounter++;

    if (!result)
    {
        action = DESTROY;
        write(tubes[TIMER_TO_SIM], &action, sizeof(action));
        closeTubes();
        exit(EXIT_FAILURE);
    }
    else if (turnCounter == TURN_LIMIT)
    {
        action = DESTROY;
        write(tubes[TIMER_TO_SIM], &action, sizeof(action));
        closeTubes();
        exit(EXIT_SUCCESS);
    }
    else
    {
        alarm(period);
    }
}

int main(int argc, char const *argv[])
{
    printf("[TIM] Start\n");

    if (argc != ARG_COUNT)
    {
        fprintf(stderr, USAGE_FORMAT, argv[0]);
        return EXIT_FAILURE;
    }

    sscanf(argv[1], "%d", &period);

    struct sigaction alarm_action;

    memset(&alarm_action, 0, sizeof(struct sigaction));

    alarm_action.sa_handler = &tick;

    sigaction(SIGALRM, &alarm_action, NULL);

    openTubes();

    printf("[TIM] Initialization\n");

    enum ProcessAction action = INIT;
    write(tubes[TIMER_TO_SIM], &action, sizeof(action));

    bool result;
    read(tubes[SIM_TO_TIMER], &result, sizeof(bool));

    printf("[TIM] Running\n");

    alarm(period);

    for(;;);

    return EXIT_SUCCESS;
}
