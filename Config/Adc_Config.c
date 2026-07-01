/*******************************************************************************
 * @file    Adc_Config.c
 * @brief   File nguồn cấu hình ADC driver.
 *
 * @details Cấu hình group ADC và các hàm
 *          xử lý ngắt
 * @author  Duy Dang
 * @date    25-06-2026
 * @version v1.0.0
 ******************************************************************************/

/*******************************************************************************
 * Khai báo
 ******************************************************************************/
#include "Adc_Config.h"

/*******************************************************************************
 * Dữ liệu cấu hình
 ******************************************************************************/
Adc_ValueGroupType dataGroup[ADC_MAX_GROUP]; /* Buffer lưu kết quả ADC. */

const Adc_GroupConfigType groupConfigPtr[ADC_NUM_GROUP_CONFIG] =
{
    /* Group 0: ADC1, trigger phần mềm, chuyển đổi liên tục, dùng ngắt kèm DMA. */
    {
        .groupID = ADC_GROUP_1,                         /* Chọn group ADC1. */
        .channelConfig =
        {
            {
                .channelID = ADC_CHANNEL_0,             /* Chọn kênh. */
                .rank = 1U,                             /* Thứ tự chuyển đổi. */
                .sampleingTime = ADC_SAMPLETIME_28CYCLES_5 /* Thời gian lấy mẫu 28.5 chu kỳ. */
            }
        },
        .numOfChannel = 1U,                             /* Số kênh trong group. */
        .groupConvMode = ADC_CONV_MODE_CONTINUOUS,         /* Chế độ chuyển đổi liên tục. */
        .triggerSource = ADC_TRIGG_SRC_SW,              /* Trigger bằng phần mềm. */
        .hwTriggerSource = 0U,                           /* Không dùng trigger phần cứng. */
        .groupAccessMode = ADC_ACCESS_MODE_SINGLE,      /* Truy cập kết quả dạng single. */
        .streamBufferMode = ADC_STREAM_BUFFER_CIRCULAR,   /* Dạng streaming là có dừng không hay đọc liên tục. */
        .readMode = ADC_READ_MODE_DMA                   /* Đọc kết quả bằng DMA. */
    },
};

const Adc_ConfigType AdcDriverConfig =
{
    .groupConfigPtr = groupConfigPtr,                   /* Trỏ tới danh sách cấu hình group. */
    .numOfGroup = ADC_NUM_GROUP_CONFIG                  /* Số group ADC được cấu hình. */
};

/*******************************************************************************
 * Hàm xử lý ngắt
 ******************************************************************************/
// void ADC1_2_IRQHandler(void)
// {
//     /* Chuyển xử lý ngắt ADC1 và ADC2 cho ADC driver. */
//     Adc_IsrHandler(ADC1);
//     Adc_IsrHandler(ADC2);
// }

void DMA1_Channel1_IRQHandler(void)
{
    /* Chuyển xử lý ngắt DMA1 channel 1 cho ADC driver. */
    Adc_DmaIsrHandler(DMA1_Channel1);
}
