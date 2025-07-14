#include <stdbool.h>                    // Defines true
#include <stdlib.h>
#include <stdio.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "../Irrigation_System.X/moisture_sensor.h"
#include "../Irrigation_System.X/moisture_calibration.h"
#include "../Irrigation_System.X/LCD1602A.h"

typedef enum {
    STATE_IDLE,
    STATE_INIT,
    STATE_RUNNING,
    STATE_ERROR,
    STATE_STANDBY
} State_t;
// *****************************************************************************
// *****************************************************************************
// Section:Definitions
// *****************************************************************************
// *****************************************************************************
#define RX_BUFFER_SIZE 20
#define MAX_STATES          5          // Number of states in the state machine
#define DEBOUNCE_TIME_MS    50         // Debounce time in milliseconds
#define NUMADCMEASUREMENTSTOAVERAGE 16
#define NUMLIGHTSENSORMEASUREMENTSTOBUFFER 20 
#define MOISTURELEVELTHRESHOLD 2300
#define ADC_VREF                (1650)   //1650 mV (1.65V)
// *****************************************************************************
// *****************************************************************************
// Section: Global variable Set-Up
// *****************************************************************************
// *****************************************************************************

const char messageStart [] ="**** USART startup message *****\r\n";
const char State_Change []=         "State changed from ";
const char STATE_IDLE_message[] =   "System in IDLE State";
const char STATE_INIT_message[]=    "System in INIT State";
const char STATE_RUNNING_message[]= "System in RUNNING State";
const char STATE_ERROR_message[]=   "System in ERROR State";
const char STATE_STANDBY_message[]= "System in STANDBY State";
const char newline [] = "\r\n";
const char errorMessage[] = "\r\n***USART error has occured***\r\n";
const char StateMessage[] = "\r\n Moved to stage:";

/**********************************
 * Local function prototypes      *
 **********************************/
//void handle_command(char command);
void Check_Commands (void);
void Clear_buffer(void);
void execute_state_actions(void);
void handle_button_press(void);
void Interval1mS(TC_TIMER_STATUS status, uintptr_t context);
