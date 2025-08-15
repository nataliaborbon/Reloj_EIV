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

/** @file keyboard.c
 ** @brief Implementación del manejo de teclas y eventos de usuario para el reloj.
 **/

/* === Headers files inclusions ==================================================================================== */

#include "keyboard.h"
#include "globals.h"
#include "digital.h"
#include "FreeRTOS.h"
#include "task.h"

/* === Private variable definitions ================================================================================ */

static key_task_params_t keys[6] = {0};

/* === Public variable definitions ================================================================================= */

/* === Public function implementation ============================================================================== */

void KeyboardInit(board_t board) {
    key_events = xEventGroupCreate();

    // TECLA ACCEPT
    keys[0].key = board->accept;
    keys[0].event_bit = EVENT_KEY_ACCEPT;
    keys[0].hold_ms = 0;
    keys[0].pressed_count = 0;
    keys[0].tolerance_count = 0;

    // TECLA CANCEL
    keys[1].key = board->cancel;
    keys[1].event_bit = EVENT_KEY_CANCEL;
    keys[1].hold_ms = 0;
    keys[1].pressed_count = 0;
    keys[1].tolerance_count = 0;

    // TECLA SET TIME
    keys[2].key = board->set_time;
    keys[2].event_bit = EVENT_KEY_SET_TIME;
    keys[2].hold_ms = HOLD_TIME_MS;
    keys[2].pressed_count = 0;
    keys[2].tolerance_count = 0;

    // TECLA SET ALARM
    keys[3].key = board->set_alarm;
    keys[3].event_bit = EVENT_KEY_SET_ALARM;
    keys[3].hold_ms = HOLD_TIME_MS;
    keys[3].pressed_count = 0;
    keys[3].tolerance_count = 0;

    // TECLA INCREMENT
    keys[4].key = board->increment;
    keys[4].event_bit = EVENT_KEY_INCREMENT;
    keys[4].hold_ms = 0;
    keys[4].pressed_count = 0;
    keys[4].tolerance_count = 0;

    // TECLA DECREMENT
    keys[5].key = board->decrement;
    keys[5].event_bit = EVENT_KEY_DECREMENT;
    keys[5].hold_ms = 0;
    keys[5].pressed_count = 0;
    keys[5].tolerance_count = 0;

    // Crear tareas explícitamente, sin usar bucles
    xTaskCreate(KeyTask, "KeyAccept", 128, &keys[0], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(KeyTask, "KeyCancel", 128, &keys[1], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(KeyTask, "KeySetTime", 128, &keys[2], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(KeyTask, "KeySetAlarm", 128, &keys[3], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(KeyTask, "KeyIncrement", 128, &keys[4], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(KeyTask, "KeyDecrement", 128, &keys[5], tskIDLE_PRIORITY + 1, NULL);
}

void KeyTask(void * params) {
    key_task_params_t * key = (key_task_params_t *)params;

    while (true) {
        bool active = !DigitalInputGetIsActive(key->key);
        digital_states_t change = DigitalInputWasChanged(key->key);

        if (key->hold_ms == 0) {
            // Tecla instantánea
            if (change == DIGITAL_INPUT_WAS_ACTIVATED) {
                xEventGroupSetBits(key_events, key->event_bit);
            }
        } else {
            // Tecla con hold
            if (active) {
                key->pressed_count += KEY_TASK_PERIOD_MS;
                key->tolerance_count = 0;

                if (key->pressed_count >= key->hold_ms) {
                    xEventGroupSetBits(key_events, key->event_bit);
                }
            } else {
                if (key->tolerance_count < TOLERANCE) {
                    key->pressed_count += KEY_TASK_PERIOD_MS;
                    key->tolerance_count += KEY_TASK_PERIOD_MS;
                } else {
                    key->pressed_count = 0;
                    key->tolerance_count = 0;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(KEY_TASK_PERIOD_MS));
    }
}

/* === End of documentation ======================================================================================== */
