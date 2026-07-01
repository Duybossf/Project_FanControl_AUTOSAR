/**********************************************************
 * @file    Adc.c
 * @brief   ADC Driver Source File
 * @details Cấu hình để mapping ADC driver 
 * @version 1.0.0
 * @date    25-06-2026
 * @author  Duy Dang
 **********************************************************/
#include <Adc.h>
#include "Adc_Config.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

/**********************************************************
 * Biến Global sử để quản lý
 **********************************************************/

/* Số phần tử runtime dùng theo cách check Group của file 1: Group > ADC_MAX_GROUP là sai */
#define ADC_GROUP_RUNTIME_SIZE   (ADC_MAX_GROUP + 1U)

/* Kiểu struct để quản lý riêng cho từng Group ADC */
typedef struct
{
    Adc_ValueGroupType *ResultBufferPtr;
    Adc_ValueGroupType LastValue[ADC_MAX_CHANNEL];
    Adc_StreamNumSampleType ValidSample;
    Adc_StatusType Status;
    uint8 SetNotification;
    uint8 SetHwTrigger;
} Adc_GroupRuntimeType;

/* Con trỏ lưu cấu hình ADC đã được truyền vào Adc_Init */
static const Adc_ConfigType *Adc_CurrentConfigPtr = NULL_PTR;

/* Biến runtime quản lý từng Group ADC */
static Adc_GroupRuntimeType Adc_RuntimeData[ADC_GROUP_RUNTIME_SIZE];

/* Flag kiểm tra trạng thái ADC đã được Init*/
static uint8 checkInit = 0U;

/* Biến kiểm tra các powerState được được set*/
static uint8 checkSetPowerState = 0U;

/* Các biến trạng thái kiểm tra phục vụ API Power State*/
static Adc_PowerStateType checkPowerCurrent = ADC_FULL_POWER_STATE;
static Adc_PowerStateType checkPowerTarget = ADC_FULL_POWER_STATE;

/* Biến lưu vị trí channel hiện tại khi đọc ADC bằng interrupt EOC */
static uint8 Adc_IsrChannelIndex[ADC_GROUP_RUNTIME_SIZE];

/**********************************************************
 * API cấu hình kênh ADC
 **********************************************************/

/**
 * @brief Initializes the ADC hardware units and driver.
 *
 * @param[in] ConfigPtr Pointer to configuration set in Variant PB.
 * 
 * @details Hàm này khởi tạo ADC để sử dụng với luồng hoạt động:
 *  - Bật Clock cấu hình ADC (bật prescaler trước. Sau đó đến RCC để khi có xung thì chạy đúng tần số từ ban đầu)
 *  - Cấu hình các thông số ADC
 *  - Hiệu chuẩn ADC
 *  - Cấu hình từng kênh và thời gian lấy mẫu
 */
void Adc_Init(const Adc_ConfigType *ConfigPtr)
{
    uint8 channelIndex;
    uint8 groupIndex;

    /* Kiểm tra không cho Init lại khi ADC đã Init */
    if (checkInit != 0U)
    {
        return;
    }

    /* Kiểm tra con trỏ config */
    if ((ConfigPtr == NULL_PTR) || (ConfigPtr->groupConfigPtr == NULL_PTR))
    {
        return;
    }

    /* Kiểm tra số Group và Channel hợp lệ*/
    if ((ConfigPtr->numOfGroup == 0U) || (ConfigPtr->numOfGroup > ADC_GROUP_RUNTIME_SIZE))
    {
        return;
    }

    /* Cấu hình Clock Adc, MCU thông dụng sử dụng register PCLK2 (systeam clock) / 6 = 12Mhz cho thời gian chuyển đổi tối ưu nhất*/
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    /* Duyệt và cấu hình từng Channel trong Group được khởi tạo*/
    for (channelIndex = 0; channelIndex < ConfigPtr->numOfGroup; channelIndex++)
    {
        /* Kiểm tra Group ADC được config để thiết lập RCC*/
        const Adc_GroupConfigType* configAdc = &ConfigPtr->groupConfigPtr[channelIndex];
        if (configAdc->groupID == ADC_GROUP_1)
        {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        }
        else if (configAdc->groupID == ADC_GROUP_2)
        {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
        }
        else
        {
            continue;
        }

        /* Cấu hình mặc định Regular - khởi tạo mỗi Channel trong Group*/
        ADC_InitTypeDef configDefault;
        configDefault.ADC_Mode = ADC_Mode_Independent;
        configDefault.ADC_ScanConvMode = DISABLE;                        /* 1 kênh */
        configDefault.ADC_ContinuousConvMode = DISABLE;                  /* 1 lần mỗi lượt gọi*/
        configDefault.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;  /* Trogger bằng phần mềm*/
        configDefault.ADC_DataAlign = ADC_DataAlign_Right;               /* Căn lề data mặc định về LSB*/
        configDefault.ADC_NbrOfChannel = 1U;

        /* Cho phép bắt đầu khởi tạo Group ADC*/
        ADC_Init(GET_GROUP_ADC(configAdc->groupID), &configDefault);

        /* Cấu hình mặc định khi Trigger bằng phần mềm và không sử dụng chế độ nâng cao*/
        ADC_ExternalTrigConvCmd(GET_GROUP_ADC(configAdc->groupID), DISABLE);    /* Tắt hw trigger*/
        ADC_DMACmd(GET_GROUP_ADC(configAdc->groupID), DISABLE);                 /* Không sử dụng DMA*/
        ADC_ITConfig(GET_GROUP_ADC(configAdc->groupID), ADC_IT_EOC, DISABLE);   /* Khong xài ngắt*/

        /* Hiệu chuẩn ADC mỗi lần đo*/
        ADC_Cmd(GET_GROUP_ADC(configAdc->groupID), ENABLE);
        ADC_ResetCalibration(GET_GROUP_ADC(configAdc->groupID));
        while(ADC_GetResetCalibrationStatus(GET_GROUP_ADC(configAdc->groupID)));
        ADC_StartCalibration(GET_GROUP_ADC(configAdc->groupID));
        while(ADC_GetCalibrationStatus(GET_GROUP_ADC(configAdc->groupID)));
    }

    /* Lưu con trỏ cấu hình hiện tại */
    Adc_CurrentConfigPtr = ConfigPtr;

    /* Bật Cờ trạng thái kiểm tra tình trạng Adc_Init*/
        checkInit = 1U;
}

