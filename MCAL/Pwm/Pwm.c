/*******************************************************************************
 * @file    Pwm.c
 * @brief   Mapping sử dụng để cấu hình PWM
 *
 * @author  Duy Dang
 * @date    29-06-2026
 * @version v1.0.0
 ******************************************************************************/

/*******************************************************************************
 * Khai báo include
 ******************************************************************************/
#include "Pwm.h"
#include "Pwm_Config.h"
#include "stm32f10x_rcc.h"

/*******************************************************************************
 * Macro báo thứ tự channel sai
 ******************************************************************************/
#define PWM_INVALID_CHANNEL_INDEX   (0xFFU)

/*******************************************************************************
 * Biến phục vụ và sử dụng báo hiệu trong runtime
 ******************************************************************************/

/** Con trỏ lưu cấu hình hiện hành. */
static const Pwm_ConfigType *Pwm_CurrentConfigPtr = NULL_PTR;

/** Cờ báo module đã được khởi tạo hay chưa. */
static uint8 Pwm_InitStatus = 0U;

/** Mảng lưu duty cycle hiện hành của từng kênh */
static Pwm_DutyCycleType Pwm_CurrentDuty[PWM_NUM_OF_CHANNELS];

/** Mảng lưu trạng thái idle hay đang chạy của từng kênh. */
static uint8 Pwm_ChannelIsIdle[PWM_NUM_OF_CHANNELS];

/** Trạng thái năng lượng hiện tại */
static Pwm_PowerStateType Pwm_CurrentPowerState = PWM_RUN;
static Pwm_PowerStateType Pwm_TargetPowerState  = PWM_RUN;
static uint8 Pwm_PowerStatePrepared = 0U;

/*******************************************************************************
 * Hàm cấu hình
 ******************************************************************************/

/**
 * @brief   Cấu hình chế độ PWM1 cho channel timer tương ứng đã config time-base.
 *
 * @param[in] TIMx          nhận timer qua GET_TIMER_ID.
 * @param[in] TimerChannel  nhận channel timer qua config (0..3).
 * @param[in] CcrValue      giá trị CCR (pulse) đã tính được sau khi config duty.
 * @param[in] Polarity      nhận mức ngõ ra qua config (PWM_HIGH/PWM_LOW).
 */
static void Pwm_ConfigOutputCompare(TIM_TypeDef *Timx,
                                     uint8 TimerChannel,
                                     uint16 CcrValue,
                                     Pwm_PolarityType Polarity)
{
    TIM_OCInitTypeDef ocInitStruct;
    uint16 ocPolarityValue = (Polarity == PWM_HIGH) ? TIM_OCPolarity_High : TIM_OCPolarity_Low;
    uint16 ocNPolarityValue = (Polarity == PWM_HIGH) ? TIM_OCNPolarity_High : TIM_OCNPolarity_Low;

    ocInitStruct.TIM_OCMode       = TIM_OCMode_PWM1; /* PWM1: non-inverting theo tài liệu. */
    ocInitStruct.TIM_OutputState  = TIM_OutputState_Enable;
    ocInitStruct.TIM_OutputNState = TIM_OutputNState_Enable; /* Chỉ có ý nghĩa với TIM1 CHxN. */
    ocInitStruct.TIM_OCPolarity   = ocPolarityValue;
    ocInitStruct.TIM_OCNPolarity  = ocNPolarityValue;
    ocInitStruct.TIM_Pulse        = CcrValue;

    /* Lựa chọn timer và bật preload */
    switch (TimerChannel)
    {
        case 0U:    /* Timer 1*/
            TIM_OC1Init(Timx, &ocInitStruct);
            TIM_OC1PreloadConfig(Timx, TIM_OCPreload_Enable);
            break;

        case 1U:    /* Timer 2*/
            TIM_OC2Init(Timx, &ocInitStruct);
            TIM_OC2PreloadConfig(Timx, TIM_OCPreload_Enable);
            break;

        case 2U:    /* Timer 3*/
            TIM_OC3Init(Timx, &ocInitStruct);
            TIM_OC3PreloadConfig(Timx, TIM_OCPreload_Enable);
            break;

        case 3U:    /* Timer 4*/
            TIM_OC4Init(Timx, &ocInitStruct);
            TIM_OC4PreloadConfig(Timx, TIM_OCPreload_Enable);
            break;

        default:
            /* Channel timer không hợp lệ - không làm gì. */
            break;
    }
}

/**
 * @brief   Ghi giá trị CCR mới cho channel timer tương ứng 
 *
 * @param[in] TIMx         nhận timer qua GET_TIMER_ID.
 * @param[in] TimerChannel nhận channel timer qua config (0, 1, 2, 3).
 * @param[in] CcrValue     Giá trị CCR mới. để tính ra duty mới
 */
static void Pwm_SetCompareValue(TIM_TypeDef *TIMx, uint8 TimerChannel, uint16 CcrValue)
{
    switch (TimerChannel)
    {
        case 0U:    /* Timer 1*/
            TIM_SetCompare1(TIMx, CcrValue);
            break;

        case 1U:    /* Timer 2*/
            TIM_SetCompare2(TIMx, CcrValue);
            break;

        case 2U:    /* Timer 3*/
            TIM_SetCompare3(TIMx, CcrValue);
            break;

        case 3U:    /* Timer 4*/
            TIM_SetCompare4(TIMx, CcrValue);
            break;

        default:
            /* Channel timer không hợp lệ - không làm gì. */
            break;
    }
}

/*******************************************************************************
 * Các hàm trong API
 ******************************************************************************/

/* Cấu hình PWM */
void Pwm_Init(const Pwm_ConfigType *ConfigPtr)
{
    uint8 channelIndex;

    if (ConfigPtr == NULL_PTR)
    {
        return;
    }

    /* Lưu cấu hình hiện tại. */
    Pwm_CurrentConfigPtr = ConfigPtr;

    for (channelIndex = 0U; channelIndex < ConfigPtr->numOfChannels; channelIndex++)
    {
        /* Duyệt qua từng kênh PWM. */
        const Pwm_ChannelConfigType *channelCfg = &ConfigPtr->channelConfigPtr[channelIndex];

        /* Lấy con trỏ timer tương ứng theo timerID. */
        TIM_TypeDef *timer = PWM_GET_TIMER_ID(channelCfg->timerId);
        uint16 ccrValue;
        uint32 arrValue;

        /* Kiểm tra timer. */
        if (timer == NULL_PTR)
        {
            continue;
        }

        /* Bật clock cho timer tương ứng*/
        if (channelCfg->timerId == PWM_TIMER_1)
        {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
        }
        else if (channelCfg->timerId == PWM_TIMER_2)
        {
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
        }
        else if (channelCfg->timerId == PWM_TIMER_3)
        {
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
        }
        else if (channelCfg->timerId == PWM_TIMER_4)
        {
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
        }
        else
        {
            continue; /* Timer không hợp lệ thì bỏ qua. */
        }

        /* Cài đặt time-base line phục vụ cho pwm. */
        {
            TIM_TimeBaseInitTypeDef timeBaseInitStruct;

            timeBaseInitStruct.TIM_Period            = channelCfg->defaultPeriod;
            timeBaseInitStruct.TIM_Prescaler         = 0U; /* Có thể tính lại theo f_pwm */
            timeBaseInitStruct.TIM_ClockDivision     = TIM_CKD_DIV1;
            timeBaseInitStruct.TIM_CounterMode       = TIM_CounterMode_Up;
            timeBaseInitStruct.TIM_RepetitionCounter = 0U;
            TIM_TimeBaseInit(timer, &timeBaseInitStruct);
            TIM_ARRPreloadConfig(timer, ENABLE); /* Bật preload cho ARR để tránh glitch*/
        }
        

        /* Tính CCR từ duty mặc định 0x0000 là 0%) .. 0x8000 là 100%) */
        if (channelCfg->defaultDuty > PWM_DUTY_CYCLE_100_PERCENT)
        {
            continue; /* Duty vượt quá 100% - bỏ qua */
        }

        /* Lấy giá trị chu kỳ hiện tại của Timer. */
        arrValue = (uint32)(timer->ARR);

        /* Tính CCR theo công thức: CCR = (ARR + 1) * Duty16 / 0x8000. */
        ccrValue = (uint16)(((arrValue + 1U) * (uint32)(channelCfg->defaultDuty)) >> 15U);

        /* Cấu hình PWM output thông qua hàm*/
        Pwm_ConfigOutputCompare(timer, channelCfg->timerChannel, ccrValue, channelCfg->polarity);

        /* Bật counter. Nếu dùng timer1 cần thêm MOE (BDTR) nếu dùng CHxN. */
        TIM_Cmd(timer, ENABLE);
        if (channelCfg->timerId == PWM_TIMER_1)
        {
            TIM_CtrlPWMOutputs(timer, ENABLE); /* Bật MOE cho TIM1. */
        }

        /* Lưu trạng thái nội bộ. */
        Pwm_CurrentDuty[channelIndex]    = channelCfg->defaultDuty;

        /* Lưu trạng thái kênh. */
        Pwm_ChannelIsIdle[channelIndex]  = FALSE; /* Đang chạy. */
    }

    /* Lưu trạng thái power. */
    Pwm_CurrentPowerState = PWM_RUN;
    Pwm_TargetPowerState  = PWM_RUN;

    /* Lưu trạng thái chuẩn bị power. */
    Pwm_PowerStatePrepared = FALSE;

    /* Báo hiệu khởi tạo thành công. */
    Pwm_InitStatus = TRUE;
}

