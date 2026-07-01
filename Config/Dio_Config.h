/**
 * @file Dio_Config.h
 *
 * @brief GPIO STM32F103C8T6 mapping for the DIO driver.
 *
 * @author Duy Dang
 *
 * @date 11-06-2026
 *
 * @version v1.0.0
 */

#ifndef DIO_CONFIG_H
#define DIO_CONFIG_H

#include <stm32f10x.h>

/* Linear DIO port identifiers for STM32 GPIO ports. */
#define PORTA_DIO (0U)
#define PORTB_DIO (1U)
#define PORTC_DIO (2U)
#define PORTD_DIO (3U)

/* Build a channel identifier from a linear DIO port and a pin number. */
#define DIO_CHANNEL_PIN(PORTx, PIN) ((((PORTx) << 4U) + (PIN)))

/* Defines DIO channel IDs for GPIO port A. */
#define DIO_CHANNEL_A0  (DIO_CHANNEL_PIN(PORTA_DIO, 0U))     /* Defines pin A0. */
#define DIO_CHANNEL_A1  (DIO_CHANNEL_PIN(PORTA_DIO, 1U))     /* Defines pin A1. */
#define DIO_CHANNEL_A2  (DIO_CHANNEL_PIN(PORTA_DIO, 2U))     /* Defines pin A2. */
#define DIO_CHANNEL_A3  (DIO_CHANNEL_PIN(PORTA_DIO, 3U))     /* Defines pin A3. */
#define DIO_CHANNEL_A4  (DIO_CHANNEL_PIN(PORTA_DIO, 4U))     /* Defines pin A4. */
#define DIO_CHANNEL_A5  (DIO_CHANNEL_PIN(PORTA_DIO, 5U))     /* Defines pin A5. */
#define DIO_CHANNEL_A6  (DIO_CHANNEL_PIN(PORTA_DIO, 6U))     /* Defines pin A6. */
#define DIO_CHANNEL_A7  (DIO_CHANNEL_PIN(PORTA_DIO, 7U))     /* Defines pin A7. */
#define DIO_CHANNEL_A8  (DIO_CHANNEL_PIN(PORTA_DIO, 8U))     /* Defines pin A8. */
#define DIO_CHANNEL_A9  (DIO_CHANNEL_PIN(PORTA_DIO, 9U))     /* Defines pin A9. */
#define DIO_CHANNEL_A10 (DIO_CHANNEL_PIN(PORTA_DIO, 10U))    /* Defines pin A10. */
#define DIO_CHANNEL_A11 (DIO_CHANNEL_PIN(PORTA_DIO, 11U))    /* Defines pin A11. */
#define DIO_CHANNEL_A12 (DIO_CHANNEL_PIN(PORTA_DIO, 12U))    /* Defines pin A12. */
#define DIO_CHANNEL_A13 (DIO_CHANNEL_PIN(PORTA_DIO, 13U))    /* Defines pin A13. */
#define DIO_CHANNEL_A14 (DIO_CHANNEL_PIN(PORTA_DIO, 14U))    /* Defines pin A14. */
#define DIO_CHANNEL_A15 (DIO_CHANNEL_PIN(PORTA_DIO, 15U))    /* Defines pin A15. */

/* Defines DIO channel IDs for GPIO port B. */
#define DIO_CHANNEL_B0  (DIO_CHANNEL_PIN(PORTB_DIO, 0U))     /* Defines pin B0. */
#define DIO_CHANNEL_B1  (DIO_CHANNEL_PIN(PORTB_DIO, 1U))     /* Defines pin B1. */
#define DIO_CHANNEL_B2  (DIO_CHANNEL_PIN(PORTB_DIO, 2U))     /* Defines pin B2. */
#define DIO_CHANNEL_B3  (DIO_CHANNEL_PIN(PORTB_DIO, 3U))     /* Defines pin B3. */
#define DIO_CHANNEL_B4  (DIO_CHANNEL_PIN(PORTB_DIO, 4U))     /* Defines pin B4. */
#define DIO_CHANNEL_B5  (DIO_CHANNEL_PIN(PORTB_DIO, 5U))     /* Defines pin B5. */
#define DIO_CHANNEL_B6  (DIO_CHANNEL_PIN(PORTB_DIO, 6U))     /* Defines pin B6. */
#define DIO_CHANNEL_B7  (DIO_CHANNEL_PIN(PORTB_DIO, 7U))     /* Defines pin B7. */
#define DIO_CHANNEL_B8  (DIO_CHANNEL_PIN(PORTB_DIO, 8U))     /* Defines pin B8. */
#define DIO_CHANNEL_B9  (DIO_CHANNEL_PIN(PORTB_DIO, 9U))     /* Defines pin B9. */
#define DIO_CHANNEL_B10 (DIO_CHANNEL_PIN(PORTB_DIO, 10U))    /* Defines pin B10. */
#define DIO_CHANNEL_B11 (DIO_CHANNEL_PIN(PORTB_DIO, 11U))    /* Defines pin B11. */
#define DIO_CHANNEL_B12 (DIO_CHANNEL_PIN(PORTB_DIO, 12U))    /* Defines pin B12. */
#define DIO_CHANNEL_B13 (DIO_CHANNEL_PIN(PORTB_DIO, 13U))    /* Defines pin B13. */
#define DIO_CHANNEL_B14 (DIO_CHANNEL_PIN(PORTB_DIO, 14U))    /* Defines pin B14. */
#define DIO_CHANNEL_B15 (DIO_CHANNEL_PIN(PORTB_DIO, 15U))    /* Defines pin B15. */
#define DIO_CHANNEL_B16 (DIO_CHANNEL_PIN(PORTB_DIO, 16U))    /* Defines pin B16. */

/* Defines DIO channel IDs for GPIO port C. */
#define DIO_CHANNEL_C0  (DIO_CHANNEL_PIN(PORTC_DIO, 0U))     /* Defines pin C0. */
#define DIO_CHANNEL_C1  (DIO_CHANNEL_PIN(PORTC_DIO, 1U))     /* Defines pin C1. */
#define DIO_CHANNEL_C2  (DIO_CHANNEL_PIN(PORTC_DIO, 2U))     /* Defines pin C2. */
#define DIO_CHANNEL_C3  (DIO_CHANNEL_PIN(PORTC_DIO, 3U))     /* Defines pin C3. */
#define DIO_CHANNEL_C4  (DIO_CHANNEL_PIN(PORTC_DIO, 4U))     /* Defines pin C4. */
#define DIO_CHANNEL_C5  (DIO_CHANNEL_PIN(PORTC_DIO, 5U))     /* Defines pin C5. */
#define DIO_CHANNEL_C6  (DIO_CHANNEL_PIN(PORTC_DIO, 6U))     /* Defines pin C6. */
#define DIO_CHANNEL_C7  (DIO_CHANNEL_PIN(PORTC_DIO, 7U))     /* Defines pin C7. */
#define DIO_CHANNEL_C8  (DIO_CHANNEL_PIN(PORTC_DIO, 8U))     /* Defines pin C8. */
#define DIO_CHANNEL_C9  (DIO_CHANNEL_PIN(PORTC_DIO, 9U))     /* Defines pin C9. */
#define DIO_CHANNEL_C10 (DIO_CHANNEL_PIN(PORTC_DIO, 10U))    /* Defines pin C10. */
#define DIO_CHANNEL_C11 (DIO_CHANNEL_PIN(PORTC_DIO, 11U))    /* Defines pin C11. */
#define DIO_CHANNEL_C12 (DIO_CHANNEL_PIN(PORTC_DIO, 12U))    /* Defines pin C12. */
#define DIO_CHANNEL_C13 (DIO_CHANNEL_PIN(PORTC_DIO, 13U))    /* Defines pin C13. */
#define DIO_CHANNEL_C14 (DIO_CHANNEL_PIN(PORTC_DIO, 14U))    /* Defines pin C14. */
#define DIO_CHANNEL_C15 (DIO_CHANNEL_PIN(PORTC_DIO, 15U))    /* Defines pin C15. */
#define DIO_CHANNEL_C16 (DIO_CHANNEL_PIN(PORTC_DIO, 16U))    /* Defines pin C16. */

/* Defines DIO channel IDs for GPIO port D. */
#define DIO_CHANNEL_D0  (DIO_CHANNEL_PIN(PORTD_DIO, 0U))     /* Defines pin D0. */
#define DIO_CHANNEL_D1  (DIO_CHANNEL_PIN(PORTD_DIO, 1U))     /* Defines pin D1. */
#define DIO_CHANNEL_D2  (DIO_CHANNEL_PIN(PORTD_DIO, 2U))     /* Defines pin D2. */
#define DIO_CHANNEL_D3  (DIO_CHANNEL_PIN(PORTD_DIO, 3U))     /* Defines pin D3. */
#define DIO_CHANNEL_D4  (DIO_CHANNEL_PIN(PORTD_DIO, 4U))     /* Defines pin D4. */
#define DIO_CHANNEL_D5  (DIO_CHANNEL_PIN(PORTD_DIO, 5U))     /* Defines pin D5. */
#define DIO_CHANNEL_D6  (DIO_CHANNEL_PIN(PORTD_DIO, 6U))     /* Defines pin D6. */
#define DIO_CHANNEL_D7  (DIO_CHANNEL_PIN(PORTD_DIO, 7U))     /* Defines pin D7. */
#define DIO_CHANNEL_D8  (DIO_CHANNEL_PIN(PORTD_DIO, 8U))     /* Defines pin D8. */
#define DIO_CHANNEL_D9  (DIO_CHANNEL_PIN(PORTD_DIO, 9U))     /* Defines pin D9. */
#define DIO_CHANNEL_D10 (DIO_CHANNEL_PIN(PORTD_DIO, 10U))    /* Defines pin D10. */
#define DIO_CHANNEL_D11 (DIO_CHANNEL_PIN(PORTD_DIO, 11U))    /* Defines pin D11. */
#define DIO_CHANNEL_D12 (DIO_CHANNEL_PIN(PORTD_DIO, 12U))    /* Defines pin D12. */
#define DIO_CHANNEL_D13 (DIO_CHANNEL_PIN(PORTD_DIO, 13U))    /* Defines pin D13. */
#define DIO_CHANNEL_D14 (DIO_CHANNEL_PIN(PORTD_DIO, 14U))    /* Defines pin D14. */
#define DIO_CHANNEL_D15 (DIO_CHANNEL_PIN(PORTD_DIO, 15U))    /* Defines pin D15. */
#define DIO_CHANNEL_D16 (DIO_CHANNEL_PIN(PORTD_DIO, 16U))    /* Defines pin D16. */

#endif /* DIO_CONFIG_H */