/**
 * @brief Initializes result buffer pointer for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 * @param[in] DataBufferPtr Pointer to result data buffer.
 * @details Luồng hoạt động của API:
 *  - Kiểm tra các con trỏ khác NULL_PTR
 *  - Kiểm tra Adc_Init đã được config
 *  - Gán vùng nhớ sẽ chứa data chuyển đổi vào DataBufferPtr
 *
 * @return E_OK if the result buffer was initialized, otherwise E_NOT_OK.
 */
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr)
{
    /* Kiểm tra trạng thái khởi tạo Group ADC */
    if (checkInit == 0U)
    {
        return E_NOT_OK;
    }

    /* Kiểm tra cấu hình đã init và được lưu vô con trỏ đã đúng chưa */
    if ((Adc_CurrentConfigPtr == NULL_PTR) || (Adc_CurrentConfigPtr->groupConfigPtr == NULL_PTR))
    {
        return E_NOT_OK;
    }

    /* Kiểm tra số Group hợp lệ trong range hỗ trợ ADC của MCU*/
    if ((Group > ADC_MAX_GROUP) || (Group >= Adc_CurrentConfigPtr->numOfGroup))
    {
        return E_NOT_OK;
    }

    /* Kiểm tra con trỏ param in */
    if (DataBufferPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }

    /* KHởi tạo vùng nhớ chứa data, sẽ được đăng ký bởi *DataBufferPtr và trả về kq*/
    Adc_RuntimeData[Group].ResultBufferPtr = DataBufferPtr;
    Adc_RuntimeData[Group].ValidSample = 0U;

    return E_OK;
}

/**
 * @brief De-initializes the ADC driver.
 * @details Hàm xử lý tắt tất cả thông số ADC về giá trị không hoạt dộng
 * - Kiểm tra trạng thái Adc_init chưa. Nếu chưa thì không làm gì hết
 * - Duyệt qua cấu hình từng group để disable tất cả congfig
 */
void Adc_DeInit(void)
{
    uint8 channelIndex;
    const Adc_ConfigType* configAdc;
    const Adc_GroupConfigType* groupConfigPtr;

    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa*/
    if (checkInit == 0U)
    {
        return;
    }

    /* Kiểm tra cấu hình đã init và được lưu vô con trỏ đã đúng chưa */
    if ((Adc_CurrentConfigPtr == NULL_PTR) || (Adc_CurrentConfigPtr->groupConfigPtr == NULL_PTR))
    {
        checkInit = 0U;
        return;
    }

    /* Duyệt và tắt tất cả các cấu hình từng kênh của Group được khởi tạo*/
    for (channelIndex = 0U; channelIndex < configAdc->numOfGroup; channelIndex++)
    {
        const Adc_GroupConfigType *groupConfigPtr = &Adc_CurrentConfigPtr->groupConfigPtr[channelIndex];

        /* Tắt các chế độ trigger kể cả SW hay HW*/
        ADC_SoftwareStartConvCmd(GET_GROUP_ADC(groupConfigPtr->groupID), DISABLE);
        ADC_ExternalTrigConvCmd(GET_GROUP_ADC(groupConfigPtr->groupID), DISABLE);

        /* Xóa tất cả cờ interrupt*/
        ADC_ITConfig(GET_GROUP_ADC(groupConfigPtr->groupID), ADC_IT_EOC, DISABLE);
        ADC_ClearITPendingBit(GET_GROUP_ADC(groupConfigPtr->groupID), ADC_IT_EOC);

        /* Tắt chế độ DMA nếu đã cấu hình*/
        ADC_DMACmd(GET_GROUP_ADC(groupConfigPtr->groupID), DISABLE);

        /* Cập nhật status khi mà ADC bị Deinit*/
        Adc_RuntimeData[channelIndex].Status = ADC_IDLE;
    }

    /* Tắt luôn bộ ADC*/
    ADC_Cmd(GET_GROUP_ADC(groupConfigPtr->groupID), DISABLE);

    /* Reset toàn bộ dữ liệu runtime đã chạycủa ADC */
    for (channelIndex = 0U; channelIndex < ADC_GROUP_RUNTIME_SIZE; channelIndex++)
    {
        Adc_RuntimeData[channelIndex].ResultBufferPtr = NULL_PTR;
        Adc_RuntimeData[channelIndex].ValidSample = 0U;
        Adc_RuntimeData[channelIndex].Status = ADC_IDLE;
        Adc_RuntimeData[channelIndex].SetNotification = 0U;
        Adc_RuntimeData[channelIndex].SetHwTrigger = 0U;
    }

    /* Reset lại các biến kiểm tra hoặc biến trạng thái phục vụ API khi ADC_DeInit*/
    Adc_CurrentConfigPtr = NULL_PTR;
    checkPowerCurrent = ADC_FULL_POWER_STATE;
    checkPowerTarget = ADC_FULL_POWER_STATE;
    checkSetPowerState = 0U;

    /* Reset lại status khi adc được config*/
    checkInit = 0U;
}

/**
 * @brief Starts conversion for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 * @details Hàm cho phép chuyển đổi tất cả channels của một Group ADC
 *          Reset tất cả các con trỏ lưu giá trị buffer
 *          Hàm này chỉ chạy với SW trigger nếu Group config là SW trigger
 */
void Adc_StartGroupConversion(Adc_GroupType Group)
{
    uint8 enableHwTrigger;
    uint8 channelIndex;
    /* Lấy group ADC tương ứng*/
    const Adc_ConfigType configAdc;
    const Adc_GroupConfigType* startGroupPtr = &Adc_CurrentConfigPtr->groupConfigPtr[Group];

    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa*/
    if (checkInit == 0U)
    {
        return;
    }

    /* Kiểm tra con trỏ Group */
    if (startGroupPtr == NULL_PTR)
    {
        return;
    }

    /* Kiểm tra cấu hình ADC: HW trigger chỉ chạy khi đã EnableHardwareTrigger */
    if ((startGroupPtr->triggerSource == ADC_TRIGG_SRC_HW) && (Adc_RuntimeData[Group].SetHwTrigger == 0U))
    {
        return;
    }

    /* Kiểm tra cấu hình ADC chuyển đổi. Khác oneshot hoặc continuous thì không được phép*/
    if (startGroupPtr->groupConvMode != ADC_CONV_MODE_ONESHOT && 
        startGroupPtr->groupConvMode != ADC_CONV_MODE_CONTINUOUS)
    {
        return;
    }

    /* Cờ báo sử dụng HW trigger được config trong Group*/
    enableHwTrigger = ((startGroupPtr->triggerSource == ADC_TRIGG_SRC_HW) &&
                       (Adc_RuntimeData[Group].SetHwTrigger != 0U)) ? 1U : 0U;

    /* Cấu hình phần cứng ADC đúng theo Group trước khi bắt đầu chuyển đổi */
    {
        ADC_InitTypeDef configDefault;
        configDefault.ADC_Mode = ADC_Mode_Independent;
        configDefault.ADC_ScanConvMode = (startGroupPtr->numOfChannel > 1U) ? ENABLE : DISABLE;
        configDefault.ADC_ContinuousConvMode = (startGroupPtr->groupConvMode == ADC_CONV_MODE_CONTINUOUS) ? ENABLE : DISABLE;
        configDefault.ADC_ExternalTrigConv = (enableHwTrigger != 0U) ? startGroupPtr->hwTriggerSource : ADC_ExternalTrigConv_None;
        configDefault.ADC_DataAlign = ADC_DataAlign_Right;
        configDefault.ADC_NbrOfChannel = startGroupPtr->numOfChannel;
        ADC_Init(GET_GROUP_ADC(startGroupPtr->groupID), &configDefault);
    }

    /* Cấu hình từng kênh lẻ nếu dùng DMA */
    for (channelIndex = 0U; channelIndex < startGroupPtr->numOfChannel; channelIndex++)
    {
        /* Cấu hình từng kênh trong Group */
        ADC_RegularChannelConfig(GET_GROUP_ADC(startGroupPtr->groupID), startGroupPtr->channelConfig->channelID, 
        startGroupPtr->channelConfig->rank, startGroupPtr->channelConfig->sampleingTime);
    }

    /* Kiểm tra Group cho phép dùng DMA*/
    if (startGroupPtr->readMode == ADC_READ_MODE_DMA)
    {
        /* Config cho chế độ sử dụng DMA*/
        DMA_InitTypeDef dmaConfig;
        ADC_TypeDef* tempDmaConfigPtr = GET_GROUP_ADC(startGroupPtr->groupID);

        /* Bật clock cho DMA */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        /* Xóa cấu hình DMA */
        DMA_Cmd(DMA1_Channel1, DISABLE);
        DMA_DeInit(DMA1_Channel1);

        /*
         * Clear flag DMA trước khi start lần mới.
         */
        DMA_ClearFlag(DMA1_FLAG_GL1);
        DMA_ClearFlag(DMA1_FLAG_TC1);
        DMA_ClearFlag(DMA1_FLAG_HT1);
        DMA_ClearFlag(DMA1_FLAG_TE1);

        /*
         * Reset vị trí lưu result về base address đã được setup
         */
        Adc_RuntimeData[Group].ValidSample = 0U;

        dmaConfig.DMA_PeripheralBaseAddr = ((uint32)&tempDmaConfigPtr->DR);
        dmaConfig.DMA_MemoryBaseAddr = (uint32)Adc_RuntimeData[Group].ResultBufferPtr;
        dmaConfig.DMA_DIR = DMA_DIR_PeripheralSRC;

        /*
         * Single access:
         *     BufferSize = số channel trong group.
         *
         * Streaming access:
         *     BufferSize = số channel * số sample stream.
         */
        if (startGroupPtr->groupAccessMode == ADC_ACCESS_MODE_STREAMING)
        {
            dmaConfig.DMA_BufferSize = (startGroupPtr->numOfChannel) * ADC_MAX_GROUP;
        }
        else
        {
            dmaConfig.DMA_BufferSize = startGroupPtr->numOfChannel;
        }

        dmaConfig.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        dmaConfig.DMA_MemoryInc = DMA_MemoryInc_Enable;
        dmaConfig.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
        dmaConfig.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;

        /*
         * Linear buffer:
         *     DMA_Mode_Normal.
         *
         * Circular buffer:
         *     DMA_Mode_Circular.
         */
        if (startGroupPtr->streamBufferMode == ADC_STREAM_BUFFER_CIRCULAR)
        {
            dmaConfig.DMA_Mode = DMA_Mode_Circular;
        }
        else
        {
            dmaConfig.DMA_Mode = DMA_Mode_Normal;
        }

        dmaConfig.DMA_Priority = DMA_Priority_High;
        dmaConfig.DMA_M2M = DMA_M2M_Disable;

        /* Bắt đầu khởi tạo DMA*/
        DMA_Init(DMA1_Channel1, &dmaConfig);

        /* Sử dụng ngắt DMA*/
        DMA_ITConfig(DMA1_Channel1, DMA_IT_TC | DMA_IT_HT | DMA_IT_TE, ENABLE);

        /* Cấu hình NVIC để nhận ngắt từ DMA*/
        {
            NVIC_InitTypeDef NVIC_InitStruct;
            NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel1_IRQn;
            NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
            NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
            NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

            NVIC_Init(&NVIC_InitStruct);
        }
        NVIC_EnableIRQ(DMA1_Channel1_IRQn);

        /*
         * Bật DMA channel trước.
         */
        DMA_Cmd(DMA1_Channel1, ENABLE);

        /*
         * Bật ADC DMA request sau khi DMA đã sẵn sàng.
         */
        ADC_DMACmd(tempDmaConfigPtr, ENABLE);
    }
    else    /* Không sử dụng DMA */
    {
        ADC_DMACmd(GET_GROUP_ADC(startGroupPtr->groupID), DISABLE);
    }

    /* Cho phép bắt đầu chuyển đổi với trigger sw */
    if (startGroupPtr->triggerSource == ADC_TRIGG_SRC_SW)
    {
        ADC_ClearFlag(GET_GROUP_ADC(startGroupPtr->groupID), ADC_FLAG_EOC);
        /* Đặt trạng thái nhóm ADC */
        Adc_RuntimeData[Group].Status = ADC_BUSY;
        ADC_SoftwareStartConvCmd(GET_GROUP_ADC(startGroupPtr->groupID), ENABLE);
    }
}

