#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>

#define TURN_LIMIT 100

// TUBE NAMES

#define SIM_TO_TIMER_NAME "./sim_to_timer"
#define TIMER_TO_SIM_NAME "./timer_to_sim"

#define SIM_TO_CITIZEN_NAME "./sim_to_citizen"
#define CITIZEN_TO_SIM_NAME "./citizen_to_sim"

enum ProcessAction
{
    INIT,
    UPDATE,
    DESTROY
};

/**
 * @brief Try to create a tube with the given name if the operation fails then 
 * the process will automaticaly exit.
 * 
 * @param name 
 * @param flags 
 */
void createTube(const char* name, int flags);

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
 * 
 * If the running process is epidemic_sim then tubes will be first created 
 * before being opened.
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
 * If the running process is epidemic_sim then tubes will be destroyed.
 */
void closeTubes();

/**
 * @brief Send action to a process through requestTube and wait the result by
 * reading resultTube.
 * 
 * @param requestTube 
 * @param resultTube 
 * @param action 
 * 
 * @return true if the action has been successfully completed
 */
bool sendAction(int requestTube, int resultTube, enum ProcessAction action);

/**
 * @brief Read an action from the given tube
 * 
 * @param tube Tube to read
 * 
 * @return an action
 */
enum ProcessAction readAction(int tube);

/**
 * @brief Send the action result through the given tube
 * 
 * @param tube 
 * @param result 
 */
void sendResult(int tube, bool result);

#endif