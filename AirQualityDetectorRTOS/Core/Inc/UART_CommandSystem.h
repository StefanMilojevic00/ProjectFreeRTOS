/*
 * UART_CommandSystem.h
 *
 *  Created on: Apr 22, 2024
 *      Author: Stefan.Milojevic
 */

#ifndef INC_UART_COMMANDSYSTEM_H_
#define INC_UART_COMMANDSYSTEM_H_

#include "main.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


#define TIMEOUT_UART 10

//@brief Function to clear the reciving buffer
void ClearRxBuffer();

//@brief Function to initialize UART transmision
//@param huart UART handler defined by mx
void TerminalInit(UART_HandleTypeDef *huart);


//@brief Function to handle the transmision
//@param huart UART handler defined by mx
void TerminalHandler(UART_HandleTypeDef *huart);

//@brief Checks is the tramsmision is complete
//@retval bool true if it is, false if it is not
bool IsTransferComplete();

//@brief Function to return the pointer to the Rx data
//@retval uint8_t* Pointer to the Rx data
char* GetRxBuffer();

//@brief Functiont to compare a command from Uart with the system command
//@param cmp_cmd Command from UART
//@param cmdStrings Matrix containing all possible commands
//@param array_element Total number of commands
//@param ret_val Index of specific command in matrix
//@retval bool Returns true if the command exists
bool StringCompareFromUART(char* cmd_string, char** matrix,  uint16_t numOfStrings, uint16_t* index);

#endif /* INC_UART_COMMANDSYSTEM_H_ */
