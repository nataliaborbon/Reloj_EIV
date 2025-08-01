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

#ifndef BSP_H_
#define BSP_H_

/** @file bsp.h
 ** @brief Configuración básica del hardware de la placa
 **/

/* === Headers files inclusions ==================================================================================== */

#include "digital.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "poncho.h"
#include "screen.h"

/* === Header for C++ compatibility ================================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */

/* === Public data type declarations =============================================================================== */

/**
 * @brief Puntero constante a una estructura que representa las entradas y salidas digitales de la placa.
 *
 * La estructura agrupa salidas digitales (LEDs) y entradas digitales (teclas).
 *
 */
typedef struct board_s {
    digital_output_t buzzer;   /**< Salida para el zumbador */
    digital_input_t set_time;  /**< Tecla Fijar hora */
    digital_input_t set_alarm; /**< Tecla Fijar alarma */
    digital_input_t decrement; /**< Tecla Decrementar */
    digital_input_t increment; /**< Tecla Incrementar */
    digital_input_t accept;    /**< Tecla Aceptar */
    digital_input_t cancel;    /**< Tecla Cancelar */
    screen_t screen;           /**< Display 7 segmentos */
    digital_output_t led;      /**< Salida para el led */
} const * board_t;

/* === Public variable declarations ================================================================================ */

/* === Public function declarations ================================================================================ */

/**
 * @brief Crea una estructura con los periféricos de la placa.
 *
 * Inicializa el Led de salida, las teclas y el display, devuelve un puntero a la estructura que los contiene.
 *
 * @return Puntero a la estructura de la placa.
 */
board_t BoardCreate(void);

void SisTick_Init(uint16_t ticks);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* BSP_H_ */