/*
 * AirQualityIndicator.h
 *
 *  Created on: Apr 15, 2024
 *      Author: Stefan.Milojevic
 */

#include "AirQualityIndicator.h"

static ledRow_t loadConfiguration;


void AirQualityIncicatorInit()
{
	LedRowInit(&loadConfiguration,
					IndicatorGreen_GPIO_Port, IndicatorGreen_Pin,
					IndicatorYellow1_GPIO_Port, IndicatorYellow1_Pin,
					IndicatorYellow2_GPIO_Port, IndicatorYellow2_Pin,
					IndicatorBlue1_GPIO_Port, IndicatorBlue1_Pin,
					IndicatorBlue2_GPIO_Port, IndicatorBlue2_Pin,
					IndicatorBlue3_GPIO_Port, IndicatorBlue3_Pin);
}

void SetIndicatorLEDs(float PPM_Value)
{
	LedRowWrite(&loadConfiguration, PPM_Value);
}

void SetIndicatorLEDsNum(const uint8_t num)
{
	LedRowWriteNum(&loadConfiguration, num);
}

