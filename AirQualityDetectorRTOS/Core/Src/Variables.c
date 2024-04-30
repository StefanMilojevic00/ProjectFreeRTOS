#include "Variables.h"

extern UART_HandleTypeDef huart1;

void DevicesInit()
{
	AlarmInit();
	SignalSystemInit();
	AirQualityIncicatorInit();
	LED_init();
	TerminalInit(&huart1);
}
