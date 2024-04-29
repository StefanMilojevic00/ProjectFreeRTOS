/*
 * Button.c
 *
 *  Created on: Apr 10, 2024
 *      Author: Marko.Manic
 */
#include "Button.h"



void CreateNewButton(Button_t* button_instance, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    button_instance->btn_press_flag = false;
    button_instance->read_button_flag = false;
    button_instance->btn_press_detect_flag = false;

    button_instance->btn_last_state = GPIO_PIN_SET;
    button_instance->active_state = GPIO_PIN_SET;
    button_instance->GPIOx = GPIOx;
    button_instance->GPIO_Pin = GPIO_Pin;

    button_instance->button_state = B_IDLE;
}

bool ReadButton(Button_t* button_instance, bool* readEnable)
{
    if (*readEnable)
    {
        GPIO_PinState read_pin_status = HAL_GPIO_ReadPin(button_instance->GPIOx, button_instance->GPIO_Pin);

        switch (button_instance->button_state)
        {
            case B_IDLE:

                if (read_pin_status == button_instance->active_state)
                {
                	button_instance->button_state = B_DEBOUNCE;
                    button_instance->btn_press_detect_flag = false;
                }
                break;

            case B_DEBOUNCE:

            	button_instance->button_state = B_DETECT;
                button_instance->btn_press_detect_flag = false;

                break;

            case B_DETECT:

                if (read_pin_status == button_instance->active_state)
                {
                    button_instance->btn_press_flag = true;
                    button_instance->btn_last_state = read_pin_status;
                    button_instance->btn_press_detect_flag = true;
                    readEnable = false;
                }
                button_instance->button_state = B_DETECT_IDLE;
                break;

            case B_DETECT_IDLE:

            	button_instance->btn_press_detect_flag = false;
            	button_instance->btn_last_state = read_pin_status;
            	if (read_pin_status != button_instance->active_state)
                {
                    button_instance->button_state = B_IDLE;
                }
                break;
        }
    }
    bool retval = button_instance->btn_press_detect_flag;
    return retval;
}