/* Hàm hủy cấu hình PWM */
void Pwm_DeInit(void)
{
    uint8 channelIndex;

    /* Kiểm tra đã cấu hình và có dữ liệu cấu hình chưa.*/
    if ((Pwm_InitStatus == FALSE) || (Pwm_CurrentConfigPtr == NULL_PTR))
    {
        return;
    }

    for (channelIndex = 0U; channelIndex < Pwm_CurrentConfigPtr->numOfChannels; channelIndex++)
    {
        /* Duyệt qua từng kênh PWM */
        const Pwm_ChannelConfigType *channelCfg = &Pwm_CurrentConfigPtr->channelConfigPtr[channelIndex];
        TIM_TypeDef *timer = PWM_GET_TIMER_ID(channelCfg->timerId);

        if (timer == NULL_PTR)
        {
            continue;
        }

        /* Đưa channel về idle trước khi tắt. */
        Pwm_SetOutputToIdle(channelCfg->channelId);

        /* Tắt MOE nếu đã dùng TIM1. */
        if (channelCfg->timerId == PWM_TIMER_1)
        {
            TIM_CtrlPWMOutputs(timer, DISABLE);
        }

        /* tắt hết timer. */
        TIM_Cmd(timer, DISABLE);
        TIM_DeInit(timer);
    }

    /* Cập nhật lại con trỏ dữ liệu cấu hình khi tắt*/
    Pwm_CurrentConfigPtr   = NULL_PTR;

    /* Cập nhật lại trạng thái power */
    Pwm_CurrentPowerState  = PWM_RUN;
    Pwm_TargetPowerState   = PWM_RUN;

    /* Cập nhật lại trạng thái chuẩn bị power */
    Pwm_PowerStatePrepared = FALSE;

    /* Báo hiệu lại là chưa khởi tạo */
    Pwm_InitStatus = FALSE;
}

