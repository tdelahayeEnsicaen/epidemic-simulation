#include "PressAgency.h"
#include "Process.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>

#define JOURNALISTS_TO_PRESS 0

int queues[1];

// ----------------- PROCESS INFORMATION ------------------

Process previousProcess =
{
    .input = { .name = PRESS_TO_SIM_NAME },
    .output = { .name = SIM_TO_PRESS_NAME }
};

const char* getProcessName()
{
    return "PRESS";
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

bool parseArguments(int argc, char const *argv[])
{
    argc = argc;
    argv = argv;
    return true;
}

bool initialize()
{
    mq_unlink(JOURNALISTS_TO_PRESS_NAME);
    queues[JOURNALISTS_TO_PRESS] = mq_open(JOURNALISTS_TO_PRESS_NAME, O_RDONLY | O_CREAT, 0644, NULL);

    return queues[JOURNALISTS_TO_PRESS] != -1;
}

bool update()
{
    struct mq_attr attr;
    int result;
    result = mq_getattr(queues[JOURNALISTS_TO_PRESS], &attr);

    while (attr.mq_curmsgs && result != -1)
    {
        char* buffer = malloc(attr.mq_msgsize);

        mq_receive(queues[JOURNALISTS_TO_PRESS], buffer, attr.mq_msgsize, NULL);

        printMessage(buffer[0], buffer+1);

        result = mq_getattr(queues[JOURNALISTS_TO_PRESS], &attr);

        free(buffer);
    }

    return true;
}

bool destroy()
{
    mq_close(queues[JOURNALISTS_TO_PRESS]);
    mq_unlink(JOURNALISTS_TO_PRESS_NAME);

    return true;
}

void printMessage(NewsType type, char* msg)
{
    switch (type)
    {
    case DEAD_COUNT:
        printf("[PRESS] Dead count: %d\n", (int) (msg[0] * 0.65f));
        break;

    case AVERAGE_CONTAMINATION:
        printf("[PRESS] Avg contamination: %f\n", *((float*)msg) * 0.90f);
        break;

    case CONTAMINED_CITIZENS:
        printf("[PRESS] Contamined citizens: %d\n", (int) (msg[0] * 0.90f));
        break;

    case JOURNALIST_CONTAMINATION:
        if (*((float*)msg) > 0.80f)
            printf("[PRESS] Journalist contamination: %f\n", *((float*)msg));
        break;
    
    default:
        break;
    }
}