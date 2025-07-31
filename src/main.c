/* Copyright 2022, Laboratorio de Microprocesadores
 * Facultad de Ciencias Exactas y Tecnología
 * Universidad Nacional de Tucuman
 * http://www.microprocesadores.unt.edu.ar/
 * Copyright 2022, Esteban Volentini <evolentini@herrera.unt.edu.ar>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** \brief Simple sample of use LPC HAL gpio functions
 **
 ** \addtogroup samples Sample projects
 ** \brief Sample projects to use as a starting point
 ** @{ */

/* === Headers files inclusions =============================================================== */

#include "chip.h"
#include <stdbool.h>
#include "digital.h"
#include "bsp.h"
#include "clock.h"

/* === Macros definitions ====================================================================== */

/* === Private data type declarations ========================================================== */

/* === Private variable declarations =========================================================== */

/* === Private function declarations =========================================================== */

void ActivarAlarma(void);

void DesactivarAlarma(void);

/* === Public variable definitions ============================================================= */

static board_t board;

static clock_t reloj;

const struct alarm_driver_s mi_alarm_driver = {
    .activate = ActivarAlarma,
    .deactivate = DesactivarAlarma,
};

/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */

void ActivarAlarma(void) {
}

void DesactivarAlarma(void) {
}

/* === Public function implementation ========================================================= */

/*
int main(void) {

    SisTick_Init(1);

    reloj = ClockCreate(1000, &mi_alarm_driver);
    board = BoardCreate();

    clock_time_t value;

    for (int index = 0; index < 100; index++) {
        for (int delay = 0; delay < 25000; delay++) {
            __asm("NOP");
        }
    }

    ClockGetTime(reloj, &value);
    __asm volatile("cpsid i");
    ScreenWriteBCD(board->screen, value.bcd, 6);
    __asm volatile("cpsid i");
}
*/
void SysTick_Handler(void) {
    ScreenRefresh(board->screen);
    ClockNewTick(reloj);
}

int main(void) {
    SisTick_Init(1000);
    reloj = ClockCreate(1000, &mi_alarm_driver);
    board = BoardCreate();

    clock_time_t hora_inicial = {.bcd = {6, 3, 7, 5, 9, 1}};
    ClockSetTime(reloj, &hora_inicial);

    while (1) {
        clock_time_t hora_actual;
        ClockGetTime(reloj, &hora_actual);
        ScreenWriteBCD(board->screen, hora_actual.bcd, 6);
    }
}
/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */
