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
#include "edu-ciaa.h"

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
    digital_output_t led_red;    /**< LED color rojo */
    digital_output_t led_green;  /**< LED color verde */
    digital_output_t led_blue;   /**< LED color azul */
    digital_output_t led_yellow; /**< LED color amarillo */
    digital_input_t tec_1;       /**< Tecla 1 */
    digital_input_t tec_2;       /**< Tecla 2 */
    digital_input_t tec_3;       /**< Tecla 3 */
    digital_input_t tec_4;       /**< Tecla 4 */
} const * board_t;

/* === Public variable declarations ================================================================================ */

/* === Public function declarations ================================================================================ */

/**
 * @brief Crea una estructura con los periféricos de la placa.
 *
 * Inicializa los LEDs y teclas, y devuelve un puntero a la estructura que los contiene.
 *
 * @return Puntero a la estructura de la placa.
 */
board_t BoardCreate(void);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* BSP_H_ */