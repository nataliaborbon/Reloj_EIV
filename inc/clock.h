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

#ifndef CLOCK_H_
#define CLOCK_H_

/** @file clock.h
 ** @brief Declaraciones para el manejo de un reloj con funcionalidad de alarma.
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

/** @brief Puntero a función que activa una alarma */
typedef void (*alarm_activate_t)(void);

/** @brief Puntero a función que desactiva una alarma */
typedef void (*alarm_deactivate_t)(void);

/**
 * @brief Estructura que define la interfaz de control de la alarma
 */
typedef struct alarm_driver_s {
    alarm_activate_t activate;     /**< Función para activar la alarma */
    alarm_deactivate_t deactivate; /**< Función para desactivar la alarma */
} const * alarm_driver_t;

/**
 * @brief Estados posibles para la alarma
 */
typedef enum AlarmStates {
    ALARM_DISABLE, //!< Desactiva la alarma
    ALARM_ENABLE   //!< Activa la alarma
} AlarmStates;

/**
 * @brief Estructura que representa el tiempo en formato BCD
 */
typedef union {
    struct {
        uint8_t seconds[2]; /**< Segundos en BCD (2 dígitos) */
        uint8_t minutes[2]; /**< Minutos en BCD (2 dígitos) */
        uint8_t hours[2];   /**< Horas en BCD (2 dígitos) */
    } time;
    uint8_t bcd[6]; /**< Tiempo completo como arreglo de 6 bytes en BCD */
} clock_time_t;

/**
 * @brief Puntero a estructura que representa una instancia del reloj
 */
typedef struct clock_s * clock_t;

/* === Public variable declarations ================================================================================ */

/* === Public function declarations ================================================================================ */

/**
 * @brief Crea e inicializa una nueva instancia del reloj
 * @param ticks_per_second Cantidad de tics por segundo
 * @param alarm_driver Interfaz para el control de la alarma
 * @return Puntero a la instancia estática del reloj
 */
clock_t ClockCreate(uint16_t ticks_per_second, alarm_driver_t alarm_driver);

/**
 * @brief Obtiene la hora actual almacenada en el reloj
 * @param clock Instancia del reloj
 * @param result Puntero donde se almacenará la hora obtenida
 * @return true si se obtuvo correctamente, false si no
 */
bool ClockGetTime(clock_t clock, clock_time_t * result);

/**
 * @brief Establece una nueva hora en el reloj
 * @param clock Instancia del reloj
 * @param new_time Puntero a la nueva hora a configurar
 * @return true si se configuró correctamente, false si no
 */
bool ClockSetTime(clock_t clock, const clock_time_t * new_time);

/**
 * @brief Notifica al reloj que ha transcurrido un nuevo tic
 * @param clock Instancia del reloj
 */
void ClockNewTick(clock_t clock);

/**
 * @brief Verifica si la hora actual es válida
 * @param clock Instancia del reloj
 * @return true si la hora es válida, false si no
 */
bool ClockIsCurrentTimeValid(clock_t clock);

/**
 * @brief Configura la hora de activación de la alarma
 * @param clock Instancia del reloj
 * @param alarm_time Hora deseada para la alarma
 * @return true si se configuró correctamente, false si no
 */
bool ClockSetAlarmTime(clock_t clock, const clock_time_t * alarm_time);

/**
 * @brief Obtiene la hora configurada para la alarma
 * @param clock Instancia del reloj
 * @param result Puntero donde se almacenará la hora de la alarma
 * @return true si se obtuvo correctamente, false si no
 */
bool ClockGetAlarmTime(clock_t clock, clock_time_t * result);

/**
 * @brief Habilita o deshabilita la alarma
 * @param clock Instancia del reloj
 * @param state Estado deseado (ENABLE o DISABLE)
 * @return true si la operación fue exitosa, false si no
 */
bool ClockSetAlarmState(clock_t clock, AlarmStates state);

/**
 * @brief Verifica si la alarma está habilitada
 * @param clock Instancia del reloj
 * @return true si está habilitada, false si no
 */
bool ClockIsAlarmEnabled(clock_t clock);

/**
 * @brief Pospone la alarma por una cantidad específica de minutos
 * @param clock Instancia del reloj
 * @param minutes Minutos de posposición
 */
void ClockSnoozeAlarm(clock_t clock, uint8_t minutes);

/**
 * @brief Finaliza la alarma (la apaga si está sonando)
 * @param clock Instancia del reloj
 */
void ClockFinishAlarm(clock_t clock);

/**
 * @brief Verifica si la alarma está sonando actualmente
 * @param clock Instancia del reloj
 * @return true si está sonando, false si no
 */
bool ClockIsAlarmRinging(clock_t clock);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_H_ */
