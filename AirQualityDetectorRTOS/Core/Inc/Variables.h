/*
 * Variables.h
 *
 *  Created on: Apr 12, 2024
 *      Author: Stefan.Milojevic
 */

#ifndef INC_VARIABLES_H_
#define INC_VARIABLES_H_

#include <stdbool.h>
#include <stdint.h>
#include "LED_Handler.h"
#include "GasSensor.h"
#include "AlarmSystem.h"
#include "UART.h"
#include "SignalSystem.h"
#include "AirQualityIndicator.h"
#include "UART_CommandSystem.h"
#include "main.h"


//FSM to handle the states in which the system works
typedef enum {P_IDLE_START, P_IDLE, P_WORK_S1, P_WORK_S3, P_WORK_S5} ProgramStateFSM;

//FSM for IDLE to WORK regime movement
typedef enum {PC_IDLE, PC_WORK_START, PC_WORK, PC_WORK_CHECK} ProgramChangeStateFSM;

//FSM to control single LED
typedef enum {LED_OFF, LED_ON_CORRECT, LED_ON_INCORRECT} LED_StatusFSM;

//FSM to handle system work state based on the command input from UART
typedef enum {cmd_0, cmd_1, cmd_2, cmd_3, cmd_4, cmd_5, cmd_6, cmd_7, cmd_8, cmd_9, cmd_10,cmd_11,
			  cmd_12, cmd_13, cmd_14, cmd_15, cmd_16, cmd_17, cmd_18, cmd_19, cmd_20} UART_commandsFSM;
			  
//@brief Function to initialize all of the systems peripherals
void DevicesInit();

#endif /* INC_VARIABLES_H_ */
