#include "Process.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char const *argv[])
{
    printf("[%s] Start\n", getProcessName());

    if (!parseArguments(argc, argv))
        return EXIT_FAILURE;

    openTubes();

    Process* pPreviousProcess;

    int nextProcessesCount;
    Process* pNextProcesses;

    pPreviousProcess = getPreviousProcess();
    pNextProcesses = getNextProcesses(&nextProcessesCount);

    bool isRunning = true;

    ProcessAction action = NONE;
    bool result = true;

    while (isRunning)
    {
        action = getNextAction(action, result);

        switch(action)
        {
        case INIT:
            printf("[%s] Initialization\n", getProcessName());
            result = initialize();
            printf("[%s] Running\n", getProcessName());

            for (int i = 0; i < nextProcessesCount && result; i++)
            {
                result &= sendAction(pNextProcesses[i], INIT);
            }
            break;

        case UPDATE:
            //printf("[%s] Update\n", getProcessName());
            result = update();

            for (int i = 0; i < nextProcessesCount && result; i++)
            {
                result &= sendAction(pNextProcesses[i], UPDATE);
            }
            break;

        case DESTROY:
            for (int i = 0; i < nextProcessesCount; i++)
            {
                result &= sendAction(pNextProcesses[i], DESTROY);
            }

            result = destroy();

            printf("[%s] Destroy\n", getProcessName());

            isRunning = false;
            break;

        default:
            printf("[%s] Error: invalid action %d\n", getProcessName(), action);
            result = false;
            break;
        }

        if (pPreviousProcess)
        {
            sendResult(pPreviousProcess->input, result);
        }
    }

    closeTubes();

    return EXIT_SUCCESS;
}

// ------------------ PROCESS LIFE CYCLE ------------------

ProcessAction getNextAction(ProcessAction previousAction, bool previousActionResult)
{
    Process* pPreviousProcess = getPreviousProcess();

    if (pPreviousProcess)
    {
        return readAction(pPreviousProcess->output);
    }
    else
    {
        if (!previousActionResult && previousAction != DESTROY)
        {
            return DESTROY;
        }

        static int turnCounter = 0;

        switch (previousAction)
        {
        case NONE:
            return INIT;
        case INIT:
            return UPDATE;

        case UPDATE:
            turnCounter++;
            return turnCounter == TURN_LIMIT ? DESTROY : UPDATE;

        case DESTROY:
            fprintf(stderr, "[%s] Error: invalid state\n", getProcessName());
            exit(EXIT_FAILURE);
        
        default:
            fprintf(stderr, "[%s] Error: invalid action %d\n", getProcessName(), previousAction);
            exit(EXIT_FAILURE);
        }
    }
}

// ----------------- PROCESS COMMUNICATION ----------------

void createTube(const char* name)
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

void openTubes()
{
    Process* pPreviousProcess;

    int nextProcessesCount;
    Process* pNextProcesses;

    pPreviousProcess = getPreviousProcess();

    if (pPreviousProcess)
    {
        pPreviousProcess->input.id = openTube(pPreviousProcess->input.name, O_WRONLY);
        pPreviousProcess->output.id = openTube(pPreviousProcess->output.name, O_RDONLY);
    }

    pNextProcesses = getNextProcesses(&nextProcessesCount);

    for (int i = 0; i < nextProcessesCount; i++)
    {
        createTube(pNextProcesses[i].output.name);
        createTube(pNextProcesses[i].input.name);
    }

    for (int i = 0; i < nextProcessesCount; i++)
    {
        pNextProcesses[i].output.id = openTube(pNextProcesses[i].output.name, O_RDONLY);
        pNextProcesses[i].input.id = openTube(pNextProcesses[i].input.name, O_WRONLY);
    }
}

void closeTube(const char* name, int tube, bool destroy)
{
    close(tube);

    if (destroy)
    {
        unlink(name);
    }
}

void closeTubes()
{
    Process* pPreviousProcess;

    int nextProcessesCount;
    Process* pNextProcesses;

    pPreviousProcess = getPreviousProcess();

    if (pPreviousProcess)
    {
        closeTube(pPreviousProcess->input.name, pPreviousProcess->input.id, true);
        closeTube(pPreviousProcess->output.name, pPreviousProcess->output.id, true);
    }

    pNextProcesses = getNextProcesses(&nextProcessesCount);

    for (int i = 0; i < nextProcessesCount; i++)
    {
        closeTube(pNextProcesses[i].input.name, pNextProcesses[i].input.id, false);
        closeTube(pNextProcesses[i].output.name, pNextProcesses[i].output.id, false);
    }
}

bool sendAction(Process process, ProcessAction action)
{
    bool result;

    write(process.input.id, &action, sizeof(ProcessAction));

    read(process.output.id, &result, sizeof(bool));

    return result;
}

ProcessAction readAction(Tube tube)
{
    ProcessAction action;

    read(tube.id, &action, sizeof(ProcessAction));

    return action;
}

void sendResult(Tube tube, bool result)
{
    write(tube.id, &result, sizeof(bool));
}