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

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "chip.h"
#include <stdbool.h>
#include "digital.h"
#include "bsp.h"
#include "clock.h"
#include <stdio.h>
#include "globals.h"
#include "keyboard.h"
#include "control.h"

/* === Macros definitions ====================================================================== */

/* === Private function declarations =========================================================== */

/**
 * @brief Activa el indicador de la alarma.
 * En este caso, activa un LED conectado al hardware.
 */
void ActivarAlarma(void);

/**
 * @brief Desactiva el indicador de la alarma.
 * En este caso, apaga un LED conectado al hardware.
 */
void DesactivarAlarma(void);

/* === Public variable definitions ============================================================= */

static volatile uint32_t miliseconds = 0; /**< Contador global de milisegundos. */

/**
 * @brief Implementación del driver de alarma que define las funciones de activación y desactivación.
 */
const struct alarm_driver_s mi_alarm_driver = {
    .activate = ActivarAlarma,     /**< Función utilizada para activar la alarma. */
    .deactivate = DesactivarAlarma /**< Función utilizada para desactivar la alarma. */
};

/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */

void ActivarAlarma(void) {
    DigitalOutputActivate(board->led);
}

void DesactivarAlarma(void) {
    DigitalOutputDeactivate(board->led);
}

// void Blinking(void * parameters) {
//     while (true) {
//         // DigitalOutputToggle(board->led);
//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }

void DisplayTask(void * pvParameters) {
    TickType_t last = xTaskGetTickCount();

    while (1) {
        clock_time_t time_to_show;

        if (xSemaphoreTake(hour_mutex, portMAX_DELAY) == pdTRUE) {

            ClockGetTime(reloj, &hour);

            switch (mode) {
            case SHOWING_TIME:
                time_to_show = hour;
                break;
            case ADJUSTING_CURRENT_MINUTES:
                time_to_show = adjusting;
                break;
            case ADJUSTING_CURRENT_HOURS:
                time_to_show = adjusting;
                break;
            case ADJUSTING_ALARM_MINUTES:
                time_to_show = adjusting;
                break;
            case ADJUSTING_ALARM_HOURS:
                time_to_show = adjusting;
                break;
            default:
                time_to_show = hour;
            }

            xSemaphoreGive(hour_mutex);
        }

        ScreenWriteBCD(board->screen, time_to_show.bcd, 6);

        if (mode == SHOWING_TIME) {
            if (miliseconds < 500) {
                ScreenSetPoint(board->screen, 1);
            } else {
                ScreenClearPoint(board->screen, 1);
            }
        }
        ScreenRefresh(board->screen);

        vTaskDelayUntil(&last, pdMS_TO_TICKS(1));
    }
}

void ClockTickTask(void * pvParameters) {
    TickType_t last = xTaskGetTickCount();

    while (1) {
        if (xSemaphoreTake(hour_mutex, portMAX_DELAY) == pdTRUE) {

            ClockNewTick(reloj);

            xSemaphoreGive(hour_mutex);
        }

        miliseconds++;
        if (miliseconds >= 1000) {
            miliseconds = 0;
        }
        vTaskDelayUntil(&last, pdMS_TO_TICKS(1));
    }
}

int main(void) {
    SystemCoreClockUpdate();

    reloj = ClockCreate(1000, &mi_alarm_driver);
    board = BoardCreate();
    ChangeMode(0);

    // Inicialización de teclado y creación de sus tareas
    KeyboardInit(board);

    ScreenWriteBCD(board->screen, hour.bcd, 6);

    hour_mutex = xSemaphoreCreateMutex();

    // xTaskCreate(Blinking, "Prueba", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(DisplayTask, "Display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(ClockTickTask, "ClockTick", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
    xTaskCreate(ControlTask, "ClockControl", 512, NULL, tskIDLE_PRIORITY + 3, NULL);
    vTaskStartScheduler();

    while (true) {
    }
}
/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */
