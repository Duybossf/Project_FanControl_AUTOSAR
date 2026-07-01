/*******************************************************************************
 * @file    Port_Config.c
 * @brief   Port driver configuration source file.
 *
 * @author  Duy Dang
 * @date    24-06-2026
 * @version v1.0.0
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "Port_Config.h"

/*******************************************************************************
 * Static Variables
 ******************************************************************************/
const Port_PinConfigType Port_ConfigPins[PORT_NUM_PIN_CONFIG] =
{
    /* Cấu hình LED PC13. */
    {
        .portID = PORT_C,
        .pinID = 13U,
        .pinMode = PORT_PIN_MODE_DIO,
        .profileMode = PORT_PIN_OUT_PP,
        .pinSpeed = GPIO_Speed_2MHz,
        .pinDirection = PORT_PIN_OUT,
        .levelPin = PORT_PIN_LEVEL_HIGH,
        .changeDirection = 0U,
        .changeMode = 0U
    },

    /* Cấu hình ADC chân PA0 */
    {
        .portID = PORT_A,
        .pinID = 0U,
        .pinMode = PORT_PIN_MODE_ADC,
        .profileMode = PORT_PIN_IN_AIN,
        .pinSpeed = GPIO_Speed_50MHz,
        .pinDirection = PORT_PIN_IN,
        .levelPin = PORT_PIN_LEVEL_HIGH,
        .changeDirection = 0U,
        .changeMode = 0U
    },

    /* Cấu hình PWM PA8 Tim1 và Channel 1*/
    {
        .portID = PORT_A,
        .pinID = 8U,
        .pinMode = PORT_PIN_MODE_PWM,
        .profileMode = PORT_PIN_OUT_AFPP,
        .pinSpeed = GPIO_Speed_50MHz,
        .pinDirection = PORT_PIN_OUT,
        .levelPin = PORT_PIN_LEVEL_HIGH,
        .changeDirection = 0U,
        .changeMode = 0U
    }
};

const Port_ConfigType Port_Config =
{
    .pinConfig = Port_ConfigPins,
    .numOfPin = PORT_NUM_PIN_CONFIG
};
