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

#ifndef GLOBALS_H_
#define GLOBALS_H_

/**
 * @file globals.h
 * @brief Variables globales y tipos compartidos entre módulos
 */
/* === Headers files inclusions ==================================================================================== */

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "event_groups.h"
#include "bsp.h"
#include "clock.h"
#include <semphr.h>

/* === Header for C++ compatibility ================================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */

#define EVENT_KEY_ACCEPT    (1 << 0) /**< Evento tecla aceptar */
#define EVENT_KEY_CANCEL    (1 << 1) /**< Evento tecla cancelar */
#define EVENT_KEY_SET_TIME  (1 << 2) /**< Evento tecla ajustar hora */
#define EVENT_KEY_SET_ALARM (1 << 3) /**< Evento tecla ajustar alarma */
#define EVENT_KEY_INCREMENT (1 << 4) /**< Evento tecla incrementar valor */
#define EVENT_KEY_DECREMENT (1 << 5) /**< Evento tecla decrementar valor */

/* === Public data type declarations =============================================================================== */

/**
 * @brief Modos de operación del reloj.
 */
typedef enum {
    UNCONFIGURED,              /**< Estado inicial sin configuración. */
    SHOWING_TIME,              /**< Mostrando la hora actual. */
    ADJUSTING_CURRENT_MINUTES, /**< Modo de ajuste de los minutos actuales. */
    ADJUSTING_CURRENT_HOURS,   /**< Modo de ajuste de las horas actuales. */
    ADJUSTING_ALARM_MINUTES,   /**< Modo de ajuste de los minutos de la alarma. */
    ADJUSTING_ALARM_HOURS,     /**< Modo de ajuste de las horas de la alarma. */
} mode_t;

/* === Public variable declarations ================================================================================ */

// Variables globales accesibles desde cualquier módulo
extern clock_t reloj;                      /**< Reloj principal */
extern mode_t mode;                        /**< Modo actual del sistema */
extern volatile uint32_t inactivity_count; /**< Contador global de inactividad */
extern board_t board;                      /**< Puntero a la estructura principal del hardware */
extern EventGroupHandle_t key_events;      /**< Event group para eventos de teclas */
extern clock_time_t hour;                  /**< Hora actual del sistema */
extern clock_time_t alarm;                 /**< Hora configurada para la alarma */
extern clock_time_t adjusting;             /**< Valor temporal mientras se ajusta hora o alarma */
extern SemaphoreHandle_t hour_mutex;       /**< Mutex para proteger las variables de hora, alarma y adjusting */

/* === Public function declarations ================================================================================ */

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* GLOBALS_H_ */
