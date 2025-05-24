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

#ifndef DIGITAL_H_
#define DIGITAL_H_

/** @file digital.h
 ** @brief Módulo para la gestión de entradas y salidas digitales.
 **/

/* === Headers files inclusions ==================================================================================== */

#include <stdint.h>
#include <stdbool.h>

/* === Header for C++ compatibility ================================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */

/* === Public data type declarations =============================================================================== */

/**
 * @brief Estados de cambio de una entrada digital.
 *
 * Esta enumeración define los posibles estados de transición que puede presentar una entrada digital
 * entre dos lecturas consecutivas. Es útil para detectar flancos o cambios de estado.
 */
typedef enum digital_states_e {
    DIGITAL_INPUT_WAS_DEACTIVATED = -1, /**< La entrada digital pasó de activada a desactivada. */
    DIGITAL_INPUT_NO_CHANGE = 0,        /**< No se detectó ningún cambio en el estado de la entrada digital. */
    DIGITAL_INPUT_WAS_ACTIVATED = 1,    /**< La entrada digital pasó de desactivada a activada. */
} digital_states_t;

/**
 * @brief Puntero a una instancia de una salida digital
 */
typedef struct digital_output_s * digital_output_t;

/**
 * @brief Puntero a una instancia de una entrada digital
 */
typedef struct digital_input_s * digital_input_t;

/* === Public variable declarations ================================================================================ */

/* === Public function declarations ================================================================================ */

/**
 * @brief Crea una salida digital.
 *
 * Esta función inicializa y configura una salida digital en un pin específico.
 *
 * @param gpio  Número del puerto GPIO a utilizar.
 * @param bit   Bit dentro del puerto GPIO que se utilizará.
 * @return digital_output_t  Puntero a la instancia de la salida digital creada.
 */
digital_output_t DigitalOutputCreate(uint8_t gpio, uint8_t bit);

/**
 * @brief Activa una salida digital.
 *
 * Esta función establece el pin correspondiente en estado alto (activo).
 *
 * @param output  Puntero a la instancia de la salida digital, obtenida mediante DigitalOutputCreate().
 */
void DigitalOutputActivate(digital_output_t output);

/**
 * @brief Desactiva una salida digital.
 *
 * Esta función establece el pin correspondiente en estado bajo (inactivo).
 *
 * @param output  Puntero a la instancia de la salida digital, obtenida mediante DigitalOutputCreate().
 */
void DigitalOutputDeactivate(digital_output_t output);

/**
 * @brief Cambia el estado de una salida digital.
 *
 * Esta función conmuta el estado actual de la salida digital: si estaba activa, la desactiva; y si estaba desactivada,
 * la activa.
 *
 * @param output  Puntero a la instancia de la salida digital, obtenida mediante DigitalOutputCreate().
 */
void DigitalOutputToggle(digital_output_t output);

/**
 * @brief Crea una entrada digital.
 *
 * Inicializa una entrada digital en un pin específico y permite configurar si su lógica está invertida.
 *
 * @param gpio      Número del puerto GPIO a utilizar.
 * @param bit       Bit dentro del puerto GPIO que se utilizará.
 * @param inverted  `true` si la lógica de la entrada es invertida; `false` en caso contrario.
 * @return digital_input_t  Puntero a la instancia de la entrada digital creada.
 */
digital_input_t DigitalInputCreate(uint8_t gpio, uint8_t bit, bool inverted);

/**
 * @brief Lee el estado actual de una entrada digital.
 *
 * @param input  Puntero a la instancia de la entrada digital, obtenida mediante DigitalInputCreate().
 * @return `true` si la entrada está activa; `false` en caso contrario.
 */
bool DigitalInputGetIsActive(digital_input_t input);

/**
 * @brief Verifica si el estado de una entrada digital ha cambiado.
 *
 * Determina si el estado de la entrada digital ha sido modificado desde la última verificación.
 *
 * @param input  Puntero a la instancia de la entrada digital, obtenida mediante DigitalInputCreate().
 * @return enum digital_states_e  Estado del cambio: activada, desactivada o sin cambios.
 */
enum digital_states_e DigitalInputWasChanged(digital_input_t input);

/**
 * @brief Verifica si una entrada digital fue activada.
 *
 * Devuelve `true` si la entrada cambió de inactiva a activa desde la última consulta.
 *
 * @param input  Puntero a la instancia de la entrada digital, obtenida mediante DigitalInputCreate().
 * @return `true` si la entrada fue activada; `false` en caso contrario.
 */
bool DigitalInputWasActivated(digital_input_t input);

/**
 * @brief Verifica si una entrada digital fue desactivada.
 *
 * Devuelve `true` si la entrada cambió de activa a inactiva desde la última consulta.
 *
 * @param input  Puntero a la instancia de la entrada digital, obtenida mediante DigitalInputCreate().
 * @return `true` si la entrada fue desactivada; `false` en caso contrario.
 */
bool DigitalInputWasDeactivated(digital_input_t input);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* DIGITAL_H_ */
