/*******************************************************************************
 * @file    Pwm.h
 * @brief   File header khai báo API cho module PWM MCAL.
 *
 * @details Cca kiểu dũ liệu và hàm sử dụng cho PWM driver
 *
 * @author  Duy Dang
 * @date    29-06-2026
 * @version v1.0.0
 ******************************************************************************/

#ifndef PWM_H
#define PWM_H

/*******************************************************************************
 * Khai báo include
 ******************************************************************************/
#include "Std_Types.h"


/*******************************************************************************
 * Kiểu dữ liệu
 ******************************************************************************/

/**
 * @brief   Kiểu dữ liệu định danh kênh PWM logic.
 * @details Kiểu này dùng để chọn kênh PWM trong các API public.
 */
typedef uint8 Pwm_ChannelType;

/**
 * @brief   Kiểu dữ liệu duty cycle của PWM.
 * @details Giá trị duty dùng thang 0x0000 đến 0x8000, tương ứng 0% đến 100%.
 */
typedef uint16 Pwm_DutyCycleType;

/**
 * @brief   Kiểu dữ liệu chu kỳ PWM.
 * @details Giá trị period được tính theo số tick của thanh ghi ARR.
 */
typedef uint32 Pwm_PeriodType;

/**
 * @brief   Trạng thái mức logic của ngõ ra PWM.
 */
typedef enum
{
    PWM_LOW = 0U, /* Ngõ ra PWM ở mức thấp. */
    PWM_HIGH      /* Ngõ ra PWM ở mức cao. */
} Pwm_OutputStateType;

/**
 * @brief   Kiểu dữ liệu cực tính ngõ ra PWM.
 */
typedef Pwm_OutputStateType Pwm_PolarityType;

/**
 * @brief   Kiểu dữ liệu trạng thái idle của ngõ ra PWM.
 */
typedef Pwm_OutputStateType Pwm_IdleStateType;

/**
 * @brief   Kiểu dữ liệu cạnh kích hoạt notification callback.
 */
typedef enum
{
    PWM_RISING_EDGE = 0U, /* Kích hoạt callback ở cạnh lên. */
    PWM_FALLING_EDGE,    /* Kích hoạt callback ở cạnh xuống. */
    PWM_BOTH_EDGES       /* Kích hoạt callback ở cả hai cạnh. */
} Pwm_EdgeNotificationType;

/**
 * @brief   Kiểu dữ liệu phân loại kênh PWM theo khả năng đổi period.
 */
typedef enum
{
    PWM_FIXED_PERIOD = 0U,    /* Kênh có period cố định. */
    PWM_FIXED_PERIOD_SHIFTED, /* Kênh có period cố định và lệch pha. */
    PWM_VARIABLE_PERIOD       /* Kênh cho phép thay đổi period. */
} Pwm_ChannelClassType;

/**
 * @brief   Kiểu dữ liệu trạng thái năng lượng của module PWM.
 */
typedef enum
{
    PWM_RUN = 0U, /* Module PWM đang hoạt động. */
    PWM_SLEEP     /* Module PWM ở trạng thái tiết kiệm năng lượng. */
} Pwm_PowerStateType;

/**
 * @brief   Kiểu dữ liệu kết quả yêu cầu chuyển trạng thái năng lượng.
 */
typedef enum
{
    PWM_SERVICE_ACCEPTED = 0U, /* Yêu cầu được chấp nhận. */
    PWM_NOT_INIT,             /* PWM driver chưa được khởi tạo. */
    PWM_SEQUENCE_ERROR,       /* Thứ tự gọi API không hợp lệ. */
    PWM_HW_FAILURE,           /* Lỗi phần cứng khi xử lý yêu cầu. */
    PWM_POWER_STATE_NOT_SUPP  /* Trạng thái năng lượng không được hỗ trợ. */
} Pwm_PowerStateRequestResultType;

/**
 * @brief   Kiểu dữ liệu callback notification của kênh PWM.
 */
typedef void (*Pwm_NotificationCallbackType)(void);

/**
 * @brief   Cấu hình cho một kênh PWM.
 * @details Cấu trúc này ánh xạ kênh logic tới timer, channel timer, chân GPIO
 *          và các tham số PWM cơ bản.
 */
typedef struct
{
    Pwm_ChannelType channelId;                          /* ID kênh PWM. */
    uint8 timerId;                                      /* ID timer phần cứng. */
    uint8 timerChannel;                                 /* Channel của timer: 0, 1, 2, 3 */
    Pwm_PeriodType defaultPeriod;                       /* Tần số PWM mặc định. */
    Pwm_DutyCycleType defaultDuty;                      /* Duty mặc định. */
    Pwm_OutputStateType outputState;                    /* Ngõ ra ban đầu. */
    Pwm_IdleStateType idleState;                        /* Mức idle của ngõ ra. */
    Pwm_ChannelClassType channelClass;                  /* Phân loại kênh PWM. */
    Pwm_PolarityType polarity;                          /* Cực tính ngõ ra PWM. */
    Pwm_NotificationCallbackType notificationCallback;  /* Callback notification. */
} Pwm_ChannelConfigType;

/**
 * @brief   Cấu hình tổng thể cho module PWM.
 */
typedef struct
{
    const Pwm_ChannelConfigType *channelConfigPtr;  /* Con trỏ tới danh sách cấu hình kênh. */
    uint8 numOfChannels;                            /* Số kênh PWM được cấu hình. */
} Pwm_ConfigType;

/*******************************************************************************
 * Khai báo API
 ******************************************************************************/

/**
 * @brief   Khởi tạo toàn bộ PWM.
 * @details Hàm này đọc cấu hình, bật clock timer/GPIO, cấu hình chân PWM,
 *          thiết lập period, duty mặc định và bật timer để phát xung.
 *
 * @param[in] ConfigPtr Con trỏ tới cấu hình tổng thể của PWM driver.
 */
void Pwm_Init(const Pwm_ConfigType *ConfigPtr);

/**
 * @brief   Đưa PWM về trạng thái mặc định.
 * @details Hàm này tắt timer, tắt output compare và đưa các chân PWM về trạng
 *          thái idle theo cấu hình.
 */
void Pwm_DeInit(void);

/**
 * @brief   Thay đổi duty cycle của một kênh PWM đang chạy.
 * @details Giá trị duty dùng thang 0x0000 đến 0x8000. Driver tính lại CCR và
 *          cập nhật duty theo cơ chế preload nếu được cấu hình.
 *
 * @param[in] Channel Kênh PWM cần thay đổi duty.
 * @param[in] Duty16  Giá trị duty mới.
 */
void Pwm_SetDutyCycle(Pwm_ChannelType Channel, Pwm_DutyCycleType Duty16);

/**
 * @brief   Thay đổi period và duty cycle của một kênh PWM.
 * @details API này dùng cho kênh có class là PWM_VARIABLE_PERIOD. Khi
 *          PeriodTicks bằng 0, ngõ ra PWM được đưa về trạng thái idle.
 *
 * @param[in] Channel     Kênh PWM cần thay đổi.
 * @param[in] PeriodTicks Giá trị period mới theo số tick ARR.
 * @param[in] Duty16      Giá trị duty mới.
 */
void Pwm_SetPeriodAndDuty(Pwm_ChannelType Channel,
                          Pwm_PeriodType PeriodTicks,
                          Pwm_DutyCycleType Duty16);

/**
 * @brief   Đưa ngõ ra PWM của một kênh về trạng thái idle.
 *
 * @param[in] Channel Kênh PWM cần đưa về idle.
 */
void Pwm_SetOutputToIdle(Pwm_ChannelType Channel);

/**
 * @brief   Đọc trạng thái logic hiện tại của ngõ ra PWM.
 *
 * @param[in] Channel Kênh PWM cần đọc trạng thái.
 *
 * @return PWM_HIGH nếu ngõ ra đang ở mức cao, ngược lại trả về PWM_LOW.
 */
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType Channel);

/**
 * @brief   Bật notification callback theo cạnh cho một kênh PWM.
 *
 * @param[in] Channel Kênh PWM cần bật notification.
 * @param[in] Edge    Cạnh kích hoạt notification.
 */
void Pwm_EnableNotification(Pwm_ChannelType Channel,
                            Pwm_EdgeNotificationType Edge);

/**
 * @brief   Tắt notification callback của một kênh PWM.
 *
 * @param[in] Channel Kênh PWM cần tắt notification.
 */
void Pwm_DisableNotification(Pwm_ChannelType Channel);

/**
 * @brief   Lấy thông tin version của PWM driver.
 *
 * @param[out] VersionInfo Con trỏ nhận thông tin version.
 */
void Pwm_GetVersionInfo(Std_VersionInfoType *VersionInfo);

/**
 * @brief   Chuyển PWM sang trạng thái năng lượng mới.
 *
 * @param[in]  PowerState Trạng thái năng lượng cần chuyển tới.
 * @param[out] Result     Con trỏ nhận kết quả xử lý yêu cầu.
 *
 * @return E_OK nếu yêu cầu hợp lệ, ngược lại trả về E_NOT_OK.
 */
Std_ReturnType Pwm_SetPowerState(Pwm_PowerStateType PowerState,
                                 Pwm_PowerStateRequestResultType *Result);

/**
 * @brief   Lấy trạng thái năng lượng hiện tại của PWM.
 *
 * @param[out] PowerState Con trỏ nhận trạng thái năng lượng hiện tại.
 *
 * @return E_OK nếu lấy trạng thái thành công, ngược lại trả về E_NOT_OK.
 */
Std_ReturnType Pwm_GetCurrentPowerState(Pwm_PowerStateType *PowerState);

/**
 * @brief   Lấy trạng thái năng lượng mục tiêu của PWM.
 *
 * @param[out] PowerState Con trỏ nhận trạng thái năng lượng mục tiêu.
 *
 * @return E_OK nếu lấy trạng thái thành công, ngược lại trả về E_NOT_OK.
 */
Std_ReturnType Pwm_GetTargetPowerState(Pwm_PowerStateType *PowerState);

/**
 * @brief   Chuẩn bị chuyển trạng thái năng lượng cho PWM.
 * @details Hàm này đưa các kênh về idle, tắt notification và kiểm tra điều kiện
 *          an toàn trước khi gọi Pwm_SetPowerState.
 *
 * @param[in] PowerState Trạng thái năng lượng đích cần chuẩn bị.
 *
 * @return E_OK nếu chuẩn bị thành công, ngược lại trả về E_NOT_OK.
 */
Std_ReturnType Pwm_PreparePowerState(Pwm_PowerStateType PowerState);

#endif /* PWM_H */
