#include "Process.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

void createTube(const char* name, int flags)
{
    unlink(name);

    int result = mkfifo(name, 0644);

    if (result == -1)
    {
        fprintf(stderr, "Failed to create tube: %s\n", name);
        exit(EXIT_FAILURE);
    }
}

int openTube(const char* name, int flags)
{
    int tube = open(name, flags);

    if (tube == -1)
    {
        fprintf(stderr, "Failed to open tube: %s\n", name);
        exit(EXIT_FAILURE);
    }

    return tube;
}

void closeTube(const char* name, int tube, bool destroy)
{
    close(tube);

    if (destroy)
    {
        unlink(name);
    }
}

bool sendAction(int requestTube, int resultTube, enum ProcessAction action)
{
    bool result;

    write(requestTube, &action, sizeof(enum ProcessAction));

    read(resultTube, &result, sizeof(bool));

    return result;
}

enum ProcessAction readAction(int tube)
{
    enum ProcessAction action;

    read(tube, &action, sizeof(enum ProcessAction));

    return action;
}

void sendResult(int tube, bool result)
{
    write(tube, &result, sizeof(bool));
}