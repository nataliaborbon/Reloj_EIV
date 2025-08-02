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
#include <stdio.h>

/* === Macros definitions ====================================================================== */

#define HOLD_TIME_MS                                                                                                   \
    3000 /**< Tiempo en milisegundos que debe mantenerse presionado un botón para entrar al modo de ajuste. */
#define TOLERANCE 100 /**< Tolerancia en milisegundos para evitar repeticiones no deseadas de lectura de botones. */
#define INACTIVITY                                                                                                     \
    30000 /**< Tiempo en milisegundos sin interacción del usuario para volver al modo de visualización. */

/**
 * @brief Modos de operación del reloj.
 */
typedef enum {
    UNCONFIGURED,              /**< Estado inicial sin configuración. */
    SHOWING_TIME,              /**< Mostrando la hora actual. */
    ADJUSTING_CURRENT_MINUTES, /**< Modo de ajuste de los minutos actuales. */
    ADJUSTING_CURRENT_HOURS,   /**< Modo de ajuste de las horas actuales. */
    ADJUSTING_ALARM_MINUTES,   /**< Modo de ajuste de los minutos de la alarma. */
    ADJUSTING_ALARM_HOURS,     /**< Modo de ajuste de las horas de la alarma. */
} mode_t;

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

static board_t board;            /**< Estructura que representa la placa de hardware. */
static clock_t reloj;            /**< Puntero al objeto reloj. */
static mode_t mode;              /**< Modo actual de funcionamiento del sistema. */
static clock_time_t hour = {0};  /**< Variable auxiliar para almacenar la hora actual. */
static clock_time_t alarm = {0}; /**< Variable auxiliar para almacenar la hora de la alarma. */
clock_time_t adjusting = {0};    /**< Variable para almacenar la hora en proceso de ajuste. */

static volatile uint32_t miliseconds = 0;           /**< Contador global de milisegundos. */
static volatile uint32_t key_set_time_duration = 0; /**< Tiempo que se mantuvo presionado el botón de ajuste de hora. */
static volatile uint32_t key_set_alarm_duration =
    0; /**< Tiempo que se mantuvo presionado el botón de ajuste de alarma. */
static volatile uint32_t key_set_time_tolerance = 0;  /**< Tolerancia para el botón de ajuste de hora. */
static volatile uint32_t key_set_alarm_tolerance = 0; /**< Tolerancia para el botón de ajuste de alarma. */
static volatile uint32_t inactivity_count =
    0; /**< Contador de inactividad para detectar falta de interacción del usuario. */

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

/* === Public function implementation ========================================================= */

void SysTick_Handler(void) {
    ScreenRefresh(board->screen);
    ClockNewTick(reloj);
    miliseconds++;
    if (miliseconds == 1000) {
        miliseconds = 0;
    }
    if (mode == SHOWING_TIME) {
        ClockGetTime(reloj, &hour);
        ScreenWriteBCD(board->screen, hour.bcd, 6);
        if (miliseconds < 500) {
            ScreenSetPoint(board->screen, 1);
        } else {
            ScreenClearPoint(board->screen, 1);
        }
    }

    if (!DigitalInputGetIsActive(board->set_time)) {
        key_set_time_duration++;
        key_set_time_tolerance = 0;
    } else {
        if (key_set_time_tolerance < TOLERANCE) {
            key_set_time_duration++;
            key_set_time_tolerance++;
        } else {
            key_set_time_duration = 0;
            key_set_time_tolerance = 0;
        }
    }

    if (!DigitalInputGetIsActive(board->set_alarm)) {
        key_set_alarm_duration++;
        key_set_alarm_tolerance = 0;
    } else {
        if (key_set_alarm_tolerance < TOLERANCE) {
            key_set_alarm_duration++;
            key_set_alarm_tolerance++;
        } else {
            key_set_alarm_duration = 0;
            key_set_alarm_tolerance = 0;
        }
    }

    if (DigitalInputGetIsActive(board->accept) && DigitalInputGetIsActive(board->increment) &&
        DigitalInputGetIsActive(board->decrement) && DigitalInputGetIsActive(board->set_alarm) &&
        DigitalInputGetIsActive(board->set_time)) {
        inactivity_count++;
    } else {
        inactivity_count = 0;
    }
}

int main(void) {
    SisTick_Init(1000);
    reloj = ClockCreate(1000, &mi_alarm_driver);
    board = BoardCreate();

    ChangeMode(UNCONFIGURED);

    while (1) {

        if (DigitalInputWasActivated(board->accept)) {
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

        if (DigitalInputWasActivated(board->cancel)) {
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

        if (key_set_time_duration == HOLD_TIME_MS) {
            ChangeMode(ADJUSTING_CURRENT_MINUTES);
            ClockGetTime(reloj, &hour);
            adjusting = hour;
            adjusting.bcd[0] = 0;
            adjusting.bcd[1] = 0;
            ScreenWriteBCD(board->screen, hour.bcd, 6);
        }

        if (key_set_alarm_duration == HOLD_TIME_MS) {
            ChangeMode(ADJUSTING_ALARM_MINUTES);
            adjusting = alarm;
            adjusting.bcd[0] = 0;
            adjusting.bcd[1] = 0;
            ScreenWriteBCD(board->screen, alarm.bcd, 6);
        }

        if (DigitalInputWasActivated(board->increment)) {
            if (mode == ADJUSTING_CURRENT_MINUTES) {
                IncrementBCD(adjusting.time.minutes, LIMIT_MINUTES);
            } else if (mode == ADJUSTING_CURRENT_HOURS) {
                IncrementBCD(adjusting.time.hours, LIMIT_HOURS);
            }

            if (mode == ADJUSTING_ALARM_MINUTES) {
                IncrementBCD(adjusting.time.minutes, LIMIT_MINUTES);
            } else if (mode == ADJUSTING_ALARM_HOURS) {
                IncrementBCD(adjusting.time.hours, LIMIT_HOURS);
            }

            ScreenWriteBCD(board->screen, adjusting.bcd, 6);
        }

        if (DigitalInputWasActivated(board->decrement)) {
            if (mode == ADJUSTING_CURRENT_MINUTES) {
                DecrementBCD(adjusting.time.minutes, LIMIT_MINUTES);
            } else if (mode == ADJUSTING_CURRENT_HOURS) {
                DecrementBCD(adjusting.time.hours, LIMIT_HOURS);
            }

            if (mode == ADJUSTING_ALARM_MINUTES) {
                DecrementBCD(adjusting.time.minutes, LIMIT_MINUTES);
            } else if (mode == ADJUSTING_ALARM_HOURS) {
                DecrementBCD(adjusting.time.hours, LIMIT_HOURS);
            }

            ScreenWriteBCD(board->screen, adjusting.bcd, 6);
        }

        if ((mode == ADJUSTING_CURRENT_HOURS || mode == ADJUSTING_CURRENT_MINUTES || mode == ADJUSTING_ALARM_HOURS ||
             mode == ADJUSTING_ALARM_MINUTES) &&
            (inactivity_count == INACTIVITY)) {
            inactivity_count = 0;
            if (ClockIsCurrentTimeValid(reloj)) {
                ChangeMode(SHOWING_TIME);
            } else {
                ChangeMode(UNCONFIGURED);
            }
        }
    }
}
/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */
