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

/**
 * @file test_clock.c
 * @brief Contiene el código fuente para la inicialización y configuración de las entradas y salidas digitales de la
 * placa.
 *
 */

/* === Headers files inclusions =============================================================== */

#include "unity.h"
#include "clock.h"

/* === Macros definitions ====================================================================== */

#define CLOCK_TICKS_PER_SECOND 1

#define TEST_ASSERT_TIME(hours_tens, hours_units, minutes_tens, minutes_units, seconds_tens, seconds_units,            \
                         current_time)                                                                                 \
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(seconds_units, current_time.bcd[0], "Difference in unit seconds");                 \
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(seconds_tens, current_time.bcd[1], "Difference in tens seconds");                  \
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(minutes_units, current_time.bcd[2], "Difference in unit minutes");                 \
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(minutes_tens, current_time.bcd[3], "Difference in tens minutes");                  \
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(hours_units, current_time.bcd[4], "Difference in unit hours");                     \
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(hours_tens, current_time.bcd[5], "Difference in tens hours");

/* === Private data type declarations ========================================================== */

/* === Private variable declarations =========================================================== */

clock_t clock;

static void AlarmActivate(void);
static void AlarmDeactivate(void);

static const struct alarm_driver_s alarm_driver = {
    .activate = AlarmActivate,
    .deactivate = AlarmDeactivate,
};
/* === Private function declarations =========================================================== */

static void AlarmActivate(void) {
    // Simulate alarm activation
}
static void AlarmDeactivate(void) {
    // Simulate alarm deactivation
}

static void SimulatedSeconds(clock_t clock, uint16_t seconds) {
    for (uint16_t i = 0; i < CLOCK_TICKS_PER_SECOND * seconds; i++) {
        ClockNewTick(clock);
    }
}

/* === Public variable definitions ============================================================= */

/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */

/* === Public function implementation ========================================================= */

void setUp(void) {
    clock = ClockCreate(CLOCK_TICKS_PER_SECOND, &alarm_driver);
}

void test_set_up_with_invalid_time(void) {
    clock_time_t current_time = {.bcd = {1, 2, 3, 4, 5, 6}};

    clock_t clock_local = ClockCreate(CLOCK_TICKS_PER_SECOND, &alarm_driver);
    TEST_ASSERT_FALSE(ClockGetTime(clock_local, &current_time));
    TEST_ASSERT_EACH_EQUAL_UINT8(0, current_time.bcd, 6);
}

void test_set_up_and_adjust_with_valid_time(void) {
    clock_time_t new_time = {.time = {
                                 .seconds = {0, 0},
                                 .minutes = {0, 0},
                                 .hours = {1, 0},
                             }};
    clock_time_t current_time = {0};

    TEST_ASSERT_TRUE(ClockSetTime(clock, &new_time));
    TEST_ASSERT_TRUE(ClockGetTime(clock, &current_time));
    TEST_ASSERT_TIME(0, 1, 0, 0, 0, 0, current_time);
}

void test_set_up_and_clock_advance_one_second(void) {
    clock_time_t current_time = {0};

    ClockSetTime(clock, &(clock_time_t){0});
    SimulatedSeconds(clock, 1);
    ClockGetTime(clock, &current_time);
    TEST_ASSERT_TIME(0, 0, 0, 0, 0, 1, current_time);
}

void test_set_up_and_clock_advance_ten_seconds(void) {
    clock_time_t current_time = {0};

    ClockSetTime(clock, &(clock_time_t){0});
    SimulatedSeconds(clock, 10);
    ClockGetTime(clock, &current_time);
    TEST_ASSERT_TIME(0, 0, 0, 0, 1, 0, current_time);
}

void test_clock_rollover_after_full_day(void) {
    clock_time_t almost_midnight = {.time = {.hours = {3, 2}, .minutes = {9, 5}, .seconds = {9, 5}}};
    ClockSetTime(clock, &almost_midnight);
    SimulatedSeconds(clock, 1); // avanzar 1 segundo
    clock_time_t current = {0};
    ClockGetTime(clock, &current);
    TEST_ASSERT_TIME(0, 0, 0, 0, 0, 0, current);
}

void test_clock_does_not_advance_before_ticks_per_second(void) {
    clock_t slow_clock = ClockCreate(5, &alarm_driver);
    clock_time_t now = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {0, 0}}};

    ClockSetTime(slow_clock, &now);
    for (int i = 0; i < 4; i++) ClockNewTick(slow_clock);

    clock_time_t current = {0};
    ClockGetTime(slow_clock, &current);
    TEST_ASSERT_TIME(0, 0, 0, 0, 0, 0, current);

    ClockNewTick(slow_clock);
    ClockGetTime(slow_clock, &current);
    TEST_ASSERT_TIME(0, 0, 0, 0, 0, 1, current);
}

void test_set_up_and_clock_starts_invalid_and_can_set_valid_time(void) {
    TEST_ASSERT_FALSE(ClockIsCurrentTimeValid(clock));

    clock_time_t valid_time = {.time = {.hours = {3, 2}, .minutes = {9, 5}, .seconds = {9, 5}}};
    TEST_ASSERT_TRUE(ClockSetTime(clock, &valid_time));
    TEST_ASSERT_TRUE(ClockIsCurrentTimeValid(clock));

    clock_time_t read_time = {0};
    TEST_ASSERT_TRUE(ClockGetTime(clock, &read_time));
    TEST_ASSERT_TIME(2, 3, 5, 9, 5, 9, read_time);
}

void test_set_up_and_clock_rejects_invalid_time(void) {
    clock_time_t invalid_time = {.time = {.hours = {4, 4}, .minutes = {0, 6}, .seconds = {0, 6}}};
    TEST_ASSERT_FALSE(ClockSetTime(clock, &invalid_time));
    TEST_ASSERT_FALSE(ClockSetAlarmTime(clock, &invalid_time));
}

void test_set_up_and_alarm_triggers_correctly(void) {
    clock_time_t now = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {0, 0}}};
    clock_time_t alarm = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {5, 0}}};

    TEST_ASSERT_TRUE(ClockSetTime(clock, &now));
    TEST_ASSERT_TRUE(ClockSetAlarmTime(clock, &alarm));

    SimulatedSeconds(clock, 5);

    TEST_ASSERT_TRUE(ClockIsAlarmRinging(clock));
}

void test_set_up_and_snooze_delays_alarm(void) {
    clock_time_t now = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {0, 0}}};
    clock_time_t alarm = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {0, 5}}};

    ClockSetTime(clock, &now);
    ClockSetAlarmTime(clock, &alarm);

    SimulatedSeconds(clock, 50);
    TEST_ASSERT_TRUE(ClockIsAlarmRinging(clock));

    ClockSnoozeAlarm(clock, 1); // +1 minuto

    TEST_ASSERT_FALSE(ClockIsAlarmRinging(clock));

    SimulatedSeconds(clock, 60);

    TEST_ASSERT_TRUE(ClockIsAlarmRinging(clock));
}

void test_set_up_and_finish_alarm_resets_ringing_and_delta(void) {
    clock_time_t now = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {0, 0}}};
    clock_time_t alarm = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {2, 0}}};

    ClockSetTime(clock, &now);
    ClockSetAlarmTime(clock, &alarm);

    SimulatedSeconds(clock, 2);

    TEST_ASSERT_TRUE(ClockIsAlarmRinging(clock));

    ClockFinishAlarm(clock);

    TEST_ASSERT_FALSE(ClockIsAlarmRinging(clock));
}

void test_set_up_and_alarm_rings_again_after_one_day(void) {
    clock_time_t start_time = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {0, 0}}};

    clock_time_t alarm_time = {
        .time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {0, 1}} // Suena al segundo 60
    };

    TEST_ASSERT_TRUE(ClockSetTime(clock, &start_time));
    TEST_ASSERT_TRUE(ClockSetAlarmTime(clock, &alarm_time));

    // Avanzar hasta el momento de la alarma
    SimulatedSeconds(clock, 10);
    TEST_ASSERT_TRUE(ClockIsAlarmRinging(clock));
    // Apagar alarma
    ClockFinishAlarm(clock);
    TEST_ASSERT_FALSE(ClockIsAlarmRinging(clock));
    // Avanzar un día
    TEST_ASSERT_TRUE(ClockSetTime(clock, &start_time));
    SimulatedSeconds(clock, 10);
    TEST_ASSERT_TRUE(ClockIsAlarmRinging(clock));
}

void test_set_up_and_set_alarm_time_and_change_states(void) {
    clock_time_t start_time = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {0, 0}}};

    clock_time_t alarm_time = {
        .time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {0, 1}} // Suena al segundo 60
    };

    TEST_ASSERT_TRUE(ClockSetTime(clock, &start_time));
    TEST_ASSERT_TRUE(ClockSetAlarmTime(clock, &alarm_time));

    TEST_ASSERT_FALSE(ClockSetAlarmState(clock, DISABLE));

    // Avanzar hasta el momento de la alarma
    SimulatedSeconds(clock, 10);
    TEST_ASSERT_FALSE(ClockIsAlarmRinging(clock));

    TEST_ASSERT_TRUE(ClockSetTime(clock, &start_time));
    TEST_ASSERT_TRUE(ClockSetAlarmState(clock, ENABLE));
    SimulatedSeconds(clock, 10);
    TEST_ASSERT_TRUE(ClockIsAlarmRinging(clock));
}

void test_set_up_and_try_to_set_alarm_with_invalid_current_time(void) {
    clock_time_t time = {.time = {.hours = {2, 2}, .minutes = {0, 0}, .seconds = {0, 0}}};
    TEST_ASSERT_FALSE(ClockSetAlarmTime(clock, &time));
    TEST_ASSERT_FALSE(ClockIsAlarmEnabled(clock));
}

void test_null_safe_clock_operations(void) {
    clock_time_t time = {.time = {.hours = {2, 2}, .minutes = {0, 0}, .seconds = {0, 0}}};

    TEST_ASSERT_FALSE(ClockSetTime(NULL, &time));
    TEST_ASSERT_FALSE(ClockGetTime(NULL, &time));
    TEST_ASSERT_FALSE(ClockSetAlarmTime(NULL, &time));
    TEST_ASSERT_FALSE(ClockGetAlarmTime(NULL, &time));
    TEST_ASSERT_FALSE(ClockSetAlarmState(NULL, ENABLE));
    TEST_ASSERT_FALSE(ClockIsAlarmEnabled(NULL));
    TEST_ASSERT_FALSE(ClockIsAlarmRinging(NULL));
}

void test_set_up_alarm_disabled_does_not_ring_even_when_time_matches(void) {
    clock_time_t now = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {0, 0}}};
    clock_time_t alarm = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {2, 0}}};

    ClockSetTime(clock, &now);
    ClockSetAlarmTime(clock, &alarm);
    ClockSetAlarmState(clock, DISABLE);
    SimulatedSeconds(clock, 2);

    TEST_ASSERT_FALSE(ClockIsAlarmRinging(clock));
}

void test_set_up_snooze_does_nothing_if_alarm_not_ringing(void) {
    clock_time_t now = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {0, 0}}};
    clock_time_t alarm = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {1, 0}}};

    ClockSetTime(clock, &now);
    ClockSetAlarmTime(clock, &alarm);

    // Se trata de atrazar la  alarma sin que suene
    ClockSnoozeAlarm(clock, 5);
    SimulatedSeconds(clock, 1); // avanza hasta que debería sonar

    TEST_ASSERT_TRUE(ClockIsAlarmRinging(clock)); // igual suena en el horario original
}

void test_set_up_changing_current_time_cancels_upcoming_alarm(void) {
    clock_time_t initial = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {0, 0}}};
    clock_time_t alarm = {.time = {.hours = {0, 0}, .minutes = {0, 0}, .seconds = {9, 0}}};

    ClockSetTime(clock, &initial);
    ClockSetAlarmTime(clock, &alarm);

    SimulatedSeconds(clock, 8); // un segundo antes de sonar

    clock_time_t skip = {.time = {.hours = {1, 0}, .minutes = {0, 0}, .seconds = {0, 0}}};
    ClockSetTime(clock, &skip);

    SimulatedSeconds(clock, 1); // debería haber sonado pero la hora cambió

    TEST_ASSERT_FALSE(ClockIsAlarmRinging(clock));
}

/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */
