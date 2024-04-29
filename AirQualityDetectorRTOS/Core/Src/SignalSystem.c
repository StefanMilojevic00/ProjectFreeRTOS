/*
 * SignalSystem.c
 *
 *  Created on: Apr 12, 2024
 *      Author: Stefan.Milojevic
 */
#include <stdio.h> //just in case
#include "SignalSystem.h"



static Button_t button;

void SignalSystemInit()
{
	CreateNewButton(&button, Btn_pin_GPIO_Port, Btn_pin_Pin);
}

bool ReadSignal(bool* readEnable)
{
	bool Status = ReadButton(&button, readEnable);
	return Status;
}

