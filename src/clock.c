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
 ** @brief Plantilla para la creación de archivos de código fuente en lenguaje C
 **/

/* === Headers files inclusions ==================================================================================== */

#include "clock.h"
#include <stddef.h>
#include <string.h>

/* === Macros definitions ========================================================================================== */

/* === Private data type declarations ============================================================================== */

typedef struct clock_s {
    clock_time_t current_time;
    clock_time_t alarm_time;

    uint8_t alarm_delta;

    uint16_t ticks_counter;
    uint8_t ticks_per_second;

    bool is_valid_current_time;
    bool is_valid_alarm_time;
    bool is_alarm_enabled;
    bool has_alarm_to_ring;
    bool is_alarm_ringing;
};

/* === Private function declarations =============================================================================== */

static void IncrementTime(clock_t self) {
}

static void checkAlarm(clock_t self) {
    if (self == NULL || !self->is_valid_current_time || !self->is_valid_alarm_time || !self->is_alarm_enabled) {
        return;
    }

    if (memcmp(&self->current_time, &self->alarm_time, sizeof(clock_time_t)) == 0) {
        // GENERATE EVENT TO RING ALARM
        self->is_alarm_ringing = true;
    }
}

static bool IsValidClockTime(clock_time_t * time) {
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

clock_t ClockCreate(uint16_t ticks_per_second) {
    static struct clock_s self[1];
    memset(self, 0, sizeof(struct clock_s));

    self->is_valid_current_time = false;
    self->ticks_per_second = ticks_per_second;
    self->ticks_counter = 0;
    self->alarm_delta = 0;

    self->is_valid_alarm_time = false;
    self->is_alarm_enabled = false;
    self->has_alarm_to_ring = false;

    return self;
}

bool ClockGetTime(clock_t self, clock_time_t * result) {
    if (result == NULL) {
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
    return true;
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
    if (self == NULL || alarm_time == NULL) {
        return false;
    }

    if (!IsValidClockTime(alarm_time)) {
        self->is_valid_alarm_time = false;
        self->is_alarm_enabled = false;
        self->has_alarm_to_ring = false;

        return false;
    }

    memcpy(&self->alarm_time, alarm_time, sizeof(clock_time_t));
    self->is_valid_alarm_time = true;
    self->is_alarm_enabled = true;
    self->has_alarm_to_ring = true;

    return true;
}

bool ClockGetAlarmTime(clock_t self, const clock_time_t * result) {
    if (self == NULL || result == NULL) {
        return false;
    }

    memcpy(result, &self->alarm_time, sizeof(clock_time_t));
    return self->is_valid_alarm_time;
}

bool ClockSetAlarmState(clock_t self, AlarmStates state) {
    if (self == NULL) {
        return false;
    }

    switch (state) {
    case ENABLE:
        self->is_alarm_enabled = true;
        self->has_alarm_to_ring = true;
        break;
    case DISABLE:
        self->is_alarm_enabled = false;
        self->has_alarm_to_ring = false;
        break;
    default:
        break;
    }

    return true;
}

bool ClockIsAlarmEnabled(clock_t self) {
    if (self == NULL) {
        return false;
    }

    return self->is_alarm_enabled;
}

void ClockSnoozeAlarm(clock_t self, uint8_t minutes) {
    if (self == NULL || !self->is_valid_alarm_time || !self->is_alarm_enabled) {
        return;
    }

    // logica para posponer la alarma
}

void ClockFinishAlarm(clock_t self) {
    if (self == NULL || !self->is_alarm_enabled || !self->is_alarm_ringing) {
        return;
    }

    self->is_alarm_ringing = false;
    self->alarm_delta = 0;
    // GENERETE EVENT TO FINISH ALARM
    self->has_alarm_to_ring = false;
}

/* === End of documentation ======================================================================================== */
