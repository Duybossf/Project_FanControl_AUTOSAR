/**
 * @file    Port.c
 * @brief   Config các API của Port Driver để mapping với MCU
 *
 * @author  Duy Dang
 * @date    23-06-2026
 * @version v1.0.0
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <Port.h>
#include <Port_Config.h>
#include <Std_Types.h>
#include "stm32f10x_rcc.h"

/* Cờ trạng thái báo tình trạng Port_Init được khởi tạo hay chưa*/
static uint8 initStatus = 0U;

/*******************************************************************************
 * Static Function Declarations
 ******************************************************************************/
/* Hàm cấu hình 1 pin để phục vụ cho nhiều hàm khác*/
static void Port_ConfigSinglePin(const Port_PinConfigType* config);

/**
 * @brief Cấu hình phần cứng (RCC clock + GPIO) cho một pin.
 * @details Cấu hình cho 1 Pin để dùng chung cho các hàm khác. Hàm này sẽ được gọi trong Port_Init() để cấu hình tất cả các pin.
 *          Logic:
 *              - Bật RCC clock cho Port tương ứng.
 *              - Cấu hình GPIO_InitTypeDef cho Pin tương ứng.
 *              - Cấu hình mode, speed, direction, level cho Pin.
 *              - Nếu pin mode là ADC hoặc PWM thì bỏ qua cấu hình direction không hợp lệ
 *              - Nếu pin mode là DIO thì cấu hình direction và level ban đầu.
 * @param[in] config Con trỏ tới cấu hình của một pin cụ thể.
 */
static void Port_ConfigSinglePin(const Port_PinConfigType* config)
{
    if (config == NULL_PTR)
    {
        return;
    }

    /* Nhận diện Port và bật Clock cấu hình để map với MCU*/
    switch (config->portID)
    {
        case PORT_A:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            break;
    
        case PORT_B:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
            break;
    
        case PORT_C:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
            break;
    
        case PORT_D:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
            break;
    
        default:
            /* Port không hợp lệ, không làm gif hết -> thoát ra */
            break;
    }

    /* Nhận diện Pin của MCU để bắt đầu Config*/
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = PORT_PIN_ID(config->pinID);

    /* Đặt tốc độ xung Clock cho cấu hình*/
    GPIO_InitStruct.GPIO_Speed = config->pinSpeed;  /* Speed phụ thuộc vào MCU*/

    /* Cấu hình mode cho Pin, Các mode sẽ có như DIO, ADC, PWM,...*/
    switch (config->pinMode)
    {
        /* Cấu hình pin là một Digital input/output*/
        case PORT_PIN_MODE_DIO:
            if ((config->pinDirection == PORT_PIN_OUT))
            {
                /* Cấu hình Pin dạng Output*/
                if ((config->profileMode) == PORT_PIN_OUT_PP)
                {
                    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
                }
                else if ((config->profileMode) == PORT_PIN_OUT_OD)
                {
                    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
                }
                else
                {
                    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
                }
            }
            else
            {
                /* Cấu hình Pin dạng Input*/
                if ((config->profileMode) == PORT_PIN_IN_IPD)
                {
                    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
                }
                else if ((config->profileMode) == PORT_PIN_IN_IPU)
                {
                    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
                }
                else
                {
                    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
                }
            }
            break;
            
        case PORT_PIN_MODE_ADC:
            /* Bỏ qua cấu hình Output nếu sử dụng mode ADC*/
            if ((config->pinDirection == PORT_PIN_OUT))
            {
                return;
            }
            else
            {
                if (config->profileMode == PORT_PIN_IN_AIN)
                {
                    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
                }
                else
                {
                    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
                }
            }
            break;

        case PORT_PIN_MODE_PWM:
            /* Bỏ qua cấu hình Input nếu sử dụng mode PWM*/
            if ((config->pinDirection == PORT_PIN_IN))
            {
                return;
            }
            else
            {
                if (config->profileMode == PORT_PIN_OUT_AFPP)
                {
                    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
                }
                else
                {
                    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
                }
            }
            break;

        default:
            /* pin mode không hợp lệ, bỏ qua pin này. */
            return;
    }

    /* Bắt đầu khởi tạo Pin*/
    GPIO_Init(PORT_GET_GPIO(config->portID), &GPIO_InitStruct);

    /* Khởi tạo giá trị pin ban đầu để tránh glitch nếu chạy mode DIO: Output*/
    if (((config->pinMode) == PORT_PIN_MODE_DIO) && ((config->pinDirection) == PORT_PIN_OUT))
    {
        if (config->levelPin == PORT_PIN_LEVEL_HIGH)
        {
            GPIO_SetBits(PORT_GET_GPIO(config->portID), PORT_PIN_ID(config->pinID));
        }
        else
        {
            GPIO_ResetBits(PORT_GET_GPIO(config->portID), PORT_PIN_ID(config->pinID));
        }
    }
}

/*******************************************************************************
 * Global Function Definitions
 ******************************************************************************/
void Port_Init (const Port_ConfigType* ConfigPtr)
{
    uint16 indexPin;
    /* Kiểm tra Ptr cấu hình. Nếu null value thì thoát API*/
    if (ConfigPtr == NULL_PTR)
    {
        return;
    }

    /* Duyệt qua và cấu hình cho từng Pin được Config*/
    for (indexPin = 0; indexPin < ConfigPtr->numOfPin; indexPin++)
    {
        Port_ConfigSinglePin(&ConfigPtr->pinConfig[indexPin]);
    }

    /* Bật Cờ trạng thái kiểm tra tình trạng Port_Init*/
    initStatus = 1U;
}



/**
 * @brief Sets the port pin direction.
 * @details Hàm này cho phép set chiều của một Pin trong Port.
 *          Logic: 
 *              - Kiểm tra port đã initial chưa. Nếu chưa thì không được phép
 *              - Kiểm tra cờ trạng thái cho phép đổi chiều hay không. Nếu chwua thì không được phép
 *              - Set Direction cho Pin và Initial lại Port.
 * @param[in] Pin Port pin ID number.
 * @param[in] Direction Port pin direction.
 */
void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction)
{
    /* Kiểm tra cờ Port_Init đã chạy chưa*/
    if (initStatus == 0U)
    {
        return;
    }

    /* Kiểm tra Param[in] Pin hợp lệ với số Pin đã config*/
    if (Pin >= (Port_Config.numOfPin))
    {
        return;
    }

    /* Kiểm tra cờ trạng thái được phép đổi chiều Pin không*/
    if (Port_ConfigPins[Pin].changeDirection == 0U)
    {
        return;
    }

    /* Cập nhật Diẻction tại Pin được request và cấu hình lại pin với giá trị mới */
    Port_PinConfigType* Port_ConfigPinUpdate = (Port_PinConfigType*)&Port_ConfigPins[Pin];
    Port_ConfigPinUpdate->pinDirection = Direction;

    /* Cấu hình lại pin với giá trị mới */
    Port_ConfigSinglePin(Port_ConfigPinUpdate);
}

/**
 * @brief Refreshes the direction of all configured port pins.
 * @details Logic:
 *              - Duyệt qua tất cả các Pin đã được config, bỏ qua các Pin được phép đổi chiều runtime
 *              - Refresh lại tất cả Pin không đổi chiều runtime về lại baseline
 */
void Port_RefreshPortDirection(void)
{
    uint16 pinIndex;

    /* Kiểm tra cờ Port_Init đã chạy chưa*/
    if (initStatus == 0U)
    {
        return;
    }

    /* Duyệt qua tất cả các Pin đã được Config*/
    for (pinIndex = 0U; pinIndex < PORT_NUM_PIN_CONFIG; pinIndex++)
    {
        /* Kiểm tra các Pin đã cấu hình nào không được phép đổi chiều runtime*/
        if (Port_ConfigPins[pinIndex].changeDirection == 0U)
        {
            /* Cấu hình lại pin với giá trị mới */
            Port_ConfigSinglePin(&Port_ConfigPins[pinIndex]);
        }
    }
}

/**
 * @brief Returns the version information of this module.
 * @details Logic:
 *              - Trả về phiên bản của module sau mỗi lần update module
 *
 * @param[in] versioninfo Pointer to where to store the version information.
 */
void Port_GetVersionInfo(Std_VersionInfoType* versioninfo)
{
    if (versioninfo == NULL_PTR)
    {
        return;
    }

    /* Nhận thông tin module*/
    versioninfo->vendorID = PORT_VENDOR_ID;
    versioninfo->moduleID = PORT_MODULE_ID;
    versioninfo->sw_major_version = PORT_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = PORT_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = PORT_SW_PATCH_VERSION;
}

/**
 * @brief Sets the port pin direction.
 * @details Hàm này cho phép đổi mode của một Pin trong Port.
 *          Logic: 
 *              - Kiểm tra port đã initial chưa. Nếu chưa thì không được phép
 *              - Kiểm tra cờ trạng thái cho phép đổi mode hay không. Nếu chwua thì không được phép
 *              - Set Mode cho Pin và Initial lại Port.
 * @param[in] Pin Port pin ID number.
 * @param[in] Direction Port pin direction.
 */
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode)
{
    /* Kiểm tra cờ Port_Init đã chạy chưa*/
    if (initStatus == 0U)
    {
        return;
    }

    /* Kiểm tra Param[in] Pin hợp lệ với số Pin đã config*/
    if (Pin >= (Port_Config.numOfPin))
    {
        return;
    }

    /* Kiểm tra cờ trạng thái được phép đổi mode Pin không*/
    if (Port_ConfigPins[Pin].changeMode == 0U)
    {
        return;
    }

    /* Cập nhật Mode tại Pin được request và cấu hình lại pin với giá trị mới */
    Port_PinConfigType* Port_ConfigPinUpdate = (Port_PinConfigType*)&Port_ConfigPins[Pin];
    Port_ConfigPinUpdate->pinMode = Mode;

    /* Cấu hình lại pin với giá trị mới */
    Port_ConfigSinglePin(Port_ConfigPinUpdate);
}