/**
 * @brief Stops conversion for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 * @details Dừng quá trình chuyển đổi của ADC bao gồm cả cờ DMA nếu có dùng
 */
void Adc_StopGroupConversion(Adc_GroupType Group)
{
    /* Lấy group ADC tương ứng*/
    const Adc_ConfigType configAdc;
    const Adc_GroupConfigType* stopGroupPtr = &Adc_CurrentConfigPtr->groupConfigPtr[Group]; /* Lấy group ADC tương ứng*/

    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa*/
    if (checkInit == 0U)
    {
        return;
    }

    /* Kiểm tra con trỏ Group */
    if (stopGroupPtr == NULL_PTR)
    {
        return;
    }

    /* Tắt tất cả dịch vụ, xóa cờ bắt đầu quá trình chuyển đổi ADC bao gồm cả DMA*/
    ADC_SoftwareStartConvCmd(GET_GROUP_ADC(stopGroupPtr->groupID), DISABLE);
    ADC_ClearFlag(GET_GROUP_ADC(stopGroupPtr->groupID), ADC_FLAG_EOC);
    ADC_DMACmd(GET_GROUP_ADC(stopGroupPtr->groupID), DISABLE);
}

/**
 * @brief Reads the latest conversion result of an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 * @param[out] DataBufferPtr Pointer to result data buffer.
 *
 * @return E_OK if data was read, otherwise E_NOT_OK.
 */
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr)
{
    /* Lấy cấu hình group ADC tương ứng khi nhận param in*/
    const Adc_ConfigType configAdc;
    const Adc_GroupConfigType* readGroupPtr = &Adc_CurrentConfigPtr->groupConfigPtr[Group];
    uint8 readIndex = 0U;

    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa*/
    if (checkInit == 0U)
    {
        return E_NOT_OK;
    }

    /* Kiểm tra con trỏ Group */
    if (readGroupPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }

    /* Kiểm tra con trỏ Group */
    if (DataBufferPtr == NULL_PTR)
    {
        return E_NOT_OK;
    }

    /* Kiểm tra cờ báo EOC của ADC: đã chuyển đổi xong thì mới bắt đầu đọc được. Phụ thuộc vào MCU*/
    if (ADC_GetFlagStatus(GET_GROUP_ADC(readGroupPtr->groupID), ADC_FLAG_EOC) == RESET)
    {
        return E_NOT_OK;
    }

    /* Nếu không cấu hình DMA: đọc trực tiếp thanh ghi DR và lưu vào runtime buffer */
    for (readIndex = 0U; (readIndex < readGroupPtr->numOfChannel) && (readIndex < ADC_MAX_CHANNEL); readIndex++)
    {
        if (ADC_GetFlagStatus(GET_GROUP_ADC(readGroupPtr->groupID), ADC_FLAG_EOC) == RESET)
        {
            return E_NOT_OK;
        }

        /* Lưu giá trị chuyển đổi cuối cùng */
        Adc_RuntimeData[Group].LastValue[readIndex] = (Adc_ValueGroupType)ADC_GetConversionValue(GET_GROUP_ADC(readGroupPtr->groupID));

        /* Lưu giá trị vào buffer kết quả */
        Adc_RuntimeData[Group].ResultBufferPtr[readIndex] = Adc_RuntimeData[Group].LastValue[readIndex];

        /* Trả về giá trị cho buffer lấy ra */
        DataBufferPtr[readIndex] = Adc_RuntimeData[Group].LastValue[readIndex];
    }

    /* Trả về khi đọc thành công*/
    return E_OK;
}

/**
 * @brief Enables the hardware trigger for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 * @details Hàm này cho phép adc group bật hw trigger
 */