/* Cài đặt duty cycle của kênh PWM */
void Pwm_SetDutyCycle(Pwm_ChannelType Channel, Pwm_DutyCycleType Duty16)
{
    uint8 channelIndex = PWM_INVALID_CHANNEL_INDEX;
    uint8 validIndex;
    const Pwm_ChannelConfigType *channelCfg;
    TIM_TypeDef *timer;

    /* Kiểm tra trạng thái khởi tạo */
    if ((Pwm_InitStatus == FALSE) || (Pwm_CurrentConfigPtr == NULL_PTR))
    {
        return;
    }

    /* Tìm đúng index của kênh logic Channel trong bảng cấu hình hiện hành. */
    for (validIndex = 0U; validIndex < Pwm_CurrentConfigPtr->numOfChannels; validIndex++)
    {
        if (Pwm_CurrentConfigPtr->channelConfigPtr[validIndex].channelId == Channel)
        {
            channelIndex = validIndex;
            break;
        }
    }

    /* Không tìm thấy kênh hợp lệ - thoát. */
    if (channelIndex == PWM_INVALID_CHANNEL_INDEX)
    {
        return;
    }

    channelCfg = &Pwm_CurrentConfigPtr->channelConfigPtr[channelIndex];
    timer      = PWM_GET_TIMER_ID(channelCfg->timerId);

    if (timer == NULL_PTR)
    {
        return;
    }

    /* Kiểm tra giá trị duty cycle */
    if (Duty16 == PWM_DUTY_CYCLE_0_PERCENT)
    {
        /* Đặt duty 0 bằng cách  đặt CCR */
        Pwm_SetCompareValue(timer, channelCfg->timerChannel, 0U);

        /* Đặt trạng thái kênh là idle */
        Pwm_ChannelIsIdle[channelIndex] = TRUE;
    }
    else if (Duty16 == PWM_DUTY_CYCLE_100_PERCENT)
    {
        /* Đặt duty 100 bằng cách cho CCR bằng ARR + 1 */
        Pwm_SetCompareValue(timer, channelCfg->timerChannel, (uint16)(timer->ARR + 1U));

        /* Cập nhật trạng thái kênh */
        Pwm_ChannelIsIdle[channelIndex] = FALSE;
    }
    else
    {
        /* Tính lại CCR theo thang chuẩn 0x0000 là 0% và 0x8000 là 100% */
        uint32 arrValue = (uint32)(timer->ARR);
        uint16 ccrValue = (uint16)(((arrValue + 1U) * (uint32)Duty16) >> 15U);

        /* Cập nhật giá trị CCR mới */
        Pwm_SetCompareValue(timer, channelCfg->timerChannel, ccrValue);

        /* Cập nhật trạng thái kênh */
        Pwm_ChannelIsIdle[channelIndex] = FALSE;
    }

    /* Cập nhật duty cycle hiện tại */
    Pwm_CurrentDuty[channelIndex] = Duty16;
}

/* Cài đặt period và duty cycle */
void Pwm_SetPeriodAndDuty(Pwm_ChannelType Channel,
                          Pwm_PeriodType PeriodTicks,
                          Pwm_DutyCycleType Duty16)
{
    uint8 channelIndex = PWM_INVALID_CHANNEL_INDEX;
    uint8 validIndex;
    const Pwm_ChannelConfigType *channelCfg;
    TIM_TypeDef *timer;
    uint16 ccrValue;

    /* Kiểm tra trạng thái khởi tạo */
    if ((Pwm_InitStatus == FALSE) || (Pwm_CurrentConfigPtr == NULL_PTR))
    {
        return;
    }

    /* Tìm đúng index của kênh logic Channel trong bảng cấu hình hiện hành. */
    for (validIndex = 0U; validIndex < Pwm_CurrentConfigPtr->numOfChannels; validIndex++)
    {
        if (Pwm_CurrentConfigPtr->channelConfigPtr[validIndex].channelId == Channel)
        {
            channelIndex = validIndex;
            break;
        }
    }

    if (channelIndex == PWM_INVALID_CHANNEL_INDEX)
    {
        return;
    }

    channelCfg = &Pwm_CurrentConfigPtr->channelConfigPtr[channelIndex];
    timer      = PWM_GET_TIMER_ID(channelCfg->timerId);

    if (timer == NULL_PTR)
    {
        return;
    }

    /* Kiểm tra loại kênh cho phép đổi period */
    if (channelCfg->channelClass != PWM_VARIABLE_PERIOD)
    {
        return;
    }

    /* Chỉ áp dụng cho đúng kênh tìm được */
    if (PeriodTicks == 0U)
    {
        /* Nếu period = 0 thì đầu ra về Idle. */
        Pwm_SetOutputToIdle(Channel);
    }
    else
    {
        uint32 arrValue;

        /* Cập nhật ARR mới thông qua period đã truyền vào */
        TIM_SetAutoreload(timer, (uint16)PeriodTicks);

        /* Tính lại giá trị CCR theo ARR mới 0x0000 là 0% và 0x8000 là 100% */
        arrValue = (uint32)PeriodTicks;
        ccrValue = (uint16)(((arrValue + 1U) * (uint32)Duty16) >> 15U);

        /* Đặt lại giá trị ccr mới cho timer để có duty cycle mong muốn */
        Pwm_SetCompareValue(timer, channelCfg->timerChannel, ccrValue);

        /* Cập nhật duty cycle hiện tại */
        Pwm_CurrentDuty[channelIndex]   = Duty16;

        /* Báo hiệu kênh đang hoạt động */
        Pwm_ChannelIsIdle[channelIndex] = FALSE;
    }
}

