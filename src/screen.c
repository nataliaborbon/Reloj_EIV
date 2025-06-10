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

/** @file screen.c
 ** @brief Implementación del módulo para la gestión de una pantalla multiplexada de 7 segmentos.
 **/

/* === Headers files inclusions ==================================================================================== */

#include "screen.h"
#include <stdint.h>
#include "chip.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "poncho.h"

/* === Macros definitions ========================================================================================== */

#ifndef SCREEN_MAX_DIGITS
#define SCREEN_MAX_DIGITS 8
#endif

/* === Private data type declarations ============================================================================== */

/**
 * @brief Estructura que representa el estado de una pantalla multiplexada de 7 segmentos.
 */
struct screen_s {
    uint8_t digits;                      /**< Cantidad de dígitos manejados por la pantalla */
    uint8_t current_digit;               /**< Índice del dígito actualmente activo */
    uint8_t flashing_from;               /**< Primer dígito que debe parpadear */
    uint8_t flashing_to;                 /**< Último dígito que debe parpadear */
    uint8_t digits_flash_freq;           /**< Frecuencia de parpadeo de dígitos */
    uint8_t digits_flash_count;          /**< Contador para parpadeo de dígitos */
    uint8_t points_flash_freq;           /**< Frecuencia de parpadeo de puntos decimales */
    uint8_t points_flash_count;          /**< Contador para parpadeo de puntos decimales */
    screen_driver_t driver;              /**< Driver de funciones para manejar hardware */
    uint8_t value[SCREEN_MAX_DIGITS];    /**< Imagen en segmentos por cada dígito */
    bool point_on[SCREEN_MAX_DIGITS];    /**< Indica si el punto está encendido por dígito */
    bool point_flash[SCREEN_MAX_DIGITS]; /**< Indica si el punto debe parpadear por dígito */
};

/* === Private function declarations =============================================================================== */

/* === Private variable definitions ================================================================================ */

/**
 * @brief Imágenes (máscaras de segmentos) correspondientes a los dígitos decimales 0-9.
 */
static const uint8_t IMAGES[10] = {
    SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F,             // 0
    SEGMENT_B | SEGMENT_C,                                                             // 1
    SEGMENT_A | SEGMENT_B | SEGMENT_D | SEGMENT_E | SEGMENT_G,                         // 2
    SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_G,                         // 3
    SEGMENT_B | SEGMENT_C | SEGMENT_F | SEGMENT_G,                                     // 4
    SEGMENT_A | SEGMENT_C | SEGMENT_D | SEGMENT_F | SEGMENT_G,                         // 5
    SEGMENT_A | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F | SEGMENT_G,             // 6
    SEGMENT_A | SEGMENT_B | SEGMENT_C,                                                 // 7
    SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F | SEGMENT_G, // 8
    SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F,             // 9
};

/* === Public variable definitions ================================================================================= */

/* === Private function definitions ================================================================================ */

/* === Public function implementation ============================================================================== */

screen_t ScreenCreate(uint8_t digits, screen_driver_t driver) {
    screen_t self = malloc(sizeof(struct screen_s));
    if (digits > SCREEN_MAX_DIGITS) {
        digits = SCREEN_MAX_DIGITS;
    }
    if (self != NULL) {
        self->digits = digits;
        self->driver = driver;
        self->current_digit = 0;
        self->digits_flash_freq = 0;
        self->digits_flash_count = 0;
        self->points_flash_freq = 0;
        self->points_flash_count = 0;
        memset(self->point_on, 0, sizeof(self->point_on));
        memset(self->point_flash, 0, sizeof(self->point_flash));
    }
    return self;
}

void ScreenWriteBCD(screen_t self, uint8_t value[], uint8_t size) {
    memset(self->value, 0, sizeof(self->value));
    if (size > self->digits) {
        size = self->digits;
    }
    for (uint8_t i = 0; i < size; i++) {
        self->value[i] = IMAGES[value[i]];
    }
}

void ScreenRefresh(screen_t self) {
    uint8_t segments;

    self->driver->DigitsTurnOff();
    self->current_digit = (self->current_digit + 1) % self->digits;

    if (self->current_digit == 0) {
        if (self->digits_flash_freq > 0) {
            self->digits_flash_count = (self->digits_flash_count + 1) % self->digits_flash_freq;
        }
        if (self->points_flash_freq > 0) {
            self->points_flash_count = (self->points_flash_count + 1) % self->points_flash_freq;
        }
    }

    bool digits_visible = (self->digits_flash_count < (self->digits_flash_freq / 2));
    bool points_visible = (self->points_flash_count < (self->points_flash_freq / 2));

    segments = self->value[self->current_digit];

    if (self->digits_flash_freq && self->current_digit >= self->flashing_from &&
        self->current_digit <= self->flashing_to && !digits_visible) {
        segments = 0;
    }

    if (self->point_on[self->current_digit]) {
        if (!self->point_flash[self->current_digit] || points_visible) {
            segments |= SEGMENT_P;
        }
    }

    self->driver->SegmentsUpdate(segments);
    self->driver->DigitTurnOn(self->current_digit);
}

int DisplayFlashDigits(screen_t self, uint8_t from, uint8_t to, uint16_t divisor) {
    int result = 0;

    if ((from > to) || (from >= SCREEN_MAX_DIGITS) || (to >= SCREEN_MAX_DIGITS)) {
        result = -1;
    } else if (!self) {
        result = -1;
    } else {
        self->flashing_from = from;
        self->flashing_to = to;
        self->digits_flash_freq = 2 * divisor;
        self->digits_flash_count = 0;
    }
    return result;
}

void ScreenSetPoint(screen_t screen, uint8_t digit) {
    if (digit < screen->digits) {
        screen->point_on[digit] = true;
        screen->point_flash[digit] = false;
    }
}

void ScreenClearPoint(screen_t screen, uint8_t digit) {
    if (digit < screen->digits) {
        screen->point_on[digit] = false;
        screen->point_flash[digit] = false;
    }
}

void ScreenFlashPoint(screen_t screen, uint8_t digit, uint8_t divisor) {
    if (digit < screen->digits) {
        screen->point_on[digit] = true;
        screen->point_flash[digit] = true;
        screen->points_flash_freq = 2 * divisor;
    }
}

/* === End of documentation ========================================================================================*/
