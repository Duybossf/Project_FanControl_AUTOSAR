/**
 * @file Dio.h
 * 
 * @brief This is standard Dio Driver AUTOSAR Classic Platform
 * 
 * @author Duy Dang
 * 
 * @date 10-06-2026
 * 
 * @version v1.0.0
 * 
 */
/****************************************************************************** */

#ifndef DIO_H
#define DIO_H

#include <Std_Types.h>

/****************************************************************************** */
/**
 * Numeric ID of a DIO channel
 */
typedef uint8 Dio_ChannelType;

/**
 * Numeric ID of a DIO port
 */
typedef uint16 Dio_PortType;

/**
 * Type for the definition of a channel group, which consists of several adjoining channels within a
 * port.
 */
typedef struct
{
    Dio_PortType port;      /**< Port on which the channel group is defined. */
    uint8 mask;             /**< Defines the positions of the channel group. */
    uint8 offset;           /**< Position of the channel group on the port, counted from the LSB. */
} Dio_ChannelGroupType;

/**
 * These are the possible levels a DIO channel can have (input or output). The symbols STD_LOW
 * (Physical state 0V) and STD_HIGH (Physical state 5V or 3.3V) shall be reused from Std_Types.h.
 */
typedef uint8 Dio_LevelType;

/**
 * If the microcontroller owns ports of different port widths (e.g. 4, 8, 16 bits),
 * Dio_PortLevelType inherits the size of the largest port.
 */
typedef uint16 Dio_PortLevelType;

/****************************************************************************** */
/**
 * @brief Returns the level of the specified DIO channel.
 *
 * @param[in] ChannelId ID of the DIO channel.
 *
 * @return STD_HIGH Physical level of the corresponding pin is high.
 *         STD_LOW  Physical level of the corresponding pin is low.
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId);

/**
 * @brief  Service to set a level of a channel.
 *
 * @param[in] ChannelId ID of the DIO channel.
 * @param[in] Level Value to be written
 */
void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level);

/**
 * @brief  Returns the level of all channels of that port.
 *
 * @param[in] PortId ID of the DIO port.
 *
 * @return Dio_PortLevelType.
 */
Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId);

/**
 * @brief  Service to set a value of the port.
 *
 * @param[in] PortId ID of the DIO port.
 * @param[in] Level Value to be written
 * 
 */
void Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level);

/**
 * @brief  This Service reads a subset of the adjoining bits of a port.
 *
 * @param[in] ChannelGroupIdPtr Pointer to ChannelGroup
 * 
 * @return Level of a subset of the adjoining bits of a port.
 */
Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr);

/**
 * @brief  Service to set a subset of the adjoining bits of a port to a specified level.
 *
 * @param[in] ChannelGroupIdPtr Pointer to ChannelGroup
 * @param[in] Level Value to be written
 * 
 */
void Dio_WriteChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr, Dio_PortLevelType Level);

/**
 * @brief   Service to get the version information of this module.
 *
 * @param[in] VersionInfo  Pointer to where to store the version information of this module.
 * 
 */
void Dio_GetVersionInfo(Std_VersionInfoType* VersionInfo);

/**
 * @brief   Service to flip the level of a channel and return the level after flip.
 *
 * @param[in] ChannelId   ID of DIO channel
 * 
 * @return STD_HIGH: The physical level of the corresponding Pin is STD_HIGH.
 *         STD_LOW: The physical level of the corresponding Pin is STD_LOW.
 */
Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId);

/**
 * @brief    Service to set the value of a given port with required mask.
 *
 * @param[in] PortId   ID of DIO Port
 * @param[in] Level  Value to be written
 * @param[in] Mask   Channels to be masked in the port
 * 
 */
void Dio_MaskedWritePort(Dio_PortType PortId, Dio_PortLevelType Level, Dio_PortLevelType Mask);

#endif /* DIO_H */
