/**
 * @file    Adc.h
 * @brief   ADC driver public interface for AUTOSAR Classic Platform.
 * @author  Duy Dang
 * @date    15-06-2026
 * @version v1.0.0
 */

#ifndef ADC_H
#define ADC_H

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <Std_Types.h>
#include "Adc_Types.h"

/*******************************************************************************
 * Function Declarations
 ******************************************************************************/

/**
 * @brief Initializes the ADC hardware units and driver.
 *
 * @param[in] ConfigPtr Pointer to configuration set in Variant PB.
 */
void Adc_Init(const Adc_ConfigType* ConfigPtr);

/**
 * @brief Initializes result buffer pointer for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 * @param[in] DataBufferPtr Pointer to result data buffer.
 *
 * @return E_OK if the result buffer was initialized, otherwise E_NOT_OK.
 */
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr);

/**
 * @brief De-initializes the ADC driver.
 */
void Adc_DeInit(void);

/**
 * @brief Starts conversion for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 */
void Adc_StartGroupConversion(Adc_GroupType Group);

/**
 * @brief Stops conversion for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 */
void Adc_StopGroupConversion(Adc_GroupType Group);

/**
 * @brief Reads the latest conversion result of an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 * @param[out] DataBufferPtr Pointer to result data buffer.
 *
 * @return E_OK if data was read, otherwise E_NOT_OK.
 */
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr);

/**
 * @brief Enables the hardware trigger for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 */
void Adc_EnableHardwareTrigger(Adc_GroupType Group);

/**
 * @brief Disables the hardware trigger for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 */
void Adc_DisableHardwareTrigger(Adc_GroupType Group);

/**
 * @brief Enables the notification callback for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 */
void Adc_EnableGroupNotification(Adc_GroupType Group);

/**
 * @brief Disables the notification callback for an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 */
void Adc_DisableGroupNotification(Adc_GroupType Group);

/**
 * @brief Gets the current status of an ADC channel group.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 *
 * @return Current group status.
 */
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group);

/**
 * @brief Gets the latest stream result pointer and number of valid samples.
 *
 * @param[in] Group Numeric ID of requested ADC channel group.
 * @param[out] PtrToSamplePtr Pointer receiving latest sample pointer.
 *
 * @return Number of valid samples currently available.
 */
Adc_StreamNumSampleType Adc_GetStreamLastPointer(
    Adc_GroupType Group,
    Adc_ValueGroupType** PtrToSamplePtr);

/**
 * @brief Gets ADC driver version information.
 *
 * @param[out] VersionInfo Pointer to version information storage.
 */
void Adc_GetVersionInfo(Std_VersionInfoType* VersionInfo);

/**
 * @brief Applies a prepared ADC power state transition.
 *
 * @param[out] Result Pointer receiving power state request result.
 *
 * @return E_OK if the transition was applied, otherwise E_NOT_OK.
 */
Std_ReturnType Adc_SetPowerState(Adc_PowerStateRequestResultType* Result);

/**
 * @brief Gets the current ADC power state.
 *
 * @param[out] CurrentPowerState Pointer receiving current power state.
 * @param[out] Result Pointer receiving power state request result.
 *
 * @return E_OK if the state was read, otherwise E_NOT_OK.
 */
Std_ReturnType Adc_GetCurrentPowerState(
    Adc_PowerStateType* CurrentPowerState,
    Adc_PowerStateRequestResultType* Result);

/**
 * @brief Gets the prepared ADC target power state.
 *
 * @param[out] TargetPowerState Pointer receiving target power state.
 * @param[out] Result Pointer receiving power state request result.
 *
 * @return E_OK if the state was read, otherwise E_NOT_OK.
 */
Std_ReturnType Adc_GetTargetPowerState(
    Adc_PowerStateType* TargetPowerState,
    Adc_PowerStateRequestResultType* Result);

/**
 * @brief Prepares an ADC target power state.
 *
 * @param[in] PowerState Target power state to prepare.
 * @param[out] Result Pointer receiving power state request result.
 *
 * @return E_OK if the state was prepared, otherwise E_NOT_OK.
 */
Std_ReturnType Adc_PreparePowerState(
    Adc_PowerStateType PowerState,
    Adc_PowerStateRequestResultType* Result);

/**
 * @brief Handles asynchronous ADC power state transitions.
 */
void Adc_Main_PowerTransitionManager(void);

#endif /* ADC_H */
