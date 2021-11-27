#include <pthread.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#define TURN_LIMIT 100

#define ARG_COUNT 2
#define USAGE_FORMAT "%s [Period(in seconds)]\n"

int turnCounter = 0;
int period;
pid_t targetPid;

enum Action
{
    INIT,
    UPDATE,
    DESTROY
};

#define SIM_TO_TIMER 0
#define TIMER_TO_SIM 1

int tubes[2];

int openTube(char* name, int flags)
{
    int tube = open(name, flags);

    if (tube == -1)
    {
        fprintf(stderr, "Failled to open tube: %s\n", name);
        exit(EXIT_FAILURE);
    }

    return tube;
}

void openTubes()
{
    printf("[TIM] Open tubes\n");

    tubes[SIM_TO_TIMER] = openTube("./sim_to_timer", O_RDONLY);
    tubes[TIMER_TO_SIM] = openTube("./timer_to_sim", O_WRONLY);
}

void closeTubes()
{
    printf("[TIM] Close tubes\n");

    close(tubes[SIM_TO_TIMER]);
    close(tubes[TIMER_TO_SIM]);
}

void tick(int sig)
{
    printf("[TIM] Update %d/%d\n", turnCounter, TURN_LIMIT);
    sig = sig;

    enum Action action = UPDATE;
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

    alarm_action.sa_handler = &tick;

    sigaction(SIGALRM, &alarm_action, NULL);

    openTubes();

    printf("[TIM] Initialization\n");

    enum Action action = INIT;
    write(tubes[TIMER_TO_SIM], &action, sizeof(action));

    bool result;
    read(tubes[SIM_TO_TIMER], &result, sizeof(bool));

    printf("[TIM] Running\n");

    alarm(period);

    for(;;);

    return EXIT_SUCCESS;
}
