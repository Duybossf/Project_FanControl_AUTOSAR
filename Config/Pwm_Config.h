/*******************************************************************************
 * @file    Pwm.h
 * @brief   File header khai báo API cho module PWM MCAL.
 *
 * @details Khai báo macro để config PWM
 *
 * @author  Duy Dang
 * @date    29-06-2026
 * @version v1.0.0
 ******************************************************************************/

#ifndef PWM_CONFIG_H
#define PWM_CONFIG_H

/*******************************************************************************
 * Khai báo include
 ******************************************************************************/
#include "Pwm.h"
#include "stm32f10x_tim.h"

/*******************************************************************************
 * Thông tin version
 ******************************************************************************/
#define PWM_VENDOR_ID              (0x181U) /* Vendor ID của nhà sản xuất. */
#define PWM_MODULE_ID              (0x01U)   /* Module ID của PWM driver. */
#define PWM_SW_MAJOR_VERSION       (1U) /* Version major của PWM driver. */
#define PWM_SW_MINOR_VERSION       (0U) /* Version minor của PWM driver. */
#define PWM_SW_PATCH_VERSION       (0U) /* Version patch của PWM driver. */

/*******************************************************************************
 * Giá trị duty cycle chuẩn
 ******************************************************************************/
#define PWM_DUTY_CYCLE_0_PERCENT   ((Pwm_DutyCycleType)0x0000U) /* Duty cycle 0%. */
#define PWM_DUTY_CYCLE_100_PERCENT ((Pwm_DutyCycleType)0x8000U) /* Duty cycle 100%. */

/*******************************************************************************
 * Kiểu dữ liệu
 ******************************************************************************/
#define PWM_TIMER_1 (0U) /* Timer 1. */
#define PWM_TIMER_2 (1U) /* Timer 2. */
#define PWM_TIMER_3 (2U) /* Timer 3. */
#define PWM_TIMER_4 (3U) /* Timer 4. */
#define PWM_GET_TIMER_ID(Timer) ((Timer == PWM_TIMER_1) ? TIM1 : \
                                ((Timer == PWM_TIMER_2) ? TIM2 : \
                                ((Timer == PWM_TIMER_3) ? TIM3 : \
                                ((Timer == PWM_TIMER_4) ? TIM4 : NULL_PTR)))) /* Lấy ID timer từ kênh PWM. */

#define PWM_NUM_OF_CHANNELS     (1U) /* Tổng số kênh PWM được cấu hình. */

/*******************************************************************************
 * Macro xác định kênh PWM
 ******************************************************************************/
#define PWM_CHANNEL_1_TIM1 (0U) /* TIM1_CH1/CH1N */
#define PWM_CHANNEL_2_TIM2 (1U) /* TIM2_CH2 */
#define PWM_CHANNEL_3_TIM3 (2U) /* TIM3_CH1 */

/*******************************************************************************
 * Khai báo biến cấu hình 
 ******************************************************************************/
extern const Pwm_ChannelConfigType PwmChannelConfig[PWM_NUM_OF_CHANNELS];
extern const Pwm_ConfigType Pwm_Config;

#endif /* PWM_CONFIG_H */