/*******************************************************************************
 * @file    Pwm_Cfg.c
 * @brief   File định nghĩa bảng cấu hình kênh PWM (Pwm_Cfg) cho STM32F103 SPL.
 *
 * @details FCấu hình pwm
 *
 * @author  Duy Dang
 * @date    29-06-2026
 * @version v1.0.0
 ******************************************************************************/

/*******************************************************************************
 * Khai báo include
 ******************************************************************************/
#include "Pwm_Config.h"

/*******************************************************************************
 * Bảng cấu hình từng kênh PWM
 ******************************************************************************/
const Pwm_ChannelConfigType PwmChannelConfig[PWM_NUM_OF_CHANNELS] =
{
    /* Cấu hình TIM1_CH1 */
    {
        .channelId            = PWM_CHANNEL_1_TIM1,
        .timerId               = PWM_TIMER_1,
        .timerChannel           = 0U,   /* CH1 */
        .defaultPeriod          = 7199U, /* f_pwm  */
        .defaultDuty             = PWM_DUTY_CYCLE_0_PERCENT,
        .outputState              = PWM_LOW,
        .idleState                = PWM_LOW,
        .channelClass             = PWM_VARIABLE_PERIOD,
        .polarity                  = PWM_HIGH,
        .notificationCallback     = NULL_PTR
    }
};

/*******************************************************************************
 * Cấu hình tổng thể PWM
 ******************************************************************************/
const Pwm_ConfigType Pwm_Config =
{
    .channelConfigPtr = PwmChannelConfig,
    .numOfChannels    = PWM_NUM_OF_CHANNELS
};