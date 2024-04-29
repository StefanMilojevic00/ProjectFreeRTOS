/*
 * AlarmSystem.c
 *
 *  Created on: Apr 11, 2024
 *      Author: Stefan.Milojevic
 */

//#define BuzzerPort Buzzer_out_GPIO_Port
//#define BuzzerPin Buzzer_out_Pin

#include "AlarmSystem.h"



static Buzzer_t loadConfiguration;

void AlarmInit()
{
	Buzzer_init(&loadConfiguration, Buzzer_out_GPIO_Port, Buzzer_out_Pin);
}

void AlarmON()
{
	BuzzerWritePin(&loadConfiguration, GPIO_PIN_SET);
}

void AlarmOFF()
{
	BuzzerWritePin(&loadConfiguration, GPIO_PIN_RESET);
}
