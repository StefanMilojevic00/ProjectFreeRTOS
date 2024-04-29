/*
 * GasSensor.c
 *
 *  Created on: Apr 11, 2024
 *      Author: Stefan.Milojevic
 */
/*
#define MQ9_GPIO_Port <Port goes here if needed>
#define MQ9_GPIO_Pin PA0
 */

#include "GasSensor.h"
#include "MQ9FlyingFish.h"



void InitGasSenosor() //Not needed for our sensor as it is made ready to meassure on launch
{
	//MQ9_init(&MQSetting, MQ_GPIO_Port, MQ9_GPIO_Pin);
}

float ReadGasSensor()
{
	uint16_t rawValue = MQ9_GetRawValue();
	float realValue = MQ9_ConverRaw2Real(rawValue);
	return realValue;
}
