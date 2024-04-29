/*
 * UART_CommandSystem.c
 *
 *  Created on: Apr 22, 2024
 *      Author: Stefan.Milojevic
 */


#include "UART_CommandSystem.h"
#include "UART.h"


extern UART_HandleTypeDef huart1;
extern char* cmdStrings[];

static uint8_t RxIndex;
static uint8_t RxData[1];
static uint8_t RxBuffer[200];
static bool transferCompleteFlag = false;



void TerminalHandler(UART_HandleTypeDef *huart)
{
	uint8_t newLine[] = "\n\r";
	uint8_t backspace[] = "\b  \b";

	if(huart->Instance == USART1)
	{
		if(RxIndex == 0)
		{
			ClearRxBuffer();
		}
		if(RxData[0] != 13) //if it is not carrige return
		{
			if(RxData[0] == 8 && RxIndex > 0) //If user presses backspace
			{
				RxBuffer[RxIndex] = '\0';
				RxIndex--;
				HAL_UART_Transmit(huart, backspace, sizeof(backspace), TIMEOUT_UART);
			}
			else
			{
				RxBuffer[RxIndex] = RxData[0];
				RxIndex++;
			}
			HAL_UART_Transmit(huart, RxData, sizeof(RxData), TIMEOUT_UART);
		}
		else
		{
			RxBuffer[RxIndex] = RxData[0];
			RxIndex = 0; // This will clear the buffer
			transferCompleteFlag = true;
			HAL_UART_Transmit(huart, newLine, sizeof(newLine), TIMEOUT_UART);
		}
	}
	HAL_UART_Receive_IT(huart, RxData, sizeof(RxData)); //Start again so it will read it always
}

void TerminalInit(UART_HandleTypeDef *huart) //Function reads 1 data byte
{
	HAL_UART_Receive_IT(huart, RxData, sizeof(RxData));
	ClearRxBuffer();
}

void ClearRxBuffer()
{
	for(int counter = 0; counter < 200; counter++)
	{
		RxBuffer[counter] = 0;
	}
	transferCompleteFlag = false;
}

bool IsTransferComplete()
{
	return transferCompleteFlag;
}

char* GetRxBuffer()
{
	return (char*)RxBuffer;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 	//Interrupt function defined by Cube
{
	TerminalHandler(huart); 	//TerminalHandler becomes a callback function

}


bool StringCompareFromUART(char* cmd_string, char** matrix,  uint16_t numOfStrings, uint16_t* index)
{

	    uint16_t count = 0;
	    bool foundIt = false;
	    bool retval;
	    cmd_string = strupr(cmd_string);
	    while((count < numOfStrings) && (foundIt == false))
	    {

	        if(strcmp(cmd_string, matrix[count]) == 0)
	        {
	            *index = count;
	            foundIt = true;

	        }
	        count++;
	    }

	    if(foundIt)
	    {
	        retval = true;
	    }
	    else
	    {
	        retval = false;
	    }

	    return retval;

}
