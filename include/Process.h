/**
 * @file Process.h
 * @brief Common header of the different processes (epidemic_sim, citizen_manager, press_agency and timer).
 * Each process must define the following functions in its own source code:
 *  - parseArguments
 *  - initialize
 *  - update
 *  - destroy
 *  - getProcessName
 *  - getPreviousProcess
 *  - getNextProcesses
 */
#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>

#define TURN_LIMIT 100

// TUBE NAMES

#define SIM_TO_TIMER_NAME "./sim_to_timer"
#define TIMER_TO_SIM_NAME "./timer_to_sim"

#define SIM_TO_CITIZEN_NAME "./sim_to_citizen"
#define CITIZEN_TO_SIM_NAME "./citizen_to_sim"

#define SIM_TO_PRESS_NAME "./sim_to_press"
#define PRESS_TO_SIM_NAME "./press_to_sim"

// QUEUE NAMES

#define JOURNALISTS_TO_PRESS_NAME "/journalists_to_press"

typedef enum
{
    INIT,
    UPDATE,
    DESTROY,
    NONE = - 1
} ProcessAction;

typedef struct 
{
    const char* name;
    int id;
} Tube;

typedef struct
{
    Tube input;
    Tube output;
} Process;

// ------------------ PROCESS LIFE CYCLE ------------------

ProcessAction getNextAction(ProcessAction previousAction, bool previousActionResult);

bool parseArguments(int argc, char const *argv[]);

bool initialize();

bool update();

bool destroy();

// ----------------- PROCESS INFORMATION ------------------

const char* getProcessName();

/**
 * @brief Return the process that should be executed before the running process
 * if NULL then the running process will be considered as the main process
 * 
 * @return a pointer to a Process struct or NULL 
 */
Process* getPreviousProcess();

/**
 * @brief Return an array of process that should before executed after the 
 * running process.
 * 
 * @param pSize Where the size of the array will be stored
 * 
 * @return an array of Process struct or NULL
 */
Process* getNextProcesses(int* pSize);

// ----------------- PROCESS COMMUNICATION ----------------

/**
 * @brief Try to create a tube with the given name if the operation fails then 
 * the process will automaticaly exit.
 * 
 * @param name 
 */
void createTube(const char* name);

/**
 * @brief Try to open a tube with the given name if the operation fails then 
 * the process will automaticaly exit.
 * 
 * @param name Tube name
 * @param flags Tube flags
 * 
 * @return tube identifier
 */
int openTube(const char* name, int flags);

/**
 * @brief Try to open all required tubes to make this process communicate with
 * others process if the operation fails this process will automaticaly exit.
 */
void openTubes();

/**
 * @brief Destroy the given tube
 * 
 * If destroy is set to true then the tube will be destroyed.
 * 
 * @param name Tube name
 * @param tube Tube id
 * @param destroy Define if the tube should be destroyed
 */
void closeTube(const char* name, int tube, bool destroy);

/**
 * @brief Close all tubes previously opened by this process.
 * 
 * If the running process is the main process then tubes will be destroyed.
 */
void closeTubes();

/**
 * @brief Send action to a process through its input tube and wait the result 
 * by reading its output tube.
 * 
 * @param process
 * @param action 
 * 
 * @return true if the action has been successfully completed
 */
bool sendAction(Process process, ProcessAction action);

/**
 * @brief Read an action from the given tube
 * 
 * @param tube Tube to read
 * 
 * @return an action
 */
ProcessAction readAction(Tube tube);

/**
 * @brief Send the action result through the given tube
 * 
 * @param tube 
 * @param result 
 */
void sendResult(Tube tube, bool result);

#endif