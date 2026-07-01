/**
 * @file Port_Config.h
 *
 * @brief Configuration to mapping MCU to serves Port.h
 *
 * @author Duy Dang
 *
 * @date 23-06-2026
 *
 * @version v1.0.0
 */

#ifndef PORT_CONFIG_H
#define PORT_CONFIG_H

#include <Std_Types.h>
#include "Port.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

/* Linear DIO port identifiers for STM32 GPIO ports. */
#define PORT_A (0U)
#define PORT_B (1U)
#define PORT_C (2U)
#define PORT_D (3U)

/* Macro ánh xạ Port ID → GPIO của STM32 */
#define PORT_GET_GPIO(portID)        \
    ((portID) == PORT_A) ? GPIOA :   \
    ((portID) == PORT_B) ? GPIOB :   \
    ((portID) == PORT_C) ? GPIOC :   \
    ((portID) == PORT_D) ? GPIOD :   \
    NULL_PTR

/* Xác định Pin trong Port vì mỗi Pin sẽ chiếm 1 bit bắt đầu từ bit0 cho PIN_0 trong stm32f10x_gpio.h*/
#define PORT_PIN_ID(pinID) (1U << (pinID))

/* Xác định các mode được config. */
#define PORT_PIN_MODE_DIO   (0U)
#define PORT_PIN_MODE_ADC   (1U)
#define PORT_PIN_MODE_PWM   (2U)

/* Xác định cấu hình khi bật mode DIO: TH Output*/
#define PORT_PIN_OUT_PP (0U)
#define PORT_PIN_OUT_OD (1U)
/* Xác định cấu hình khi bật mode DIO: TH Input*/
#define PORT_PIN_IN_IPD (2U)
#define PORT_PIN_IN_IPU (3U)
/* Xác định cấu hình khi bật mode ADC: TH Input*/
#define PORT_PIN_IN_AIN (4U)
/* Xác định cấu hình khi bật mode PWM: TH Input*/
#define PORT_PIN_OUT_AFPP (5U)

/*Xác định trạng thái một Pin*/
#define PORT_PIN_LEVEL_LOW      (0U)
#define PORT_PIN_LEVEL_HIGH     (1U)

/**********************************************************
 * Macro phiên bản, vendor, module ID
 **********************************************************/
#define PORT_VENDOR_ID 181U
#define PORT_MODULE_ID 81U
#define PORT_SW_MAJOR_VERSION 1U
#define PORT_SW_MINOR_VERSION 0U
#define PORT_SW_PATCH_VERSION 0U

extern const Port_ConfigType Port_Config;
extern const Port_PinConfigType Port_ConfigPins[];

#define PORT_NUM_PIN_CONFIG    (3U)

#endif /* PORT_CONFIG_H */
