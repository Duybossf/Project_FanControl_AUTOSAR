/**
 * @file Std_Types.h
 * 
 * @brief This is standard value AUTOSAR Classic Platform
 * 
 * @author Duy Dang
 * 
 * @date 10-06-2026
 * 
 * @version v1.0.0
 * 
 */
/****************************************************************************** */
#ifndef STD_TYPES_H
#define STD_TYPES_H

#include "Platform_Types.h"

 /**This type can be used as standard API return type which is shared between the RTE and the BSW
modules */
typedef uint8 Std_ReturnType;

#define E_OK     ((Std_ReturnType)0x00U)
#define E_NOT_OK ((Std_ReturnType)0x01U)

#define STD_HIGH 0x01U
#define STD_LOW  0x00U

#define STD_ACTIVE 0x01U
#define STD_IDLE   0x00U

#define STD_ON  0x01U
#define STD_OFF 0x00U

/*The implementation shall provide the NULL_PTR define with a void pointer to zero definition */
#ifndef NULL_PTR
#define NULL_PTR ((void *)0)
#endif

/*This type shall be used to request the version of a BSW module using the <Module name>_Get
VersionInfo() function.*/
typedef struct
{
    uint16 vendorID;
    uint16 moduleID;
    uint8 sw_major_version;
    uint8 sw_minor_version;
    uint8 sw_patch_version;
} Std_VersionInfoType;

#endif /* STD_TYPES_H */