/* Đặt trạng thái đầu ra về mức idle */
void Pwm_SetOutputToIdle(Pwm_ChannelType Channel)
{
    uint8 channelIndex = PWM_INVALID_CHANNEL_INDEX;
    uint8 validIndex;
    const Pwm_ChannelConfigType *channelCfg;
    TIM_TypeDef *timer;

    /* Kiểm tra trạng thái khởi tạo */
    if ((Pwm_InitStatus == FALSE) || (Pwm_CurrentConfigPtr == NULL_PTR))
    {
        return;
    }

    /* Tìm đúng index của kênh logic Channel. */
    for (validIndex = 0U; validIndex < Pwm_CurrentConfigPtr->numOfChannels; validIndex++)
    {
        if (Pwm_CurrentConfigPtr->channelConfigPtr[validIndex].channelId == Channel)
        {
            channelIndex = validIndex;
            break;
        }
    }

    if (channelIndex == PWM_INVALID_CHANNEL_INDEX)
    {
        return;
    }

    /* Lấy cấu hình kênh và timer tương ứng */
    channelCfg = &Pwm_CurrentConfigPtr->channelConfigPtr[channelIndex];
    timer      = PWM_GET_TIMER_ID(channelCfg->timerId);

    if (timer == NULL_PTR)
    {
        return;
    }

    /* Đặt CCR về 0 để output về mức idle */
    Pwm_SetCompareValue(timer, channelCfg->timerChannel, 0U);

    /* Báo hiệu kênh đang ở trạng thái idle */
    Pwm_ChannelIsIdle[channelIndex] = TRUE;
}

/* Lấy output của kênh PWM */
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType Channel)
{
    /* Đặt giá trị trả về mặc định */
    Pwm_OutputStateType retVal = PWM_LOW;

    uint8 channelIndex = PWM_INVALID_CHANNEL_INDEX;
    uint8 validIndex;
    const Pwm_ChannelConfigType *channelCfg;
    TIM_TypeDef *timer;

    /* Kiểm tra trạng thái khởi tạo */
    if ((Pwm_InitStatus == FALSE) || (Pwm_CurrentConfigPtr == NULL_PTR))
    {
        return PWM_LOW; /* Hàm có kiểu trả về, không được return; trống. */
    }

    /* Tìm đúng index của kênh logic Channel. */
    for (validIndex = 0U; validIndex < Pwm_CurrentConfigPtr->numOfChannels; validIndex++)
    {
        if (Pwm_CurrentConfigPtr->channelConfigPtr[validIndex].channelId == Channel)
        {
            channelIndex = validIndex;
            break;
        }
    }

    if (channelIndex == PWM_INVALID_CHANNEL_INDEX)
    {
        return PWM_LOW;
    }

    /* Lấy cấu hình kênh và timer tương ứng */
    channelCfg = &Pwm_CurrentConfigPtr->channelConfigPtr[channelIndex];
    timer      = PWM_GET_TIMER_ID(channelCfg->timerId);

    if (timer == NULL_PTR)
    {
        return PWM_LOW;
    }

    /* Xác định trạng thái output dựa trên các điều kiện */
    if (Pwm_ChannelIsIdle[channelIndex] == TRUE)
    {
        retVal = channelCfg->idleState;
    }
    else if (Pwm_CurrentDuty[channelIndex] == PWM_DUTY_CYCLE_0_PERCENT)
    {
        retVal = (channelCfg->polarity == PWM_HIGH) ? PWM_LOW : PWM_HIGH;
    }
    else if (Pwm_CurrentDuty[channelIndex] == PWM_DUTY_CYCLE_100_PERCENT)
    {
        retVal = (channelCfg->polarity == PWM_HIGH) ? PWM_HIGH : PWM_LOW;
    }
    else
    {
        retVal = PWM_LOW;
    }

    return retVal;
}

