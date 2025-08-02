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

/** @file clock.c
 ** @brief Implementación del módulo reloj con alarma
 **/

/* === Headers files inclusions ==================================================================================== */

#include "clock.h"
#include <stddef.h>
#include <string.h>

/* === Macros definitions ========================================================================================== */

/* === Private data type declarations ============================================================================== */

/**
 * @brief Estructura interna que almacena el estado del reloj y la alarma
 */
struct clock_s {
    clock_time_t current_time; /**< Hora actual */
    clock_time_t alarm_time;   /**< Hora configurada para la alarma */
    clock_time_t snooze_time;  /** < Hora a la que se pospuso la alarma */

    uint16_t alarm_delta_minutes; /**< Minutos adicionales de repetición (snooze) */

    uint16_t ticks_counter;   /**< Contador de ticks acumulados */
    uint8_t ticks_per_second; /**< Número de ticks que conforman un segundo */

    bool is_valid_current_time; /**< Indica si la hora actual es válida */
    bool is_valid_alarm_time;   /**< Indica si la hora de alarma es válida */
    bool is_alarm_enabled;      /**< Estado de habilitación de la alarma */
    bool is_alarm_ringing;      /**< Indica si la alarma está sonando */
    bool is_snooze_active;      /**< Indica si la alarma fue pospuesta */

    alarm_driver_t alarm_driver; /**< Driver para activar y desactivar la alarma */
};

/* === Private function declarations =============================================================================== */

/**
 * @brief Convierte una hora en formato BCD a segundos totales desde medianoche
 * @param time Puntero a la estructura con hora en BCD
 * @return Cantidad total de segundos representada por la hora
 */
static uint32_t BCDToSeconds(const clock_time_t * time) {
    uint8_t hours = time->time.hours[0] + time->time.hours[1] * 10;
    uint8_t minutes = time->time.minutes[0] + time->time.minutes[1] * 10;
    uint8_t seconds = time->time.seconds[0] + time->time.seconds[1] * 10;

    return (uint32_t)hours * 3600 + minutes * 60 + seconds;
}

/**
 * @brief Convierte segundos totales desde las 00:00 a formato BCD y los almacena en time
 * @param totalSeconds Segundos totales a convertir (se toma módulo 24h)
 * @param time Puntero a la estructura donde se almacenará la hora en BCD
 */
static void SecondsToBCD(uint32_t totalSeconds, clock_time_t * time) {
    if (time == NULL) {
        return;
    }

    totalSeconds %= 24 * 3600;

    uint8_t hours = totalSeconds / 3600;
    uint8_t minutes = (totalSeconds % 3600) / 60;
    uint8_t seconds = totalSeconds % 60;

    time->time.hours[1] = hours / 10;
    time->time.hours[0] = hours % 10;
    time->time.minutes[1] = minutes / 10;
    time->time.minutes[0] = minutes % 10;
    time->time.seconds[1] = seconds / 10;
    time->time.seconds[0] = seconds % 10;
}

/**
 * @brief Incrementa la hora actual del reloj en un segundo
 * @param self Instancia del reloj
 */
static void IncrementTime(clock_t self) {
    if (self == NULL || !self->is_valid_current_time) {
        return;
    }

    uint32_t totalSeconds = BCDToSeconds(&self->current_time);
    totalSeconds += 1;

    SecondsToBCD(totalSeconds, &self->current_time);
}

/**
 * @brief Verifica si es momento de activar la alarma y la activa si corresponde
 * @param self Instancia del reloj
 */
static void checkAlarm(clock_t self) {
    if (self == NULL || !self->is_valid_current_time || !self->is_alarm_enabled || !self->is_valid_alarm_time) {
        return;
    }

    if (self->is_snooze_active) {
        if (memcmp(&self->current_time, &self->snooze_time, sizeof(clock_time_t)) == 0) {
            self->alarm_driver->activate();
            self->is_alarm_ringing = true;
            self->is_snooze_active = false;
        }
        return;
    }

    if (memcmp(&self->current_time, &self->alarm_time, sizeof(clock_time_t)) == 0) {
        self->alarm_driver->activate();
        self->is_alarm_ringing = true;
    }
}

/**
 * @brief Valida que una estructura de tiempo contenga una hora válida
 * @param time Puntero al tiempo a validar
 * @return true si la hora es válida, false en caso contrario
 */
