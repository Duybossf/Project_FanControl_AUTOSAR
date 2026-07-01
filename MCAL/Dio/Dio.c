/**
 * @copydoc Dio_ReadChannel
 */
#include <Dio.h>
#include <Dio_Config.h>
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>

/**
 * @copydoc Dio_ReadChannel
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId)
{
    GPIO_TypeDef* Port;
    uint16 Pin;

    /* Chọn Port từ param in ChannelID. */
    if (ChannelId < 16U)
    {
        Port = GPIOA;
    }
    else if ((ChannelId < 32U) && (ChannelId > 15U))
    {
        Port = GPIOB;
    }
    else if ((ChannelId > 31U) && (ChannelId < 48U))
    {
        Port = GPIOC;
    }
    else
    {
        Port = GPIOD;
    }

    /* Chọn Pin từ param in ChannelID. */
    if ((ChannelId & 0x0FU) != 0U)
    {
        Pin = (uint16)(1U << (ChannelId & 0x0FU));
    }
    else
    {
        Pin = (uint16)0x0001U;
    }

    /* Xử lý logic API ReadChannel(). */
    if (GPIO_ReadInputDataBit(Port, Pin) == Bit_SET)
    {
        return STD_HIGH;
    }
    else if (GPIO_ReadInputDataBit(Port, Pin) == Bit_RESET)
    {
        return STD_LOW;
    }

    return STD_LOW;
}

void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level)
{
    GPIO_TypeDef* Port;
    uint16 Pin;

    /* Chọn Port từ param in ChannelID. */
    if (ChannelId < 16U)
    {
        Port = GPIOA;
    }
    else if ((ChannelId < 32U) && (ChannelId > 15U))
    {
        Port = GPIOB;
    }
    else if ((ChannelId > 31U) && (ChannelId < 48U))
    {
        Port = GPIOC;
    }
    else
    {
        Port = GPIOD;
    }

    /* Chọn Pin từ param in ChannelID. */
    if ((ChannelId & 0x0FU) != 0U)
    {
        Pin = (uint16)(1U << (ChannelId & 0x0FU));
    }
    else
    {
        Pin = (uint16)0x0001U;
    }

    /* Xử lý logic cho API WriteChannel()*/

    if (Level == STD_HIGH)
    {
        GPIO_SetBits(Port, Pin);
    }
    else
    {
        GPIO_ResetBits(Port, Pin);
    }
}

Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId)
{
    GPIO_TypeDef* Port;

    /* Chọn Port từ param in PortId. */
    if (PortId < 16U)
    {
        Port = GPIOA;
    }
    else if ((PortId < 32U) && (PortId > 15U))
    {
        Port = GPIOB;
    }
    else if ((PortId > 31U) && (PortId < 48U))
    {
        Port = GPIOC;
    }
    else
    {
        Port = GPIOD;
    }

    /* Xử lý đọc toàn bộ Port cho API ReadPort*/
    return (Dio_PortLevelType)(GPIO_ReadInputData(Port));
}

void Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level)
{
    GPIO_TypeDef* Port;

    /* Chọn Port từ param in ChannelID. */
    if (PortId < 16U)
    {
        Port = GPIOA;
    }
    else if ((PortId < 32U) && (PortId > 15U))
    {
        Port = GPIOB;
    }
    else if ((PortId > 31U) && (PortId < 48U))
    {
        Port = GPIOC;
    }
    else
    {
        Port = GPIOD;
    }

    /* Xử lý logic ghi Port cho API Dio_WritePort()*/
    GPIO_Write(Port, Level);
}

Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr)
{
    GPIO_TypeDef* Port;
    uint16 portValue;

    /* Chọn Port từ param in ChannelGroupIdPtr. */
    if (ChannelGroupIdPtr->port < 16U)
    {
        Port = GPIOA;
    }
    else if ((ChannelGroupIdPtr->port < 32U) && (ChannelGroupIdPtr->port > 15U))
    {
        Port = GPIOB;
    }
    else if ((ChannelGroupIdPtr->port > 31U) && (ChannelGroupIdPtr->port < 48U))
    {
        Port = GPIOC;
    }
    else
    {
        Port = GPIOD;
    }

    /** Xử lý logic đọc trạng thái 1 số pin trong Port thông qua mask và offset
     * 1. Đọc giá trị của toàn bộ Port
     * 2. Lấy giá trị vừa đọc & với mask để lấy ra giá trị các Pin cần
     * 3. Dịch offset ngược lại ban đầu để lấy giá trị các Pin cần đọc. Giá trị này là con số
     * tính cho các Pin
     * */ 
    portValue = GPIO_ReadInputData(Port);
    return (Dio_PortLevelType)((GPIO_ReadInputData(Port)) & ChannelGroupIdPtr->mask) >> ChannelGroupIdPtr->offset;
}

