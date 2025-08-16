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

#ifndef CONTROL_H_
#define CONTROL_H_

/** @file control.h
 ** @brief Cabecera para manejar la máquina de estados del reloj, alarmas y ajuste de hora
 **
 ** Este módulo contiene las funciones y variables necesarias para controlar
 ** el flujo de la aplicación del reloj, incluyendo:
 **   - Cambiar modos de operación
 **   - Incrementar y decrementar valores BCD de horas y minutos
 **   - Gestionar la máquina de estados del reloj
 **   - Ejecutar la tarea de control para las entradas de teclado
 **/

/* === Headers files inclusions ==================================================================================== */
#include <stdint.h>
#include "globals.h"
#include "clock.h"

/* === Header for C++ compatibility ================================================================================ */
#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */
/** @brief Tiempo de inactividad en milisegundos para volver al modo SHOWING_TIME. */
#define INACTIVITY 30000

/* === Public data type declarations =============================================================================== */

/* === Public variable declarations ================================================================================ */

/** @brief Estado actual de la máquina de estados */
extern mode_t mode;

/** @brief Hora actual */
extern clock_time_t hour;

/** @brief Hora configurada para la alarma */
extern clock_time_t alarm;

/** @brief Hora en proceso de ajuste */
extern clock_time_t adjusting;

/** @brief Contador de milisegundos de inactividad */
extern volatile uint32_t inactivity_count;

/* === Public function declarations ================================================================================ */
/**
 * @brief Cambia el modo actual del reloj.
 * @param value Modo al que se desea cambiar.
 *
 * Actualiza la máquina de estados y controla la visualización en pantalla
 * según el modo seleccionado.
 */
void ChangeMode(mode_t value);

/**
 * @brief Tarea principal de control del reloj.
 * @param params Parámetros de la tarea (no se usan, se ignoran).
 *
 * Esta función debe ejecutarse como una tarea FreeRTOS. Se encarga de:
 *   - Leer eventos de teclado
 *   - Cambiar modos de operación
 *   - Ajustar hora y alarma
 *   - Gestionar inactividad
 */
void ControlTask(void * params);

#ifdef __cplusplus
}
#endif

#endif /* CONTROL_H_ */
