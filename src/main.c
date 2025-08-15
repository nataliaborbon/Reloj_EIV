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

/* === Macros definitions ====================================================================== */

#define HOLD_TIME_MS                                                                                                   \
    3000 /**< Tiempo en milisegundos que debe mantenerse presionado un botón para entrar al modo de ajuste. */
#define TOLERANCE 100 /**< Tolerancia en milisegundos para evitar repeticiones no deseadas de lectura de botones. */
#define INACTIVITY                                                                                                     \
    30000 /**< Tiempo en milisegundos sin interacción del usuario para volver al modo de visualización.              \
           */
#define KEY_TASK_PERIOD_MS             10

#define EVENT_KEY_ACCEPT               (1 << 0)
#define EVENT_KEY_CANCEL               (1 << 1)
#define EVENT_KEY_SET_TIME             (1 << 2)
#define EVENT_KEY_SET_ALARM            (1 << 3)
#define EVENT_KEY_INCREMENT            (1 << 4)
#define EVENT_KEY_DECREMENT            (1 << 5)
#define EVENT_KEY_ACCEPT_INACTIVITY    (1 << 6)
#define EVENT_KEY_CANCEL_INACTIVITY    (1 << 7)
#define EVENT_KEY_SET_TIME_INACTIVITY  (1 << 8)
#define EVENT_KEY_SET_ALARM_INACTIVITY (1 << 9)
#define EVENT_KEY_INCREMENT_INACTIVITY (1 << 10)
#define EVENT_KEY_DECREMENT_INACTIVITY (1 << 11)

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

static board_t board;                /**< Estructura que representa la placa de hardware. */
static clock_t reloj;                /**< Puntero al objeto reloj. */
static mode_t mode;                  /**< Modo actual de funcionamiento del sistema. */
static clock_time_t hour = {0};      /**< Variable auxiliar para almacenar la hora actual. */
static clock_time_t alarm = {0};     /**< Variable auxiliar para almacenar la hora de la alarma. */
static clock_time_t adjusting = {0}; /**< Variable para almacenar la hora en proceso de ajuste. */

EventGroupHandle_t key_events;

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

typedef struct {
    digital_input_t key;       // Tecla física
    uint8_t event_bit;         // Bit de evento
    uint32_t hold_ms;          // Tiempo para considerar "hold"
    uint32_t pressed_count;    // Contador de tiempo presionado
    uint32_t tolerance_count;  // Contador de tolerancia
    uint32_t inactivity_count; // Cuenta de ms sin actividad
    bool inactivity;           // Flag de inactividad
} key_task_params_t;

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

static void KeyTask(void * params) {
    key_task_params_t * key = (key_task_params_t *)params;

    while (true) {
        bool active = !DigitalInputGetIsActive(key->key); // activo LOW
        digital_states_t change = DigitalInputWasChanged(key->key);

        if (key->hold_ms == 0) {
            // Tecla instantánea: disparar evento solo al activarse
            if (change == DIGITAL_INPUT_WAS_ACTIVATED) {
                xEventGroupSetBits(key_events, key->event_bit);
            }
        } else {
            // Tecla con hold
            if (active) {
                // Aumentar tiempo presionado
                key->pressed_count += KEY_TASK_PERIOD_MS;
                key->tolerance_count = 0;

                if (key->pressed_count >= key->hold_ms) {
                    xEventGroupSetBits(key_events, key->event_bit);
                }
            } else {
                // Tecla liberada
                if (key->tolerance_count < TOLERANCE) {
                    // Pequeña tolerancia para rebotes: no reiniciar pressed_count
                    key->pressed_count += KEY_TASK_PERIOD_MS;
                    key->tolerance_count += KEY_TASK_PERIOD_MS;
                } else {
                    key->pressed_count = 0;
                    key->tolerance_count = 0;
                }
            }
        }

        // --- Manejo de inactividad separado ---
        if (!active) {
            if (!key->inactivity) { // solo contar si aún no se disparó
                key->inactivity_count += KEY_TASK_PERIOD_MS;
                if (key->inactivity_count >= INACTIVITY) {
                    key->inactivity = true;
                    DigitalOutputActivate(board->led);
                    key->inactivity_count = 0;
                    xEventGroupSetBits(key_events, key->event_bit << 6); // corrimiento de 6 bits
                }
            }
        } else {
            key->inactivity = false;
            key->inactivity_count = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(KEY_TASK_PERIOD_MS));
    }
}

