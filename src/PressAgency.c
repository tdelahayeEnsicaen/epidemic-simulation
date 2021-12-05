#include "Process.h"
#include "Journalist.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>

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

        enum NewsType type = buffer[0];
        float cont;

        switch (type)
        {
        case DEAD_COUNT:
            printf("[PRESS] Dead count: %d\n", (int) (buffer[1] * 0.65f));
            break;

        case AVERAGE_CONTAMINATION:
            memcpy(&cont, buffer + 1, sizeof(float));
            printf("[PRESS] Avg contamination: %f\n", cont * 0.90f);
            break;

        case CONTAMINED_CITIZENS:
            printf("[PRESS] Contamined citizens: %d\n", (int) (buffer[1] * 0.90f));
            break;

        case JOURNALIST_CONTAMINATION:
            memcpy(&cont, buffer + 1, sizeof(float));

            if (cont > 0.80f)
                printf("[PRESS] Journalist contamination: %f\n", cont);
            break;
        
        default:
            break;
        }

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