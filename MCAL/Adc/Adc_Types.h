/**
 * @file    Adc_Types.h
 * @brief   ADC driver public interface for AUTOSAR Classic Platform.
 * @author  Duy Dang
 * @date    15-06-2026
 * @version v1.0.0
 */

#ifndef ADC_TYPES_H
#define ADC_TYPES_H

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <Std_Types.h>


/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * Numeric ID of an ADC channel group.
 */
typedef uint8 Adc_GroupType;

/**
 * Numeric ID of an ADC channel.
 */
typedef uint8 Adc_ChannelType;

/**
 * Type for reading converted values of a channel group.
 *
 * The value is raw ADC data without further scaling. Alignment depends on the
 * ADC_RESULT_ALIGNMENT precompile switch.
 */
typedef uint16 Adc_ValueGroupType;

/**
 * Current conversion status of the requested ADC channel group.
 */
typedef enum
{
    ADC_IDLE = 0x00U,       /* Group conversion has not started. */
    ADC_BUSY,               /* Group conversion is running. */
    ADC_COMPLETED,          /* One conversion round has completed. */
    ADC_STREAM_COMPLETED    /* Streaming result buffer is full. */
} Adc_StatusType;

/**
 * Trigger source for an ADC channel group.
 */
typedef enum
{
    ADC_TRIGG_SRC_SW = 0x00U,   /* Group is triggered by software. */
    ADC_TRIGG_SRC_HW           /* Group is triggered by hardware. */
} Adc_TriggerSourceType;

/**
 * Conversion mode for an ADC channel group.
 */
typedef enum
{
    ADC_CONV_MODE_ONESHOT = 0x00U,
    ADC_CONV_MODE_CONTINUOUS
} Adc_GroupConvModeType;

/**
 * Priority level of the channel group. Lowest priority is 0.
 */
typedef uint8 Adc_GroupPriorityType;

/**
 * Number of group conversions in streaming access mode.
 *
 * In single access mode, this value is 1.
 */
typedef uint16 Adc_StreamNumSampleType;

/**
 * Streaming access mode buffer type.
 */
typedef enum
{
    ADC_STREAM_BUFFER_LINEAR = 0x00U,   /* Stop when the stream buffer is full. */
    ADC_STREAM_BUFFER_CIRCULAR          /* Continue by wrapping around the buffer. */
} Adc_StreamBufferModeType;

/**
 * Access mode to group conversion results.
 */
typedef enum
{
    ADC_ACCESS_MODE_SINGLE = 0x00U,     /* Single value access mode. */
    ADC_ACCESS_MODE_STREAMING           /* Streaming access mode. */
} Adc_GroupAccessModeType;

/**
 * Hardware trigger edge used to start a conversion.
 */
typedef enum
{
    ADC_HW_TRIG_RISING_EDGE = 0x00U,
    ADC_HW_TRIG_FALLING_EDGE,
    ADC_HW_TRIG_BOTH_EDGES
} Adc_HwTriggerSignalType;

/**
 * Reload value of the ADC module embedded timer, if supported by hardware.
 */
typedef uint16 Adc_HwTriggerTimerType;

/**
 * ADC prioritization mechanism.
 */
typedef enum
{
    ADC_PRIORITY_NONE = 0x00U,  /* Priority mechanism is not available. */
    ADC_PRIORITY_HW,            /* Hardware priority mechanism is available. */
    ADC_PRIORITY_HW_SW          /* Hardware and software priority are available. */
} Adc_PriorityImplementationType;

/**
 * Replacement mechanism used when a group conversion is interrupted by a higher
 * priority group.
 */
typedef enum
{
    ADC_GROUP_REPL_ABORT_RESTART = 0x00U,
    ADC_GROUP_REPL_SUSPEND_RESUME
} Adc_GroupReplacementType;

/**
 * Active limit checking range selection.
 */
typedef enum
{
    ADC_RANGE_UNDER_LOW = 0x00U,    /* Below low limit, including low limit. */
    ADC_RANGE_BETWEEN,              /* Between low and high limits, including high limit. */
    ADC_RANGE_OVER_HIGH,            /* Above high limit. */
    ADC_RANGE_ALWAYS,               /* Complete range. */
    ADC_RANGE_NOT_UNDER_LOW,        /* Above low limit. */
    ADC_RANGE_NOT_BETWEEN,          /* Above high limit or below low limit. */
    ADC_RANGE_NOT_OVER_HIGH         /* Below high limit. */
} Adc_ChannelRangeSelectType;

