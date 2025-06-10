/*********************************************************************************************************************
Copyright (c) 2025, Natalia Carolina Borbón <nataliacborbon@gmail.com>
Copyright (c) 2025, Laboratorio de Microprocesadores, Universidad Nacional de Tucumán, Argentina

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SPDX-License-Identifier: MIT
*********************************************************************************************************************/

#ifndef SCREEN_H_
#define SCREEN_H_

/** @file screen.h
 ** @brief Declaraciones del módulo para la gestión de una pantalla multiplexada de 7 segmentos.
 **/

/* === Headers files inclusions ==================================================================================== */

#include <stdint.h>

/* === Header for C++ compatibility ================================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */

/**
 * @brief Definiciones de bits para cada segmento del display de 7 segmentos.
 * Incluyen los segmentos A-G y el punto decimal (P).
 */
#define SEGMENT_A (1 << 0)
#define SEGMENT_B (1 << 1)
#define SEGMENT_C (1 << 2)
#define SEGMENT_D (1 << 3)
#define SEGMENT_E (1 << 4)
#define SEGMENT_F (1 << 5)
#define SEGMENT_G (1 << 6)
#define SEGMENT_P (1 << 7)

/* === Public data type declarations =============================================================================== */

/**
 * @brief Puntero a una estructura que representa una pantalla multiplexada de 7 segmentos.
 */
typedef struct screen_s * screen_t;

/**
 * @brief Puntero a función que apaga todos los dígitos del display.
 */
typedef void (*digits_turn_off_t)(void);

/**
 * @brief Puntero a función que actualiza los segmentos a encender.
 */
typedef void (*segments_update_t)(uint8_t);

/**
 * @brief Puntero a función que enciende un dígito específico.
 */
typedef void (*digit_turn_on_t)(uint8_t);

/**
 * @brief Estructura que agrupa las funciones de bajo nivel que controlan el hardware del display.
 */
typedef struct screen_driver_s {
    digits_turn_off_t DigitsTurnOff;
    segments_update_t SegmentsUpdate;
    digit_turn_on_t DigitTurnOn;
} const * screen_driver_t;

/* === Public variable declarations ================================================================================ */

/* === Public function declarations ================================================================================ */

/**
 * @brief Crea una nueva instancia de pantalla multiplexada.
 *
 * @param digits     Cantidad de dígitos que tiene el display.
 * @param driver     Puntero a estructura con funciones específicas del hardware.
 * @return screen_t  Puntero a la nueva instancia de pantalla.
 */
screen_t ScreenCreate(uint8_t digits, screen_driver_t driver);

/**
 * @brief Escribe un arreglo de valores BCD en el display.
 *
 * @param screen  Instancia de pantalla.
 * @param value   Arreglo con los dígitos en formato BCD (0-9).
 * @param size    Cantidad de dígitos a mostrar.
 */
void ScreenWriteBCD(screen_t screen, uint8_t value[], uint8_t size);

/**
 * @brief Refresca el estado de la pantalla.
 *
 * @param screen  Instancia de pantalla.
 */
void ScreenRefresh(screen_t screen);

/**
 * @brief Hace parpadear un rango de dígitos del display.
 *
 * @param screen   Instancia de pantalla.
 * @param from     Dígito inicial del rango (inclusive).
 * @param to       Dígito final del rango (inclusive).
 * @param divisor  Frecuencia de parpadeo.
 * @return int     0 si fue exitoso, -1 si hubo error.
 */
int DisplayFlashDigits(screen_t screen, uint8_t from, uint8_t to, uint16_t divisor);

/**
 * @brief Enciende el punto decimal de un dígito.
 *
 * @param screen  Instancia de pantalla.
 * @param digit   Índice del dígito.
 */
void ScreenSetPoint(screen_t screen, uint8_t digit);

/**
 * @brief Apaga el punto decimal de un dígito.
 *
 * @param screen  Instancia de pantalla.
 * @param digit   Índice del dígito.
 */
void ScreenClearPoint(screen_t screen, uint8_t digit);

/**
 * @brief Hace parpadear el punto decimal de un dígito.
 *
 * @param screen   Instancia de pantalla.
 * @param digit    Índice del dígito.
 * @param divisor  Frecuencia de parpadeo.
 */
void ScreenFlashPoint(screen_t screen, uint8_t digit, uint8_t divisor);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* SCREEN_H_ */