void Dio_WriteChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr, Dio_PortLevelType Level)
{
    GPIO_TypeDef* Port;
    uint16 portValue;

    /* Chọn Port từ param in ChannelGroupIdPtr. */
    if (ChannelGroupIdPtr->port < 16U)
    {
        Port = GPIOA;
    }
    else if ((ChannelGroupIdPtr->port < 32U) && (ChannelGroupIdPtr->port > 15U))
    {
        Port = GPIOB;
    }
    else if ((ChannelGroupIdPtr->port > 31U) && (ChannelGroupIdPtr->port < 48U))
    {
        Port = GPIOC;
    }
    else
    {
        Port = GPIOD;
    }

    /** Xử lý logic đọc trạng thái 1 số pin trong Port thông qua mask và offset
     * 1. Đọc giá trị của toàn bộ Port
     * 2. Reset giá trị vừa đọc bằng mask thông qua Bit manupilation.
     * 3. Ghi giá trị Level vào thông qua offset để định vị trí các Bit cần ghi. AND với mask để ghi giá trị vào
     * cho các Pin
     * */
    portValue = GPIO_ReadInputData(Port);

    /*Reset giá trị group trong Port thông qua Bit manupilation*/
    portValue &= ~(ChannelGroupIdPtr->mask);

    /*Ghi giá trị Level vào group trong port*/
    portValue |= (Level << ChannelGroupIdPtr->offset) & (ChannelGroupIdPtr->mask);
    GPIO_Write(Port, portValue);
}

void Dio_GetVersionInfo(Std_VersionInfoType* VersionInfo)
{
    VersionInfo->moduleID = 0U;
    VersionInfo->vendorID = 0U;
    VersionInfo->sw_major_version = 0U;
    VersionInfo->sw_minor_version = 0U;
    VersionInfo->sw_patch_version = 0U;
}

Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId)
{
    GPIO_TypeDef* Port;
    uint16 Pin;

    /* Chọn Port từ param in ChannelID. */
    if (ChannelId < 16U)
    {
        Port = GPIOA;
    }
    else if ((ChannelId < 32U) && (ChannelId > 15U))
    {
        Port = GPIOB;
    }
    else if ((ChannelId > 31U) && (ChannelId < 48U))
    {
        Port = GPIOC;
    }
    else
    {
        Port = GPIOD;
    }

    /* Chọn Pin từ param in ChannelID. */
    if ((ChannelId & 0x0FU) != 0U)
    {
        Pin = (uint16)(1U << (ChannelId & 0x0FU));
    }
    else
    {
        Pin = (uint16)0x0001U;
    }

    /* Xử lý đảo trạng thái Pin khi API được gọi*/
    if ((GPIO_ReadInputDataBit(Port, Pin)) == Bit_SET)
    {
        GPIO_ResetBits(Port, Pin);  /*Ghi trạng thái đảo và trả về API*/
        return STD_LOW;
    }
    else
    {
        GPIO_SetBits(Port, Pin);    /*Ghi trạng thái đảo và trả về API*/
        return STD_HIGH;
    }
}