/* Bật thông báo cho PWM */
void Pwm_EnableNotification(Pwm_ChannelType Channel,
                            Pwm_EdgeNotificationType Edge)
{
    uint8 channelIndex = PWM_INVALID_CHANNEL_INDEX;
    uint8 validIndex;
    uint8 getTimerIt = 0U;
    const Pwm_ChannelConfigType *channelCfg;
    TIM_TypeDef *timer;

    (void)Edge; /* không phân biệt cạnh, edge có thể xử lý trong ISR. */

    /* Kiểm tra trạng thái khởi tạo */
    if ((Pwm_InitStatus == FALSE) || (Pwm_CurrentConfigPtr == NULL_PTR))
    {
        return;
    }

    /* Tìm đúng index của kênh logic Channel. */
    for (validIndex = 0U; validIndex < Pwm_CurrentConfigPtr->numOfChannels; validIndex++)
    {
        if (Pwm_CurrentConfigPtr->channelConfigPtr[validIndex].channelId == Channel)
        {
            channelIndex = validIndex;
            break;
        }
    }

    /* Kiểm tra xem kênh có hợp lệ không */
    if (channelIndex == PWM_INVALID_CHANNEL_INDEX)
    {
        return;
    }

    /* Lấy cấu hình kênh và timer tương ứng */
    channelCfg = &Pwm_CurrentConfigPtr->channelConfigPtr[channelIndex];
    timer      = PWM_GET_TIMER_ID(channelCfg->timerId);

    if (timer == NULL_PTR)
    {
        return;
    }

    /* Xác định cờ ngắt dựa trên kênh timer */
    switch (channelCfg->timerChannel)
    {
        case 0U:  getTimerIt = TIM_IT_CC1; break;
        case 1U:  getTimerIt = TIM_IT_CC2; break;
        case 2U:  getTimerIt = TIM_IT_CC3; break;
        case 3U:  getTimerIt = TIM_IT_CC4; break;
        default:  break;
    }
   
    /* Bật ngắt cho timer tương ứng */
    TIM_ITConfig(timer, getTimerIt, ENABLE);
}

/* Tắt thông báo cho PWM */
void Pwm_DisableNotification(Pwm_ChannelType Channel)
{
    uint8 channelIndex = PWM_INVALID_CHANNEL_INDEX;
    uint8 validIndex;
    uint8 getTimerIt = 0U;
    const Pwm_ChannelConfigType *channelCfg;
    TIM_TypeDef *timer;

    /* Kiểm tra trạng thái khởi tạo */
    if ((Pwm_InitStatus == FALSE) || (Pwm_CurrentConfigPtr == NULL_PTR))
    {
        return;
    }

    /* Tìm đúng index của kênh logic Channel. */
    for (validIndex = 0U; validIndex < Pwm_CurrentConfigPtr->numOfChannels; validIndex++)
    {
        if (Pwm_CurrentConfigPtr->channelConfigPtr[validIndex].channelId == Channel)
        {
            channelIndex = validIndex;
            break;
        }
    }

    if (channelIndex == PWM_INVALID_CHANNEL_INDEX)
    {
        return;
    }

    /* Lấy cấu hình kênh và timer tương ứng */
    channelCfg = &Pwm_CurrentConfigPtr->channelConfigPtr[channelIndex];
    timer      = PWM_GET_TIMER_ID(channelCfg->timerId);

    if (timer == NULL_PTR)
    {
        return;
    }

    /* Xác định cờ ngắt dựa trên kênh timer */
    switch (channelCfg->timerChannel)
    {
        case 0U:  getTimerIt = TIM_IT_CC1; break;
        case 1U:  getTimerIt = TIM_IT_CC2; break;
        case 2U:  getTimerIt = TIM_IT_CC3; break;
        case 3U:  getTimerIt = TIM_IT_CC4; break;
        default:  break;
    }
   
    /* tắt ngắt cho timer tương ứng */
    TIM_ITConfig(timer, getTimerIt, DISABLE);
}