/**
 * Alignment of ADC raw results in the result buffer.
 */
typedef enum
{
    ADC_ALIGN_LEFT = 0x00U,
    ADC_ALIGN_RIGHT
} Adc_ResultAlignmentType;

/**
 * ADC power state currently active or selected as target state.
 */
typedef uint8 Adc_PowerStateType;

/**
 * Result of requests related to power state transitions.
 */
typedef enum
{
    ADC_SERVICE_ACCEPTED = 0x00U,   /* Power state change executed. */
    ADC_NOT_INIT,                   /* ADC module is not initialized. */
    ADC_SEQUENCE_ERROR,             /* Wrong API call sequence. */
    ADC_HW_FAILURE,                 /* Hardware failure prevents the state change. */
    ADC_POWER_STATE_NOT_SUPP,       /* Requested power state is not supported. */
    ADC_TRANS_NOT_POSSIBLE          /* Requested transition is not possible now. */
} Adc_PowerStateRequestResultType;

/**
 * Định nghĩa kiểu dữ liệu để chọn chế độ đọc
 */
typedef enum
{
    ADC_READ_MODE_CPU = 0x00U,  /* Không sử dụng ngắt và DMA */
    ADC_READ_MODE_DMA = 0x01U   /* Sử dụng chế độ DMA và ngắt */
}Adc_ReadModeType;

typedef void (*Adc_NotificationCbType)(void);

typedef enum
{
    ADC_SAMPLETIME_1CYCLE_5 = 0x00U,   /* Thời gian lấy mẫu 1.5 chu kỳ. */
    ADC_SAMPLETIME_7CYCLES_5 = 0x01U,  /* Thời gian lấy mẫu 7.5 chu kỳ. */
    ADC_SAMPLETIME_13CYCLES_5 = 0x02U, /* Thời gian lấy mẫu 13.5 chu kỳ. */
    ADC_SAMPLETIME_28CYCLES_5 = 0x03U, /* Thời gian lấy mẫu 28.5 chu kỳ. */
    ADC_SAMPLETIME_41CYCLES_5 = 0x04U, /* Thời gian lấy mẫu 41.5 chu kỳ. */
    ADC_SAMPLETIME_55CYCLES_5 = 0x05U, /* Thời gian lấy mẫu 55.5 chu kỳ. */
    ADC_SAMPLETIME_71CYCLES_5 = 0x06U, /* Thời gian lấy mẫu 71.5 chu kỳ. */
    ADC_SAMPLETIME_239CYCLES_5 = 0x07U /* Thời gian lấy mẫu 239.5 chu kỳ. */
} Adc_SamplingTimeType;


/**
 * ADC channel group configuration.
 */
typedef struct
{
    Adc_GroupType groupID;                          /* Xác định group ADC, sau đó define sẽ nhận*/
    /* Các param config cho một regular channel*/
    struct Adc_ChannelConfigType
    {
        Adc_ChannelType channelID;                  /* Tên Channel được cấu hình*/
        uint8 rank;                                 /* Mức độ ưu tiên */
        Adc_SamplingTimeType sampleingTime;         /* THời gian lấy mẫu */
    }channelConfig[16U];                            /* 16 Channel thuộc 2 Group ADC1, ADC2 được hỗ trợ ở MCU*/
    /* Config thuộc group type*/
    uint8 numOfChannel;                             /* Số lượng channel được cấu hình*/
    Adc_GroupConvModeType groupConvMode;            /* Chế độ chuyển đổi mẫu*/
    Adc_TriggerSourceType triggerSource;            /* Chế độ Trigger */
    uint8 hwTriggerSource;                          /* Báo Trigger bằng HW*/
    Adc_GroupAccessModeType groupAccessMode;        /* Chế độ truy cập mẫu dữ liệu */
    Adc_StreamBufferModeType streamBufferMode;      /* Chế độ đọc mẫu lữ liệu*/
    Adc_ReadModeType readMode;                      /* Chế độ đọc có sử dụng DMA hay không*/
} Adc_GroupConfigType;

/**
 * ADC driver initialization configuration.
 */
typedef struct
{
    const Adc_GroupConfigType* groupConfigPtr;
    uint8 numOfGroup;
} Adc_ConfigType;

#endif /* ADC_TYPES_H */
