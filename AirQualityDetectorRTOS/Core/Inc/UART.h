/*
 * UART.h
 *
 *  Created on: Apr 11, 2024
 *      Author: Stefan.Milojevic
 */

#ifndef INC_UART_H_
#define INC_UART_H_


#include "main.h"



//@brief Transmits a string trough UART
//@param str String to be transmitted
void UART_TransmitString(char* str);

//@brief Transmits a string trough UART without \n
//@param str String to be transmitted
void UART_TransmitStringWithoutNewLine(char* str);

//@brief Transmits float trough uart
//@param data Data to be transmitted
void UART_TransmitFloat(float data);

//@brief Function to send list of commands to the user
//@param NumOfElements Number of elements in the string
//@param CommandMatrix Matrix that holds command list
void HelpSendUART(uint16_t NumOfElements, char** CommandMatrix);


#endif /* INC_UART_H_ */