static bool IsValidClockTime(const clock_time_t * time) {
    uint8_t hourTens = time->time.hours[1];
    uint8_t hourUnits = time->time.hours[0];
    uint8_t minuteTens = time->time.minutes[1];
    uint8_t minuteUnits = time->time.minutes[0];
    uint8_t secondTens = time->time.seconds[1];
    uint8_t secondUnits = time->time.seconds[0];

    bool validHours = hourTens < 2 || (hourTens == 2 && hourUnits <= 3);
    bool validMinutes = minuteTens <= 5 && minuteUnits <= 9;
    bool validSeconds = secondTens <= 5 && secondUnits <= 9;

    return validHours && validMinutes && validSeconds;
}

/* === Private variable definitions ================================================================================ */

/* === Public variable definitions ================================================================================= */

/* === Private function definitions ================================================================================ */

/* === Public function implementation ============================================================================== */

clock_t ClockCreate(uint16_t ticks_per_second, alarm_driver_t alarm_driver) {
    static struct clock_s self[1];
    memset(self, 0, sizeof(struct clock_s));

    self->is_valid_current_time = false;
    self->ticks_per_second = ticks_per_second;
    self->ticks_counter = 0;
    self->alarm_delta_minutes = 0;

    self->is_valid_alarm_time = false;
    self->is_alarm_enabled = false;
    self->is_snooze_active = false;

    self->alarm_driver = alarm_driver;

    return self;
}

bool ClockGetTime(clock_t self, clock_time_t * result) {
    if (result == NULL || self == NULL) {
        return false;
    };

    memcpy(result, &self->current_time, sizeof(clock_time_t));
    return self->is_valid_current_time;
}

bool ClockSetTime(clock_t self, const clock_time_t * new_time) {
    if (new_time == NULL || self == NULL) {
        return false;
    }

    self->is_valid_current_time = IsValidClockTime(new_time);
    memcpy(&self->current_time, new_time, sizeof(clock_time_t));
    return self->is_valid_current_time;
}

void ClockNewTick(clock_t self) {
    if (self == NULL || !self->is_valid_current_time) {
        return;
    }

    self->ticks_counter++;
    if (self->ticks_counter >= self->ticks_per_second) {
        self->ticks_counter = 0;

        IncrementTime(self);
        checkAlarm(self);
    }
}

bool ClockIsCurrentTimeValid(clock_t self) {
    if (self == NULL) {
        return false;
    }

    return self->is_valid_current_time;
}

bool ClockSetAlarmTime(clock_t self, const clock_time_t * alarm_time) {
    if (self == NULL || alarm_time == NULL || !self->is_valid_current_time) {
        return false;
    }

    if (!IsValidClockTime(alarm_time)) {
        self->is_valid_alarm_time = false;
        self->is_alarm_enabled = false;

        return self->is_valid_alarm_time;
    }

    memcpy(&self->alarm_time, alarm_time, sizeof(clock_time_t));
    self->is_valid_alarm_time = true;
    self->is_alarm_enabled = true;
    self->is_snooze_active = false;

    return self->is_valid_alarm_time;
}

bool ClockGetAlarmTime(clock_t self, clock_time_t * result) {
    if (self == NULL || result == NULL) {
        return false;
    }

    memcpy(result, &self->alarm_time, sizeof(clock_time_t));
    return self->is_valid_alarm_time;
}

bool ClockSetAlarmState(clock_t self, AlarmStates state) {
    if (self == NULL || !self->is_valid_alarm_time || !self->is_valid_current_time) {
        return false;
    }

    switch (state) {
    case ALARM_ENABLE:
        self->is_alarm_enabled = true;
        break;
    case ALARM_DISABLE:
        self->is_alarm_enabled = false;
        break;
    default:
        break;
    }

    return self->is_alarm_enabled;
}

bool ClockIsAlarmEnabled(clock_t self) {
    if (self == NULL) {
        return false;
    }

    return self->is_alarm_enabled;
}

void ClockSnoozeAlarm(clock_t self, uint8_t minutes) {
    if (self == NULL || !self->is_valid_alarm_time || !self->is_alarm_enabled || !self->is_alarm_ringing) {
        return;
    }

    self->alarm_delta_minutes = minutes;

    uint32_t current_secs = BCDToSeconds(&self->current_time);
    current_secs += minutes * 60;
    SecondsToBCD(current_secs, &self->snooze_time);

    self->is_snooze_active = true;
    self->is_alarm_ringing = false;
    self->alarm_driver->deactivate();
}

void ClockFinishAlarm(clock_t self) {
    if (self == NULL || !self->is_alarm_enabled || !self->is_alarm_ringing) {
        return;
    }

    self->is_alarm_ringing = false;
    self->alarm_delta_minutes = 0;
    self->is_snooze_active = false;

    self->alarm_driver->deactivate();
}

bool ClockIsAlarmRinging(clock_t self) {
    if (self == NULL) {
        return false;
    }

    return self->is_alarm_ringing;
}

/* === End of documentation ======================================================================================== */
