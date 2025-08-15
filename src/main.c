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

#define INACTIVITY                                                                                                     \
    30000 /**< Tiempo en milisegundos sin interacción del usuario para volver al modo de visualización.              \
           */

#define EVENT_KEY_ACCEPT    (1 << 0)
#define EVENT_KEY_CANCEL    (1 << 1)
#define EVENT_KEY_SET_TIME  (1 << 2)
#define EVENT_KEY_SET_ALARM (1 << 3)
#define EVENT_KEY_INCREMENT (1 << 4)
#define EVENT_KEY_DECREMENT (1 << 5)

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

static clock_time_t hour = {0};      /**< Variable auxiliar para almacenar la hora actual. */
static clock_time_t alarm = {0};     /**< Variable auxiliar para almacenar la hora de la alarma. */
static clock_time_t adjusting = {0}; /**< Variable para almacenar la hora en proceso de ajuste. */

static volatile uint32_t miliseconds = 0; /**< Contador global de milisegundos. */

/**
 * @brief Implementación del driver de alarma que define las funciones de activación y desactivación.
 */
const struct alarm_driver_s mi_alarm_driver = {
    .activate = ActivarAlarma,     /**< Función utilizada para activar la alarma. */
    .deactivate = DesactivarAlarma /**< Función utilizada para desactivar la alarma. */
};

/* === Private variable definitions ============================================================ */

/**
 * @brief Límite superior para los minutos en formato BCD.
 * Representa 59 minutos (9 unidades, 5 decenas).
 */
static const uint8_t LIMIT_MINUTES[2] = {9, 5};

/**
 * @brief Límite superior para las horas en formato BCD.
 * Representa 23 horas (3 unidades, 2 decenas).
 */
static const uint8_t LIMIT_HOURS[2] = {3, 2};

/* === Private function implementation ========================================================= */

void ActivarAlarma(void) {
    DigitalOutputActivate(board->led);
}

void DesactivarAlarma(void) {
    DigitalOutputDeactivate(board->led);
}

/**
 * @brief Cambia el modo actual del sistema.
 * También actualiza el estado visual del display según el modo seleccionado.
 *
 * @param value Modo al que se desea cambiar.
 */
void ChangeMode(mode_t value) {
    mode = value;
    switch (mode) {
    case UNCONFIGURED:
        DisplayFlashDigits(board->screen, 0, 3, 100);
        ScreenClearPoint(board->screen, 0);
        ScreenFlashPoint(board->screen, 1, 100);
        ScreenClearPoint(board->screen, 2);
        ScreenClearPoint(board->screen, 3);
        break;

    case SHOWING_TIME:
        DisplayFlashDigits(board->screen, 0, 0, 0);
        ScreenClearPoint(board->screen, 0);
        ScreenClearPoint(board->screen, 1);
        ScreenClearPoint(board->screen, 2);
        if (ClockIsAlarmEnabled(reloj)) {
            ScreenSetPoint(board->screen, 3);
        } else {
            ScreenClearPoint(board->screen, 3);
        }
        break;

    case ADJUSTING_CURRENT_MINUTES:
        DisplayFlashDigits(board->screen, 2, 3, 100);
        ScreenClearPoint(board->screen, 0);
        ScreenClearPoint(board->screen, 1);
        ScreenClearPoint(board->screen, 2);
        ScreenClearPoint(board->screen, 3);
        break;

    case ADJUSTING_CURRENT_HOURS:
        DisplayFlashDigits(board->screen, 0, 1, 100);
        ScreenClearPoint(board->screen, 0);
        ScreenClearPoint(board->screen, 1);
        ScreenClearPoint(board->screen, 2);
        ScreenClearPoint(board->screen, 3);
        break;

    case ADJUSTING_ALARM_MINUTES:
        DisplayFlashDigits(board->screen, 2, 3, 100);
        ScreenFlashPoint(board->screen, 0, 100);
        ScreenFlashPoint(board->screen, 1, 100);
        ScreenFlashPoint(board->screen, 2, 100);
        ScreenFlashPoint(board->screen, 3, 100);
        break;

    case ADJUSTING_ALARM_HOURS:
        DisplayFlashDigits(board->screen, 0, 1, 100);
        ScreenFlashPoint(board->screen, 0, 100);
        ScreenFlashPoint(board->screen, 1, 100);
        ScreenFlashPoint(board->screen, 2, 100);
        ScreenFlashPoint(board->screen, 3, 100);
        break;

    default:
        break;
    }
}

/**
 * @brief Incrementa un valor BCD de dos dígitos.
 * Si se supera el valor máximo, se reinicia a 00.
 *
 * @param value Arreglo BCD de dos elementos a incrementar.
 * @param max   Valor máximo permitido (también en BCD).
 */
void IncrementBCD(uint8_t value[2], const uint8_t max[2]) {
    uint8_t current = value[1] * 10 + value[0];
    uint8_t maximum = max[1] * 10 + max[0];

    current++;
    if (current > maximum) {
        current = 0;
    }

    value[1] = current / 10;
    value[0] = current % 10;
}

/**
 * @brief Decrementa un valor BCD de dos dígitos (por ejemplo minutos u horas).
 * Si llega a 00, vuelve al valor límite especificado.
 *
 * @param value Valor BCD a decrementar.
 * @param limit Límite superior en BCD (por ejemplo, 23 horas o 59 minutos).
 */
void DecrementBCD(uint8_t value[2], const uint8_t limit[2]) {
    if (value[0] == 0) {
        value[0] = 9;
        if (value[1] == 0) {
            value[1] = limit[1];
            value[0] = limit[0];
        } else {
            value[1]--;
        }
    } else {
        value[0]--;
    }
}

void Blinking(void * parameters) {
    while (true) {
        // DigitalOutputToggle(board->led);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void DisplayTask(void * pvParameters) {
    TickType_t last = xTaskGetTickCount();

    while (1) {
        clock_time_t time_to_show;
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
        ClockNewTick(reloj);

        miliseconds++;
        if (miliseconds >= 1000) {
            miliseconds = 0;
        }

        vTaskDelayUntil(&last, pdMS_TO_TICKS(1));
    }
}

static void ControlTask(void * params) {
    (void)params;

    while (1) {
        EventBits_t events = xEventGroupWaitBits(key_events,
                                                 EVENT_KEY_ACCEPT | EVENT_KEY_CANCEL | EVENT_KEY_SET_TIME |
                                                     EVENT_KEY_SET_ALARM | EVENT_KEY_INCREMENT | EVENT_KEY_DECREMENT,
                                                 pdTRUE,  // Limpiar los bits leídos
                                                 pdFALSE, // No esperar todos, con cualquiera alcanza
                                                 0        // No bloquear, retorna inmediatamente
        );

        // TECLA ACCEPT
        if (events & EVENT_KEY_ACCEPT) {
            inactivity_count = 0;
            if (ClockIsAlarmRinging(reloj)) {
                ClockSnoozeAlarm(reloj, 5);
            }
            switch (mode) {
            case ADJUSTING_CURRENT_MINUTES:
                ChangeMode(ADJUSTING_CURRENT_HOURS);
                break;
            case ADJUSTING_CURRENT_HOURS:
                ClockSetTime(reloj, &adjusting);
                ChangeMode(SHOWING_TIME);
                break;
            case ADJUSTING_ALARM_MINUTES:
                ChangeMode(ADJUSTING_ALARM_HOURS);
                break;
            case ADJUSTING_ALARM_HOURS:
                ClockSetAlarmTime(reloj, &adjusting);
                ClockGetAlarmTime(reloj, &alarm);
                if (ClockIsCurrentTimeValid(reloj)) {
                    ChangeMode(SHOWING_TIME);
                } else {
                    ChangeMode(UNCONFIGURED);
                }
                break;
            case SHOWING_TIME:
                ClockSetAlarmState(reloj, ALARM_ENABLE);
                ChangeMode(SHOWING_TIME);
                break;
            default:
                break;
            }
        }

        // TECLA CANCEL
        if (events & EVENT_KEY_CANCEL) {
            inactivity_count = 0;
            if (ClockIsAlarmRinging(reloj)) {
                ClockFinishAlarm(reloj);
            }
            if (ClockIsCurrentTimeValid(reloj)) {
                if (mode == SHOWING_TIME) {
                    ClockSetAlarmState(reloj, ALARM_DISABLE);
                }
                ChangeMode(SHOWING_TIME);
            } else {
                ChangeMode(UNCONFIGURED);
            }
        }

        // TECLA SET TIME
        if (events & EVENT_KEY_SET_TIME) {
            inactivity_count = 0;

            ChangeMode(ADJUSTING_CURRENT_MINUTES);
            ClockGetTime(reloj, &hour);
            adjusting = hour;
            adjusting.bcd[0] = 0;
            adjusting.bcd[1] = 0;
        }

        // TECLA SET ALARM
        if (events & EVENT_KEY_SET_ALARM) {
            inactivity_count = 0;

            ChangeMode(ADJUSTING_ALARM_MINUTES);
            adjusting = alarm;
            adjusting.bcd[0] = 0;
            adjusting.bcd[1] = 0;
        }

        // TECLA INCREMENT
        if (events & EVENT_KEY_INCREMENT) {
            inactivity_count = 0;

            switch (mode) {
            case ADJUSTING_CURRENT_MINUTES:
                IncrementBCD(adjusting.time.minutes, LIMIT_MINUTES);
                break;
            case ADJUSTING_ALARM_MINUTES:
                IncrementBCD(adjusting.time.minutes, LIMIT_MINUTES);
                break;
            case ADJUSTING_CURRENT_HOURS:
                IncrementBCD(adjusting.time.hours, LIMIT_HOURS);
                break;
            case ADJUSTING_ALARM_HOURS:
                IncrementBCD(adjusting.time.hours, LIMIT_HOURS);
                break;
            default:
                break;
            }
        }

        // TECLA DECREMENT
        if (events & EVENT_KEY_DECREMENT) {
            inactivity_count = 0;

            switch (mode) {
            case ADJUSTING_CURRENT_MINUTES:
                DecrementBCD(adjusting.time.minutes, LIMIT_MINUTES);
                break;
            case ADJUSTING_ALARM_MINUTES:
                DecrementBCD(adjusting.time.minutes, LIMIT_MINUTES);
                break;
            case ADJUSTING_CURRENT_HOURS:
                DecrementBCD(adjusting.time.hours, LIMIT_HOURS);
                break;
            case ADJUSTING_ALARM_HOURS:
                DecrementBCD(adjusting.time.hours, LIMIT_HOURS);
                break;
            default:
                break;
            }
        }

        // INACTIVIDAD
        inactivity_count++;
        if (inactivity_count >= INACTIVITY) {
            inactivity_count = 0;
            if ((mode == ADJUSTING_CURRENT_HOURS || mode == ADJUSTING_CURRENT_MINUTES ||
                 mode == ADJUSTING_ALARM_HOURS || mode == ADJUSTING_ALARM_MINUTES)) {
                if (ClockIsCurrentTimeValid(reloj)) {
                    ChangeMode(SHOWING_TIME);
                } else {
                    ChangeMode(UNCONFIGURED);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1));
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

    xTaskCreate(Blinking, "Prueba", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(DisplayTask, "Display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(ClockTickTask, "ClockTick", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
    xTaskCreate(ControlTask, "ClockControl", 512, NULL, tskIDLE_PRIORITY + 3, NULL);
    vTaskStartScheduler();

    while (true) {
    }
}
/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */
