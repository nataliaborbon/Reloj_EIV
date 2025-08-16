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

/** @file control.c
 ** @brief Implementación de la lógica de control del reloj
 **/

/* === Headers files inclusions ==================================================================================== */
#include "control.h"
#include "globals.h"
#include "digital.h"
#include "bsp.h"
#include "clock.h"

/* === Private macros definitions ================================================================================= */

#define LIMIT_MINUTES {9, 5} /**< Límite superior para los minutos en BCD (59) */
#define LIMIT_HOURS   {3, 2} /**< Límite superior para las horas en BCD (23) */

/* === Private data type declarations ============================================================================== */

/* === Private function declarations ============================================================================== */

/**
 * @brief Incrementa un valor BCD de dos dígitos.
 * @param value Arreglo BCD de dos dígitos a incrementar (unidades y decenas).
 * @param max   Límite máximo en BCD (unidades y decenas).
 *
 * Si se supera el valor máximo, el valor se reinicia a 00.
 */
void IncrementBCD(uint8_t value[2], const uint8_t max[2]);

/**
 * @brief Decrementa un valor BCD de dos dígitos.
 * @param value Arreglo BCD de dos dígitos a decrementar (unidades y decenas).
 * @param limit Límite superior en BCD al que volver si se decrementa desde 00.
 *
 * Por ejemplo, decrementa minutos de 00 a 59 o horas de 00 a 23.
 */
void DecrementBCD(uint8_t value[2], const uint8_t limit[2]);

/* === Private variable definitions ================================================================================ */

/* === Public variable definitions ================================================================================= */

/* === Private function definitions ================================================================================ */

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
 * @brief Decrementa un valor BCD de dos dígitos.
 *        Si llega a 00, se reinicia al límite.
 *
 * @param value Valor BCD a decrementar.
 * @param limit Límite superior en BCD.
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

/* === Public function definitions ================================================================================ */

/**
 * @brief Cambia el modo actual del sistema y actualiza la visualización.
 *
 * @param value Nuevo modo del sistema.
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
        ScreenClearPoint(board->screen, 3);
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
 * @brief Tarea principal de control del reloj.
 *        Maneja la máquina de estados y la interacción con teclas.
 */
void ControlTask(void * params) {
    (void)params;

    static const uint8_t LIMIT_MIN[2] = LIMIT_MINUTES;
    static const uint8_t LIMIT_HR[2] = LIMIT_HOURS;

    while (1) {
        EventBits_t events = xEventGroupWaitBits(key_events,
                                                 EVENT_KEY_ACCEPT | EVENT_KEY_CANCEL | EVENT_KEY_SET_TIME |
                                                     EVENT_KEY_SET_ALARM | EVENT_KEY_INCREMENT | EVENT_KEY_DECREMENT,
                                                 pdTRUE,  // Limpiar los bits leídos
                                                 pdFALSE, // No esperar todos, con cualquiera alcanza
                                                 0        // No bloquear
        );

        // --- TECLA ACCEPT ---
        if (events & EVENT_KEY_ACCEPT) {
            inactivity_count = 0;

            if (xSemaphoreTake(hour_mutex, portMAX_DELAY) == pdTRUE) {
                if (ClockIsAlarmRinging(reloj)) {
                    ClockSnoozeAlarm(reloj, 5); // Protegido
                }
                xSemaphoreGive(hour_mutex);
            }

            switch (mode) {
            case ADJUSTING_CURRENT_MINUTES:
                ChangeMode(ADJUSTING_CURRENT_HOURS);
                break;
            case ADJUSTING_CURRENT_HOURS:
                if (xSemaphoreTake(hour_mutex, portMAX_DELAY) == pdTRUE) {
                    ClockSetTime(reloj, &adjusting); // Protegido
                    xSemaphoreGive(hour_mutex);
                }
                ChangeMode(SHOWING_TIME);
                break;
            case ADJUSTING_ALARM_MINUTES:
                ChangeMode(ADJUSTING_ALARM_HOURS);
                break;
            case ADJUSTING_ALARM_HOURS:
                if (xSemaphoreTake(hour_mutex, portMAX_DELAY) == pdTRUE) {
                    ClockSetAlarmTime(reloj, &adjusting); // Protegido
                    ClockGetAlarmTime(reloj, &alarm);
                    xSemaphoreGive(hour_mutex);
                }
                if (ClockIsCurrentTimeValid(reloj)) {
                    ChangeMode(SHOWING_TIME);
                } else {
                    ChangeMode(UNCONFIGURED);
                }
                break;
            default:
                break;
            }
        }

        // --- TECLA CANCEL ---
        if (events & EVENT_KEY_CANCEL) {
            inactivity_count = 0;

            if (xSemaphoreTake(hour_mutex, portMAX_DELAY) == pdTRUE) {
                if (ClockIsAlarmRinging(reloj)) {
                    ClockFinishAlarm(reloj); // Protegido
                }
                xSemaphoreGive(hour_mutex);
            }

            if (ClockIsCurrentTimeValid(reloj)) {
                ChangeMode(SHOWING_TIME);
            } else {
                ChangeMode(UNCONFIGURED);
            }
        }

        // --- TECLA SET TIME ---
        if (events & EVENT_KEY_SET_TIME) {
            inactivity_count = 0;
            ChangeMode(ADJUSTING_CURRENT_MINUTES);

            if (xSemaphoreTake(hour_mutex, portMAX_DELAY) == pdTRUE) {
                ClockGetTime(reloj, &hour); // Protegido
                adjusting = hour;
                adjusting.bcd[0] = 0;
                adjusting.bcd[1] = 0;
                xSemaphoreGive(hour_mutex);
            }
        }

        // --- TECLA SET ALARM ---
        if (events & EVENT_KEY_SET_ALARM) {
            inactivity_count = 0;
            ChangeMode(ADJUSTING_ALARM_MINUTES);

            if (xSemaphoreTake(hour_mutex, portMAX_DELAY) == pdTRUE) {
                adjusting = alarm; // Protegido
                adjusting.bcd[0] = 0;
                adjusting.bcd[1] = 0;
                xSemaphoreGive(hour_mutex);
            }
        }

        // --- TECLA INCREMENT ---
        if (events & EVENT_KEY_INCREMENT) {
            inactivity_count = 0;

            if (xSemaphoreTake(hour_mutex, portMAX_DELAY) == pdTRUE) {
                switch (mode) {
                case ADJUSTING_CURRENT_MINUTES:
                case ADJUSTING_ALARM_MINUTES:
                    IncrementBCD(adjusting.time.minutes, LIMIT_MIN);
                    break;
                case ADJUSTING_CURRENT_HOURS:
                case ADJUSTING_ALARM_HOURS:
                    IncrementBCD(adjusting.time.hours, LIMIT_HR);
                    break;
                default:
                    break;
                }
                xSemaphoreGive(hour_mutex);
            }
        }

        // --- TECLA DECREMENT ---
        if (events & EVENT_KEY_DECREMENT) {
            inactivity_count = 0;

            if (xSemaphoreTake(hour_mutex, portMAX_DELAY) == pdTRUE) {
                switch (mode) {
                case ADJUSTING_CURRENT_MINUTES:
                case ADJUSTING_ALARM_MINUTES:
                    DecrementBCD(adjusting.time.minutes, LIMIT_MIN);
                    break;
                case ADJUSTING_CURRENT_HOURS:
                case ADJUSTING_ALARM_HOURS:
                    DecrementBCD(adjusting.time.hours, LIMIT_HR);
                    break;
                default:
                    break;
                }
                xSemaphoreGive(hour_mutex);
            }
        }

        // --- INACTIVIDAD ---
        inactivity_count++;
        if (inactivity_count >= 30000) { // INACTIVITY
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

/* === End of documentation ======================================================================================== */