void Adc_EnableHardwareTrigger(Adc_GroupType Group)
{
    /* Lấy cấu hình group ADC tương ứng khi nhận param in*/
    const Adc_ConfigType configAdc;
    const Adc_GroupConfigType* hwTriggerGroupPtr = &Adc_CurrentConfigPtr->groupConfigPtr[Group];

    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa*/
    if (checkInit == 0U)
    {
        return;
    }

    /* Kiểm tra con trỏ Group */
    if (hwTriggerGroupPtr == NULL_PTR)
    {
        return;
    }

    /* Kiểm tra cờ báo cho phép Trigger HW */
    if (hwTriggerGroupPtr->triggerSource != ADC_TRIGG_SRC_HW)
    {
        return;
    }

    /* Config ADC khi chế chuyển đổi bằng HW được bật*/
    {
        ADC_InitTypeDef configDefaultHw;
        configDefaultHw.ADC_Mode = ADC_Mode_Independent;

        /* Bật chế độ scan nếu có nhiều hơn 1 channel*/
        configDefaultHw.ADC_ScanConvMode = (hwTriggerGroupPtr->numOfChannel > 1U) ? ENABLE : DISABLE;

        /* Kiểm tra và chọn chế độ chuyển đổi*/
        configDefaultHw.ADC_ContinuousConvMode = (hwTriggerGroupPtr->groupConvMode == ADC_CONV_MODE_CONTINUOUS) ? ENABLE : DISABLE;

        /* Cho phép bật Trigger Hw nếu cờ báo được bật*/
        configDefaultHw.ADC_ExternalTrigConv = hwTriggerGroupPtr->hwTriggerSource;

        /* Config các thông số mặc định*/
        configDefaultHw.ADC_DataAlign = ADC_DataAlign_Right;
        configDefaultHw.ADC_NbrOfChannel = hwTriggerGroupPtr->numOfChannel;

        /* Cho phép config bằng Trigger HW*/
        ADC_Init(GET_GROUP_ADC(hwTriggerGroupPtr->groupID), &configDefaultHw);
    }

    // Adc_ApplyRegularChannelConfig(groupCfg);
    ADC_ExternalTrigConvCmd(GET_GROUP_ADC(hwTriggerGroupPtr->groupID), ENABLE);
    ADC_DMACmd(GET_GROUP_ADC(hwTriggerGroupPtr->groupID), DISABLE);
}

/**
 * @brief Disables the hardware trigger for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 */
void Adc_DisableHardwareTrigger(Adc_GroupType Group)
{
    /* Lấy cấu hình group ADC tương ứng khi nhận param in*/
    const Adc_ConfigType configAdc;
    const Adc_GroupConfigType* hwTriggerGroupPtr = &Adc_CurrentConfigPtr->groupConfigPtr[Group];

    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa*/
    if (checkInit == 0U)
    {
        return;
    }

    /* Kiểm tra con trỏ Group */
    if (hwTriggerGroupPtr == NULL_PTR)
    {
        return;
    }

    /* Kiểm tra cờ báo cho phép Trigger HW */
    if (hwTriggerGroupPtr->triggerSource != ADC_TRIGG_SRC_HW)
    {
        return;
    }

    /* Bỏ cờ báo khi HW trigger đã bị disable*/
    Adc_RuntimeData[Group].SetHwTrigger = 0U;

    /* Tắt chế độ chuyển đổi Trigger HW*/
    ADC_ExternalTrigConvCmd(GET_GROUP_ADC(hwTriggerGroupPtr->groupID), DISABLE);
}

/**
 * @brief Enables the notification callback for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 */
void Adc_EnableGroupNotification(Adc_GroupType Group)
{
    /* Lấy cấu hình group ADC tương ứng khi nhận param in*/
    const Adc_ConfigType configAdc;
    const Adc_GroupConfigType* enableNotiGroupPtr = &Adc_CurrentConfigPtr->groupConfigPtr[Group];

    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa */
    if (checkInit == 0U)
    {
        return;
    }

    /* Kiểm tra con trỏ Group */
    if (enableNotiGroupPtr == NULL_PTR)
    {
        return;
    }

    /* Bật cờ kiểm tra chuyển đổi xong cho đúng Group */
    Adc_RuntimeData[Group].SetNotification = 1U;

    /* Xóa cờ ngắt và bật cờ chuyển đổi xong ADC*/
    ADC_ClearITPendingBit(GET_GROUP_ADC(enableNotiGroupPtr->groupID), ADC_IT_EOC);
    ADC_ITConfig(GET_GROUP_ADC(enableNotiGroupPtr->groupID), ADC_IT_EOC, ENABLE);
    // Adc_EnsureAdcNvicEnabled();
}

/**
 * @brief Disables the notification callback for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 */
void Adc_DisableGroupNotification(Adc_GroupType Group)
{
    /* Lấy cấu hình group ADC tương ứng khi nhận param in*/
    const Adc_ConfigType configAdc;
    const Adc_GroupConfigType* disableNotiGroupPtr = &Adc_CurrentConfigPtr->groupConfigPtr[Group];

    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa */
    if (checkInit == 0U)
    {
        return;
    }

    /* Kiểm tra con trỏ Group */
    if (disableNotiGroupPtr == NULL_PTR)
    {
        return;
    }

    /* Tắt cờ kiểm tra chuyển đổi xong cho đúng Group */
    Adc_RuntimeData[Group].SetNotification = 0U;

    /* Xóa cờ ngắt và tắt cờ chuyển đổi ADC*/
    ADC_ITConfig(GET_GROUP_ADC(disableNotiGroupPtr->groupID), ADC_IT_EOC, DISABLE);
    ADC_ClearITPendingBit(GET_GROUP_ADC(disableNotiGroupPtr->groupID), ADC_IT_EOC);
}

/**
 * @brief Gets the current status of an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 *
 * @return Current group status.
 */
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group)
{
    /* Lấy cấu hình group ADC tương ứng khi nhận param in*/
    const Adc_ConfigType configAdc;
    const Adc_GroupConfigType* getStatusGroupPtr = &Adc_CurrentConfigPtr->groupConfigPtr[Group];

    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa */
    if (checkInit == 0U)
    {
        return ADC_IDLE;
    }

    /* Kiểm tra con trỏ Group */
    if (getStatusGroupPtr == NULL_PTR)
    {
        return ADC_IDLE;
    }

    return Adc_RuntimeData[Group].Status;
}

