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

#define CLOCK_TICKS_PER_SECOND 5

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

/* === Private function declarations =========================================================== */

static void SimulatedSeconds(clock_t clock, uint8_t seconds) {
    for (uint8_t i = 0; i < CLOCK_TICKS_PER_SECOND * seconds; i++) {
        ClockNewTick(clock);
    }
}

/* === Public variable definitions ============================================================= */

/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */

/* === Public function implementation ========================================================= */

void setUp(void) {
    clock = ClockCreate(CLOCK_TICKS_PER_SECOND);
}

void test_set_up_with_invalid_time(void) {
    clock_time_t current_time = {.bcd = {1, 2, 3, 4, 5, 6}};

    clock_t clock_local = ClockCreate(CLOCK_TICKS_PER_SECOND);
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

void test_clock_advance_one_second(void) {
    clock_time_t current_time = {0};

    ClockSetTime(clock, &(clock_time_t){0});
    SimulatedSeconds(clock, 1);
    ClockGetTime(clock, &current_time);
    TEST_ASSERT_TIME(0, 0, 0, 0, 0, 1, current_time);
}

void test_clock_advance_ten_seconds(void) {
    clock_time_t current_time = {0};

    ClockSetTime(clock, &(clock_time_t){0});
    SimulatedSeconds(clock, 10);
    ClockGetTime(clock, &current_time);
    TEST_ASSERT_TIME(0, 0, 0, 0, 1, 0, current_time);
}

/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */
