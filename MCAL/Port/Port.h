/**
 * @file Port.h
 *
 * @brief Port driver public interface for AUTOSAR Classic Platform.
 *
 * @author Duy Dang
 *
 * @date 16-06-2026
 *
 * @version v1.0.0
 */

#ifndef PORT_H
#define PORT_H

#include <Std_Types.h>

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * Data type for the symbolic name of a port pin.
 */
typedef uint8 Port_PinType;

/**
 * Different port pin modes.
 */
typedef uint8 Port_PinModeType;

/**
 * Possible directions of a port pin.
 */
typedef enum
{
    PORT_PIN_IN = 0x00U,    /* Sets port pin as input. */
    PORT_PIN_OUT            /* Sets port pin as output. */
} Port_PinDirectionType;

/**
 * Type of one configured port pin.
 */
typedef struct
{
    uint8 portID;                       /* GPIO port identifier. */
    Port_PinType pinID;                 /* Pin identifier inside the selected port. */
    Port_PinModeType pinMode;           /* Pin mode configuration. */
    uint8 profileMode;                  /* Cấu hình của một chân khi bật một mode */
    uint8 pinSpeed;                     /* Pin output speed configuration. */
    Port_PinDirectionType pinDirection; /* Initial pin direction. */
    uint8 levelPin;                     /* Initial output level. */
    uint8 changeDirection;              /* Runtime direction change permission. */
    uint8 changeMode;                   /* Runtime mode change permission. */
} Port_PinConfigType;

/**
 * Type of the external data structure containing the initialization data.
 */
typedef struct
{
    const Port_PinConfigType* pinConfig; /* Pointer to configured port pins. */
    uint16 numOfPin;                     /* Number of configured pins. */
} Port_ConfigType;

/*******************************************************************************
 * Function Declarations
 ******************************************************************************/

/**
 * @brief Initializes the Port Driver module.
 *
 * @param[in] ConfigPtr Pointer to configuration set.
 */
void Port_Init(const Port_ConfigType* ConfigPtr);

/**
 * @brief Sets the port pin direction.
 *
 * @param[in] Pin Port pin ID number.
 * @param[in] Direction Port pin direction.
 */
void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction);

/**
 * @brief Refreshes the direction of all configured port pins.
 */
void Port_RefreshPortDirection(void);

/**
 * @brief Returns the version information of this module.
 *
 * @param[in] versioninfo Pointer to where to store the version information.
 */
void Port_GetVersionInfo(Std_VersionInfoType* versioninfo);

/**
 * @brief Sets the port pin mode.
 *
 * @param[in] Pin Port pin ID number.
 * @param[in] Mode New port pin mode.
 */
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode);

#endif /* PORT_H */
