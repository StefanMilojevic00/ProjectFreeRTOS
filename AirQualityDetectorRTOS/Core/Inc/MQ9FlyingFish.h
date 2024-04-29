/*
 * MQ9FlyingFish.h
 *
 *  Created on: Apr 11, 2024
 *      Author: Stefan.Milojevic
 */

#ifndef INC_MQ9FLYINGFISH_H_
#define INC_MQ9FLYINGFISH_H_

#include "main.h"



//Structure for holding senosor configuration
typedef struct
{
	GPIO_TypeDef *gpioPort; //GPIO port fot MQ9
	uint16_t dataPin; //MQ9 analog pin
} MQ9_t;

//@brief Funtion to initialize MQ9 sensor
//@param MQ9 Structure holding the configuration
//@param gpioPort MQ9 port
//@param dataPin MQ9 pin
void MQ9_init(MQ9_t* MQ9, GPIO_TypeDef* gpioPort, uint16_t dataPin);

//@brief Funtion to read raw sensor value
//@param MQ9 Structure holding the configuration
uint16_t MQ9_GetRawValue();

//@brief Function to convert raw value to real value
//@param rawValue Value to be converted
//@retval Real sensor value
float MQ9_ConverRaw2Real(uint16_t rawValue);


#endif /* INC_MQ9FLYINGFISH_H_ */