static void ControlTask(void * params) {
    (void)params;

    while (1) {
        // Espera a que ocurra algún evento de tecla
        EventBits_t events = xEventGroupWaitBits(key_events,
                                                 EVENT_KEY_ACCEPT | EVENT_KEY_CANCEL | EVENT_KEY_SET_TIME |
                                                     EVENT_KEY_SET_ALARM | EVENT_KEY_INCREMENT | EVENT_KEY_DECREMENT |
                                                     EVENT_KEY_ACCEPT_INACTIVITY | EVENT_KEY_CANCEL_INACTIVITY |
                                                     EVENT_KEY_SET_TIME_INACTIVITY | EVENT_KEY_SET_ALARM_INACTIVITY |
                                                     EVENT_KEY_INCREMENT_INACTIVITY | EVENT_KEY_DECREMENT_INACTIVITY,
                                                 pdTRUE,  // Limpiar los bits leídos
                                                 pdFALSE, // No esperar todos, con cualquiera alcanza
                                                 0        // No bloquear, retorna inmediatamente
        );

        // TECLA ACCEPT
        if (events & EVENT_KEY_ACCEPT) {
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

        // // TECLA CANCEL
        if (events & EVENT_KEY_CANCEL) {

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
            ChangeMode(ADJUSTING_CURRENT_MINUTES);
            ClockGetTime(reloj, &hour);
            adjusting = hour;
            adjusting.bcd[0] = 0;
            adjusting.bcd[1] = 0;
        }

        // TECLA SET ALARM
        if (events & EVENT_KEY_SET_ALARM) {
            ChangeMode(ADJUSTING_ALARM_MINUTES);
            adjusting = alarm;
            adjusting.bcd[0] = 0;
            adjusting.bcd[1] = 0;
        }

        // TECLA INCREMENT
        if (events & EVENT_KEY_INCREMENT) {
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
        if (events & EVENT_KEY_ACCEPT_INACTIVITY) {
            DigitalOutputDeactivate(board->led);
            if ((mode == ADJUSTING_CURRENT_HOURS || mode == ADJUSTING_CURRENT_MINUTES ||
                 mode == ADJUSTING_ALARM_HOURS || mode == ADJUSTING_ALARM_MINUTES)) {
                if (ClockIsCurrentTimeValid(reloj)) {
                    ChangeMode(SHOWING_TIME);
                } else {
                    ChangeMode(UNCONFIGURED);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
int main(void) {
    // SisTick_Init(1000);
    SystemCoreClockUpdate();

    reloj = ClockCreate(1000, &mi_alarm_driver);
    board = BoardCreate();

    static key_task_params_t keys[6] = {0};

    // TECLA ACCEPT
    keys[0].key = board->accept;
    keys[0].event_bit = EVENT_KEY_ACCEPT;
    keys[0].hold_ms = 0;
    keys[0].pressed_count = 0;
    keys[0].tolerance_count = 0;
    keys[0].inactivity_count = 0;
    keys[0].inactivity = false;

    // TECLA CANCEL
    keys[1].key = board->cancel;
    keys[1].event_bit = EVENT_KEY_CANCEL;
    keys[1].hold_ms = 0;
    keys[1].pressed_count = 0;
    keys[1].tolerance_count = 0;
    keys[1].inactivity_count = 0;
    keys[1].inactivity = false;

    // TECLA SET TIME
    keys[2].key = board->set_time;
    keys[2].event_bit = EVENT_KEY_SET_TIME;
    keys[2].hold_ms = HOLD_TIME_MS; // o 0 si querés probar
    keys[2].pressed_count = 0;
    keys[2].tolerance_count = 0;
    keys[2].inactivity_count = 0;
    keys[2].inactivity = false;

    // TECLA SET ALARM
    keys[3].key = board->set_alarm;
    keys[3].event_bit = EVENT_KEY_SET_ALARM;
    keys[3].hold_ms = HOLD_TIME_MS; // o 0 si querés probar
    keys[3].pressed_count = 0;
    keys[3].tolerance_count = 0;
    keys[3].inactivity_count = 0;
    keys[3].inactivity = false;

    // TECLA INCREMENT
    keys[4].key = board->increment;
    keys[4].event_bit = EVENT_KEY_INCREMENT;
    keys[4].hold_ms = 0;
    keys[4].pressed_count = 0;
    keys[4].tolerance_count = 0;
    keys[4].inactivity_count = 0;
    keys[4].inactivity = false;

    // TECLA DECREMENT
    keys[5].key = board->decrement;
    keys[5].event_bit = EVENT_KEY_DECREMENT;
    keys[5].hold_ms = 0;
    keys[5].pressed_count = 0;
    keys[5].tolerance_count = 0;
    keys[5].inactivity_count = 0;
    keys[5].inactivity = false;

    /* Configuramos la hora a 12:34:00 usando la variable global 'hour' */
    // hour.time.hours[1] = 1;   // decena de horas
    // hour.time.hours[0] = 2;   // unidad de horas
    // hour.time.minutes[1] = 3; // decena de minutos
    // hour.time.minutes[0] = 4; // unidad de minutos
    // hour.time.seconds[1] = 5; // decena de segundos
    // hour.time.seconds[0] = 0; // unidad de segundos

    key_events = xEventGroupCreate();

    /* Seteamos el reloj */
    // ClockSetTime(reloj, &hour);

    ChangeMode(0);
    ScreenWriteBCD(board->screen, hour.bcd, 6);

    xTaskCreate(Blinking, "Prueba", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(DisplayTask, "Display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(ClockTickTask, "ClockTick", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
    xTaskCreate(KeyTask, "KeyAccept", 128, &keys[0], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(KeyTask, "KeyCancel", 128, &keys[1], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(KeyTask, "KeySetTime", 128, &keys[2], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(KeyTask, "KeySetAlarm", 128, &keys[3], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(KeyTask, "KeyIncrement", 128, &keys[4], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(KeyTask, "KeyDecrement", 128, &keys[5], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(ControlTask, "ClockControl", 512, NULL, tskIDLE_PRIORITY + 3, NULL);
    vTaskStartScheduler();

    while (true) {
    }
}
/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */
