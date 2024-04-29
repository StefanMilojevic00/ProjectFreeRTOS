/*
 * MQ9FlyingFish.c
 *
 *  Created on: Apr 11, 2024
 *      Author: Stefan.Milojevic
 */


#include "MQ9FlyingFish.h"



extern ADC_HandleTypeDef hadc3;

static float DigitalLevels = 4096.0; // ADC has 12 bit resolution so 2^12 = 4096
static float SensorRange = 4000.0; //MQ9 meassures from 0 - 4000, subject to change

void MQ9_init(MQ9_t* MQ9, GPIO_TypeDef* gpioPort, uint16_t dataPin)
{
	MQ9->gpioPort = gpioPort;
	MQ9->dataPin = dataPin;
}

uint16_t MQ9_GetRawValue()
{
	HAL_ADC_Start(&hadc3);
	HAL_ADC_PollForConversion(&hadc3, HAL_MAX_DELAY);
	uint16_t raw = HAL_ADC_GetValue(&hadc3);
	return raw;
}

float MQ9_ConverRaw2Real(uint16_t rawValue)
{
	float scale = SensorRange / DigitalLevels;
	float realValue = rawValue * scale;
	return realValue;
}