/**
 * @brief Gets the latest stream result pointer and number of valid samples.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 * @param[out] PtrToSamplePtr Pointer receiving latest sample pointer.
 *
 * @return Number of valid samples currently available.
 */
Adc_StreamNumSampleType Adc_GetStreamLastPointer(Adc_GroupType Group, Adc_ValueGroupType** PtrToSamplePtr)
{
    /* Lấy cấu hình group ADC tương ứng khi nhận param in*/
    const Adc_ConfigType configAdc;
    const Adc_GroupConfigType* getStreamLastGroupPtr = &Adc_CurrentConfigPtr->groupConfigPtr[Group];

    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa */
    if (checkInit == 0U)
    {
        return 0U;
    }

    /* Kiểm tra con trỏ Group */
    if (getStreamLastGroupPtr == NULL_PTR)
    {
        return 0U;
    }

    /* Kiểm tra con trỏ trỏ đến giá trị group*/
    if (PtrToSamplePtr == NULL_PTR)
    {
        return 0U;
    }

    /* Kiểm tra con trỏ dữ liệu trong buffer bị null*/
    if (Adc_RuntimeData[Group].ResultBufferPtr == NULL_PTR)
    {
        return 0U;
    }

    /* Cho phép con trỏ truy cập đến giá trị data Group mới nhất*/
    *PtrToSamplePtr = Adc_RuntimeData[Group].ResultBufferPtr;
    
    /* Trả về giá trị mẫu dữ liệu hợp lệ*/
    return Adc_RuntimeData[Group].ValidSample;
}

/**
 * @brief Gets ADC driver version information.
 *
 * @param[out] VersionInfo Pointer to version information storage.
 */
void Adc_GetVersionInfo(Std_VersionInfoType* VersionInfo)
{
    /* Kiểm tra contror khởi tạo version */
    if (VersionInfo == NULL_PTR)
    {
        return;
    }

    /* Cập nhật các giá trọ phiên bản*/
    VersionInfo->vendorID = ADC_VENDOR_ID;
    VersionInfo->moduleID = ADC_MODULE_ID;
    VersionInfo->sw_major_version = ADC_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version = ADC_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version = ADC_SW_PATCH_VERSION;
}

/**
 * @brief Applies a prepared ADC power state transition.
 *
 * @param[out] Result Pointer receiving power state request result.
 *
 * @return E_OK if the transition was applied, otherwise E_NOT_OK.
 */
Std_ReturnType Adc_SetPowerState(Adc_PowerStateRequestResultType* Result)
{
    uint8 groupIndex;
    
    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa */
    if (checkInit == 0U)
    {
        *Result = ADC_NOT_INIT;
        return E_NOT_OK;
    }

    /* Kiểm tra con trỏ Result */
    if (Result == NULL_PTR)
    {
        return E_NOT_OK;
    }

    /* Kiểm tra Power state đã được set chưa*/
    if (checkSetPowerState == 0U)
    {
        *Result = ADC_SEQUENCE_ERROR;
        return E_NOT_OK;
    }

    if ((checkPowerTarget != ADC_FULL_POWER_STATE) && (checkPowerTarget != ADC_LOW_POWER_STATE))
    {
        *Result = ADC_POWER_STATE_NOT_SUPP;
        return E_NOT_OK;
    }

    
    /* Áp power state cho tất cả ADC unit đã cấu hình */
    for (groupIndex = 0U; groupIndex < Adc_CurrentConfigPtr->numOfGroup; groupIndex++)
    {
        const Adc_GroupConfigType *groupConfigPtr = &Adc_CurrentConfigPtr->groupConfigPtr[groupIndex];
        if (checkPowerTarget == ADC_FULL_POWER_STATE)
        {
            ADC_Cmd(GET_GROUP_ADC(groupConfigPtr->groupID), ENABLE);
        }
        else
        {
            ADC_SoftwareStartConvCmd(GET_GROUP_ADC(groupConfigPtr->groupID), DISABLE);
            ADC_ExternalTrigConvCmd(GET_GROUP_ADC(groupConfigPtr->groupID), DISABLE);
            ADC_Cmd(GET_GROUP_ADC(groupConfigPtr->groupID), DISABLE);
        }
    }

    /* Cập nhật trạng thái power hiện tại*/
    checkPowerCurrent = checkPowerTarget;

    /* Bật flag kiểm tra trạng thái power đã được set và trả kết quả*/
    checkSetPowerState = 1U;
    *Result = ADC_SERVICE_ACCEPTED;
    return E_OK;
}

/**
 * @brief Gets the current ADC power state.
 *
 * @param[out] CurrentPowerState Pointer receiving current power state.
 * @param[out] Result Pointer receiving power state request result.
 *
 * @return E_OK if the state was read, otherwise E_NOT_OK.
 */
Std_ReturnType Adc_GetCurrentPowerState(Adc_PowerStateType* CurrentPowerState, Adc_PowerStateRequestResultType* Result)
{
    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa */
    if (checkInit == 0U)
    {
        *Result = ADC_NOT_INIT;
        return E_NOT_OK;
    }

    /* Kiểm tra con trỏ Result và Current Power State*/
    if (Result == NULL_PTR || CurrentPowerState == NULL_PTR)
    {
        return E_NOT_OK;
    }

    /* Cập nhật lấy trạng thái power state và trả kết quả*/
    *CurrentPowerState = checkPowerCurrent;
    *Result = ADC_SERVICE_ACCEPTED;
    return E_OK;
}

