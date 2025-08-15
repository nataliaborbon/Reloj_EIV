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

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

/** @file keyboard.h
 ** @brief Cabecera para manejar la inicialización y tareas de teclado.
 **/

/* === Headers files inclusions ==================================================================================== */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "digital.h"
#include "bsp.h"
#include <stdint.h>
#include <stdbool.h>

/* === Header for C++ compatibility ================================================================================ */
#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */

#define KEY_TASK_PERIOD_MS  10   /**< Periodo de muestreo de las teclas en ms */
#define HOLD_TIME_MS        3000 /**< Tiempo para considerar presionado un botón largo */
#define TOLERANCE           100  /**< Tolerancia en ms para evitar rebotes de lectura */

#define EVENT_KEY_ACCEPT    (1 << 0) /**< Evento tecla aceptar */
#define EVENT_KEY_CANCEL    (1 << 1) /**< Evento tecla cancelar */
#define EVENT_KEY_SET_TIME  (1 << 2) /**< Evento tecla ajustar hora */
#define EVENT_KEY_SET_ALARM (1 << 3) /**< Evento tecla ajustar alarma */
#define EVENT_KEY_INCREMENT (1 << 4) /**< Evento tecla incrementar valor */
#define EVENT_KEY_DECREMENT (1 << 5) /**< Evento tecla decrementar valor */

/* === Public data type declarations =============================================================================== */
typedef struct {
    digital_input_t key;      /**< Tecla física */
    uint8_t event_bit;        /**< Bit de evento asociado */
    uint32_t hold_ms;         /**< Tiempo para considerar "hold" */
    uint32_t pressed_count;   /**< Contador de tiempo presionado */
    uint32_t tolerance_count; /**< Contador de tolerancia para rebotes */
} key_task_params_t;

/* === Public variable declarations ================================================================================ */

extern EventGroupHandle_t key_events; /**< Grupo de eventos compartido entre teclas y control */
extern board_t board;

/* === Public function declarations ================================================================================ */

/**
 * @brief Inicializa el teclado: crea event group, tareas de lectura de teclas y configura los parámetros
 * @param board Estructura de la placa con las entradas de cada tecla
 */
void KeyboardInit(board_t board);

/**
 * @brief Tarea que gestiona la lectura de una tecla específica.
 * @param params Parámetros de la tecla (tipo key_task_params_t)
 */
void KeyTask(void * params);

#ifdef __cplusplus
}
#endif

#endif /* KEYBOARD_H_ */