/* Lấy thông tin phiên bản của module PWM */
void Pwm_GetVersionInfo(Std_VersionInfoType *VersionInfo)
{
    if (VersionInfo == NULL_PTR)
    {
        return;
    }

    VersionInfo->vendorID         = PWM_VENDOR_ID;
    VersionInfo->moduleID         = PWM_MODULE_ID;
    VersionInfo->sw_major_version = PWM_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version = PWM_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version = PWM_SW_PATCH_VERSION;
}

/* Đặt trạng thái power của PWM */
Std_ReturnType Pwm_SetPowerState(Pwm_PowerStateType PowerState,
                                 Pwm_PowerStateRequestResultType *Result)
{
    Std_ReturnType retVal = E_NOT_OK;

    if (Result == NULL_PTR)
    {
        return retVal;
    }

    /* Kiểm tra trạng thái khởi tạo */
    if (Pwm_InitStatus == FALSE)
    {
        *Result = PWM_NOT_INIT;
    }

    /* Kiểm tra trạng thái chuẩn bị chuyển trạng thái */
    else if (Pwm_PowerStatePrepared != TRUE)
    {
        *Result = PWM_SEQUENCE_ERROR;
    }
    /* Kiểm tra trạng thái power state được yêu cầu */
    else if ((PowerState != PWM_RUN) && (PowerState != PWM_SLEEP))
    {
        *Result = PWM_POWER_STATE_NOT_SUPP;
    }
    else
    {
        /* Cập nhật trạng thái power state hiện tại */
        Pwm_CurrentPowerState  = PowerState;
        Pwm_PowerStatePrepared = FALSE;

        /* Trả về kết quả */
        *Result = PWM_SERVICE_ACCEPTED;
        retVal = E_OK;
    }

    return retVal;
}

/* Lấy trạng thái power hiện tại của PWM */
Std_ReturnType Pwm_GetCurrentPowerState(Pwm_PowerStateType *PowerState)
{
    Std_ReturnType retVal = E_NOT_OK;

    /* Kiểm tra tham số đầu vào */
    if (PowerState == NULL_PTR)
    {
        return retVal;
    }

    /* Lấy trạng thái power hiện tại */
    *PowerState = Pwm_CurrentPowerState;
    retVal = E_OK;

    return retVal;
}

/* Lấy trạng thái power mong muốn của PWM */
Std_ReturnType Pwm_GetTargetPowerState(Pwm_PowerStateType *PowerState)
{
    Std_ReturnType retVal = E_NOT_OK;

    /* Kiểm tra tham số đầu vào */
    if (PowerState == NULL_PTR)
    {
        return retVal;
    }

    /* Trả về trạng thái power mong muốn */
    *PowerState = Pwm_TargetPowerState;
    retVal = E_OK;

    return retVal;
}

/* Chuẩn bị trạng thái power cho PWM */
Std_ReturnType Pwm_PreparePowerState(Pwm_PowerStateType PowerState)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint8 channelIndex;

    /* Kiểm tra các điều kiện đúng không*/
    if ((Pwm_InitStatus != TRUE) || (Pwm_CurrentConfigPtr == NULL_PTR) ||
        ((PowerState != PWM_RUN) && (PowerState != PWM_SLEEP)))
    {
        return retVal;
    }

    /* Giả định ban đầu là Idle */
    for (channelIndex = 0U; channelIndex < Pwm_CurrentConfigPtr->numOfChannels; channelIndex++)
    {
        Pwm_ChannelType channelId = Pwm_CurrentConfigPtr->channelConfigPtr[channelIndex].channelId;

        /* Đặt về Idle và tắt notification */
        Pwm_SetOutputToIdle(channelId);
        Pwm_DisableNotification(channelId);
    }

    /* Cập nhật trạng thái power state */
    Pwm_TargetPowerState   = PowerState;

    /* Báo rằng trạng thái power state đã được chuẩn bị */
    Pwm_PowerStatePrepared = TRUE;
    retVal = E_OK;

    return retVal;
}