/**
 * @brief Gets the prepared ADC target power state.
 *
 * @param[out] TargetPowerState Pointer receiving target power state.
 * @param[out] Result Pointer receiving power state request result.
 *
 * @return E_OK if the state was read, otherwise E_NOT_OK.
 */
Std_ReturnType Adc_GetTargetPowerState(Adc_PowerStateType* TargetPowerState, Adc_PowerStateRequestResultType* Result)
{
    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa */
    if (checkInit == 0U)
    {
        *Result = ADC_NOT_INIT;
        return E_NOT_OK;
    }

    /* Kiểm tra con trỏ Result và Current Power State*/
    if (Result == NULL_PTR || TargetPowerState == NULL_PTR)
    {
        return E_NOT_OK;
    }

    /* Cập nhật lấy trạng thái power state và trả kết quả*/
    *TargetPowerState = checkPowerTarget;
    *Result = ADC_SERVICE_ACCEPTED;
    return E_OK;
}

/**
 * @brief Prepares an ADC target power state.
 *
 * @param[in] PowerState Target power state to prepare.
 * @param[out] Result Pointer receiving power state request result.
 *
 * @return E_OK if the state was prepared, otherwise E_NOT_OK.
 */
Std_ReturnType Adc_PreparePowerState(Adc_PowerStateType PowerState, Adc_PowerStateRequestResultType* Result)
{
    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa */
    if (checkInit == 0U)
    {
        *Result = ADC_NOT_INIT;
        return E_NOT_OK;
    }

    /* Kiểm tra con trỏ Result */
    if (Result == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if ((PowerState != ADC_FULL_POWER_STATE) && (PowerState != ADC_LOW_POWER_STATE))
    {
        *Result = ADC_POWER_STATE_NOT_SUPP;
        return E_NOT_OK;
    }

    /* Cập nhật các trạng thái power state khi đã được chuẩn bị thành công và trả về kết quả */
    checkPowerTarget = PowerState;
    checkSetPowerState = 1U;
    *Result = ADC_SERVICE_ACCEPTED;
    return E_OK;
}

/**
 * @brief Handles asynchronous ADC power state transitions.
 */
void Adc_Main_PowerTransitionManager(void)
{
    if ((checkInit != 0U) && (checkSetPowerState != 0U))
    {
        Adc_PowerStateRequestResultType resultDummy;
        (void)Adc_SetPowerState(&resultDummy);
    }
}

/**********************************************************
 * @brief   Logic ISR cho ngắt DMA channel phục vụ ADC
 * @details Hàm xử lý cả hai sự kiện Half Transfer và Transfer Complete.
 *          Khi TC xảy ra, runtime sẽ cập nhật `LastValue`, `ValidSample`, trạng thái group
 *          và gọi callback DMA complete nếu có cấu hình.
 * @param[in] DmaChannel Con trỏ DMA channel phát sinh ngắt
 **********************************************************/
void Adc_DmaIsrHandler(DMA_Channel_TypeDef *DmaChannel)
{
    uint8 groupIndex;
    uint8 channelIndex;
    uint16 dmaBufferSize;
    uint16 lastRoundIndex;

    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa */
    if ((checkInit == 0U) ||
        (DmaChannel == NULL_PTR) ||
        (Adc_CurrentConfigPtr == NULL_PTR) ||
        (Adc_CurrentConfigPtr->groupConfigPtr == NULL_PTR))
    {
        return;
    }

    /* Kiểm tra DMA channel */
    if (DmaChannel != DMA1_Channel1)
    {
        return;
    }

    /* Tìm Group ADC đang sử dụng DMA */
    for (groupIndex = 0U; groupIndex < Adc_CurrentConfigPtr->numOfGroup; groupIndex++)
    {
        const Adc_GroupConfigType *groupConfigPtr = &Adc_CurrentConfigPtr->groupConfigPtr[groupIndex];
        /* Kiểm tra con trỏ Group */
        if (groupConfigPtr == NULL_PTR)
        {
            continue;
        }

        /* Kiểm tra khi mode DMA được bật chưa*/
        if (groupConfigPtr->readMode != ADC_READ_MODE_DMA)
        {
            continue;
        }

        /* Kiểm tra ADC group */
        if (GET_GROUP_ADC(groupConfigPtr->groupID) != ADC1)
        {
            continue;
        }

        /* Kiểm tra trạng thái group */
        if (Adc_RuntimeData[groupIndex].Status != ADC_BUSY)
        {
            continue;
        }

        /* Kiểm tra buffer đã được setup bởi Adc_SetupResultBuffer chưa */
        if (Adc_RuntimeData[groupIndex].ResultBufferPtr == NULL_PTR)
        {
            return;
        }

        /* Tính kích thước buffer DMA */
        if (groupConfigPtr->groupAccessMode == ADC_ACCESS_MODE_STREAMING)
        {
            dmaBufferSize = (uint16)((uint16)groupConfigPtr->numOfChannel * (uint16)ADC_MAX_GROUP);
        }
        else
        {
            dmaBufferSize = (uint16)groupConfigPtr->numOfChannel;
        }

        /*
         * Xử lý Half Transfer.
         */
        if (DMA_GetITStatus(DMA1_IT_HT1) != RESET)
        {
            /* Xóa cờ Half Transfer */
            DMA_ClearITPendingBit(DMA1_IT_HT1);

            /* Cập nhật số lượng mẫu hợp lệ ở nửa đầu buffer */
            Adc_RuntimeData[groupIndex].ValidSample = (Adc_StreamNumSampleType)(dmaBufferSize / 2U);
        }

        /*
         * Xử lý Transfer Complete.
         */
        if (DMA_GetITStatus(DMA1_IT_TC1) != RESET)
        {
            /* Xóa cờ Transfer Complete */
            DMA_ClearITPendingBit(DMA1_IT_TC1);

            /* Tìm vị trí lần chuyển đổi cuối cùng trong buffer */
            if ((groupConfigPtr->groupAccessMode == ADC_ACCESS_MODE_STREAMING) && (dmaBufferSize >= groupConfigPtr->numOfChannel))
            {
                lastRoundIndex = (uint16)(dmaBufferSize - groupConfigPtr->numOfChannel);
            }
            else
            {
                lastRoundIndex = 0U;
            }

            /* Truyền giá trị mới nhất từ DMA buffer sang LastValue */
            for (channelIndex = 0U; ((channelIndex < groupConfigPtr->numOfChannel) && (channelIndex < ADC_MAX_CHANNEL)); channelIndex++)
            {
                Adc_RuntimeData[groupIndex].LastValue[channelIndex] = Adc_RuntimeData[groupIndex].ResultBufferPtr[lastRoundIndex + channelIndex];
            }

            /* Cập nhật số lượng mẫu hợp lệ và trạng thái group */
            if (groupConfigPtr->groupAccessMode == ADC_ACCESS_MODE_STREAMING)
            {
                Adc_RuntimeData[groupIndex].ValidSample = (Adc_StreamNumSampleType)dmaBufferSize;
                /* Cập nhật trạng thái group */
                Adc_RuntimeData[groupIndex].Status = ADC_STREAM_COMPLETED;
            }
            else
            {
                Adc_RuntimeData[groupIndex].ValidSample = (Adc_StreamNumSampleType)groupConfigPtr->numOfChannel;
                /* Cập nhật trạng thái group */
                Adc_RuntimeData[groupIndex].Status = ADC_COMPLETED;
            }

            /* Tắt ADC DMA và DMA channel nếu là One-shot */
            if ((groupConfigPtr->groupConvMode) == ADC_CONV_MODE_ONESHOT)
            {
                ADC_DMACmd(GET_GROUP_ADC(groupConfigPtr->groupID), DISABLE);
                DMA_Cmd(DmaChannel, DISABLE);
            }
        }
    }
}

/**********************************************************
 * @brief   Logic ISR cho ngắt EOC của ADCx
 * @details Hàm được gọi bởi vector adapter trong `Adc_Cfg.c`.
 *          ISR sẽ lọc các group thuộc đúng ADC instance, cập nhật giá trị mẫu cuối,
 *          cập nhật trạng thái runtime và gọi notification callback nếu được bật.
 * @param[in] AdcInstance Con trỏ ADC instance phát sinh ngắt
 **********************************************************/
void Adc_IsrHandler(ADC_TypeDef *AdcInstance)
{
    uint8 groupIndex;
    uint8 currentChannelIndex;
    Adc_ValueGroupType adcValue;

    /* Kiểm tra trạng thái Adc_Init đã được khởi tạo chưa */
    if ((checkInit == 0U) ||
        (AdcInstance == NULL_PTR) ||
        (Adc_CurrentConfigPtr == NULL_PTR) ||
        (Adc_CurrentConfigPtr->groupConfigPtr == NULL_PTR))
    {
        return;
    }

    /* Kiểm tra có đúng interrupt EOC hay không */
    if (ADC_GetITStatus(AdcInstance, ADC_IT_EOC) == RESET)
    {
        return;
    }

    /*
     * Xóa cờ EOC trước để tránh bị vào lại interrupt liên tục.
     */
    ADC_ClearITPendingBit(AdcInstance, ADC_IT_EOC);

    /* Tìm Group ADC thuộc đúng ADC instance phát sinh interrupt */
    for (groupIndex = 0U; groupIndex < Adc_CurrentConfigPtr->numOfGroup; groupIndex++)
    {
        const Adc_GroupConfigType *groupConfigPtr = &Adc_CurrentConfigPtr->groupConfigPtr[groupIndex];
        /* Kiểm tra Group có thuộc ADC instance đang interrupt không */
        if (GET_GROUP_ADC(groupConfigPtr->groupID) != AdcInstance)
        {
            continue;
        }

        /*
         * Nếu Group dùng DMA thì không xử lý ở ADC EOC ISR.
         */
        if (groupConfigPtr->readMode == ADC_READ_MODE_DMA)
        {
            continue;
        }

        /*
         * Chỉ xử lý Group đang hoạt động.
         */
        if (Adc_RuntimeData[groupIndex].Status == ADC_IDLE)
        {
            continue;
        }

        /*
         * Kiểm tra buffer đã được setup bởi Adc_SetupResultBuffer() chưa.
         */
        if (Adc_RuntimeData[groupIndex].ResultBufferPtr == NULL_PTR)
        {
            return;
        }

        /* Vị trí channel đang xử lý */
        currentChannelIndex = Adc_IsrChannelIndex[groupIndex];

        /* Đọc giá trị ADC khi không DMA và lưu vào buffer runtime.*/
        adcValue = (Adc_ValueGroupType)ADC_GetConversionValue(AdcInstance);
        Adc_RuntimeData[groupIndex].LastValue[currentChannelIndex] = adcValue;

        /* Lưu giá trị vào ResultBufferPtr đã được tạo*/
        Adc_RuntimeData[groupIndex].ResultBufferPtr[currentChannelIndex] = adcValue;

        /* Tăng vị trí channel đang xử lý */
        currentChannelIndex++;
        Adc_IsrChannelIndex[groupIndex] = currentChannelIndex;

        /* Cập nhật số lượng mẫu hợp lệ */
        Adc_RuntimeData[groupIndex].ValidSample = (Adc_StreamNumSampleType)currentChannelIndex;

        /* Reset index để chuẩn bị cho conversion round tiếp theo */
        Adc_IsrChannelIndex[groupIndex] = 0U;

        /* Cập nhật trạng thái group. */
        Adc_RuntimeData[groupIndex].Status = ADC_COMPLETED;
    }
}