/*
 * LED.h
 *
 *  Created on: Apr 15, 2024
 *      Author: Marko.Manic
 */

#ifndef INC_LED_HANDLER_H_
#define INC_LED_HANDLER_H_

#include "main.h"
#include "LED_Driver.h"



//@brief Function to initialize the single LED
void LED_init();

//@brief Function to drive the single LED based on the state of the overall system
//@param state State of the system - true for working correct, false othervise
void LED_Drive(bool state);


#endif /* INC_LED_HANDLER_H_ */
