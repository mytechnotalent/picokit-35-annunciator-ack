// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/picokit-35-annunciator-ack
// File:    monitor.c
// Desc:    Implements the latching annunciator state machine acknowledged
//          from the button or the infrared remote.
// Created: 2026

#include "picokit_35_annunciator_ack.h"
#include "monitor.h"
#include "radio.h"
#include "status_led.h"
#include "ir_remote.h"
#include "button.h"
#include "crypto_aead.h"
#include "crypto_kdf.h"
#include "envelope.h"
#include "field_secrets.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Module-ready flag.
 *
 * Set to true by monitor_init() once the peripherals are configured.
 * monitor_step() returns false while this flag is clear.
 */
static bool g_ready;

/**
 * @brief Latched annunciator flag, cleared only by an acknowledge.
 */
static bool g_latched;

/**
 * @brief Monotonic transmit sequence number.
 */
static uint16_t g_seq;

/**
 * @brief Absolute time in microseconds of the next authenticated transmit.
 */
static uint64_t g_next_tx_us;

/**
 * @brief Inbound radio line accumulator.
 */
static char g_rx_line[RADIO_LINE_BUF_LEN];

/**
 * @brief Number of bytes currently held in the inbound line accumulator.
 */
static size_t g_rx_len;

/**
 * @brief Derived XChaCha20-Poly1305 session key for telemetry.
 */
static uint8_t g_key[CRYPTO_AEAD_KEY_LEN];

/**
 * @brief True once the telemetry session key has been derived.
 */
static bool g_key_ready;

/**
 * @brief Configure the onboard heartbeat LED as a dark output.
 *
 * @param void No parameters.
 * @return void
 */
static void monitor_state_init_io(void) {
    gpio_init(PICOKIT_35_ANNUNCIATOR_ACK_LED_PIN);
    gpio_set_dir(PICOKIT_35_ANNUNCIATOR_ACK_LED_PIN, GPIO_OUT);
    gpio_put(PICOKIT_35_ANNUNCIATOR_ACK_LED_PIN, 0);
}

/**
 * @brief Reset the annunciator latch and transmit timing.
 *
 * @param void No parameters.
 * @return void
 */
static void monitor_state_init(void) {
    uint64_t now_us = time_us_64();
    g_latched = false;
    g_seq = 0u;
    g_next_tx_us = now_us + (uint64_t)PICOKIT_35_ANNUNCIATOR_ACK_TX_INTERVAL_MS * 1000u;
    g_ready = true;
}

/**
 * @brief Derive the telemetry session key from the field secret.
 *
 * LAB-ONLY: production must provision the session key through OTP rather
 * than deriving it from a committed passphrase and salt.
 *
 * @param void No parameters.
 * @return bool true when the session key was derived.
 */
static bool monitor_derive_key(void) {
    bool ok = crypto_kdf_argon2id((const uint8_t *)FIELD_SECRET_PASSPHRASE, strlen(FIELD_SECRET_PASSPHRASE), FIELD_SECRET_SALT, 16u, g_key);
    g_key_ready = ok;
    return ok;
}

/**
 * @brief Print the boot banner for the annunciator lesson.
 *
 * @param void No parameters.
 * @return void
 */
static void monitor_banner(void) {
    printf("=== PICOKIT-35 ANNUNCIATOR ACK // LATCHED LED + AUTHENTICATED HEARTBEAT ===\n");
}

/**
 * @brief Derive the field key and announce a ready monitor.
 *
 * @param void No parameters.
 * @return bool true when the field key was derived and installed.
 */
static bool monitor_finish(void) {
    bool ok = monitor_derive_key();
    if (ok) {
        monitor_banner();
    }
    return ok;
}

/**
 * @brief Blink the onboard heartbeat LED exactly once.
 *
 * @param void No parameters.
 * @return void
 */
static void monitor_heartbeat(void) {
    gpio_put(PICOKIT_35_ANNUNCIATOR_ACK_LED_PIN, 1);
    sleep_us(MONITOR_HEARTBEAT_BLINK_US);
    gpio_put(PICOKIT_35_ANNUNCIATOR_ACK_LED_PIN, 0);
    sleep_us(MONITOR_HEARTBEAT_BLINK_US);
}

/**
 * @brief Drive the annunciator lamp from the latched state.
 *
 * @param void No parameters.
 * @return void
 */
static void monitor_set_led(void) {
    uint8_t step = g_latched ? STATUS_LED_STEP_RED : STATUS_LED_STEP_GREEN;
    status_led_show_step(step);
}

/**
 * @brief Latch the annunciator from a plaintext gateway notice.
 *
 * @param payload Pointer to the NUL-terminated payload text.
 * @return void
 */
static void monitor_notice(const char *payload) {
    if ((payload != NULL) && (strcmp(payload, MONITOR_NOTICE) == 0)) {
        g_latched = true;
        monitor_set_led();
        printf("ALARM\n");
    }
}

/**
 * @brief Acknowledge the annunciator, clear the latch, and darken red.
 *
 * @param void No parameters.
 * @return void
 */
static void monitor_ack(void) {
    g_latched = false;
    monitor_set_led();
    printf("ACK\n");
}

/**
 * @brief Consume one acknowledge press from the push-button.
 *
 * @param void No parameters.
 * @return void
 */
static void monitor_poll_button(void) {
    if (button_consume_press()) {
        monitor_ack();
    }
}

/**
 * @brief Poll the infrared eye and acknowledge on the remote key.
 *
 * @param void No parameters.
 * @return void
 */
static void monitor_poll_ir(void) {
    ir_command_t cmd;
    if (ir_remote_poll(&cmd) && (cmd.command == MONITOR_KEY_ACK)) {
        monitor_ack();
    }
}

/**
 * @brief Poll the button and the infrared eye for an acknowledge.
 *
 * @param void No parameters.
 * @return void
 */
static void monitor_input_tick(void) {
    monitor_poll_button();
    monitor_poll_ir();
}

/**
 * @brief Format the heartbeat JSON body for the latched state.
 *
 * @param frame Pointer to the mutable frame output buffer.
 * @param frame_len Capacity of the frame output buffer in bytes.
 * @return size_t Number of JSON bytes written, or zero on overflow.
 */
static size_t monitor_build_frame(char *frame, size_t frame_len) {
    int written = snprintf(frame, frame_len, "{\"n\":%u,\"s\":%u,\"s\":%u}", (unsigned)PACKET_NODE_ID, (unsigned)g_seq, (unsigned)g_latched);
    return (written > 0 && (size_t)written < frame_len) ? (size_t)written : 0u;
}

/**
 * @brief Seal the current heartbeat body into a hex envelope.
 *
 * @param hex Pointer to the NUL-terminated hex output buffer.
 * @param hex_len Capacity of the hex output buffer in bytes.
 * @return bool true when the heartbeat was sealed and encoded.
 */
static bool monitor_seal_frame(char *hex, size_t hex_len) {
    char frame[PICOKIT_35_ANNUNCIATOR_ACK_FRAME_SIZE];
    uint8_t nonce[ENVELOPE_NONCE_LEN];
    uint8_t ad = (uint8_t)PACKET_NODE_ID;
    size_t frame_len = monitor_build_frame(frame, sizeof(frame));
    envelope_fill_nonce(nonce);
    return envelope_seal_hex(g_key, nonce, &ad, 1u, (const uint8_t *)frame, frame_len, hex, hex_len);
}

/**
 * @brief Build and transmit the authenticated heartbeat frame.
 *
 * @param void No parameters.
 * @return void
 */
static void monitor_transmit(void) {
    char hex[ENVELOPE_MAX_HEX_LEN];
    if (!g_key_ready) {
        return;
    }
    if (monitor_seal_frame(hex, sizeof(hex))) {
        radio_send_frame(PICOKIT_35_ANNUNCIATOR_ACK_UART, (const uint8_t *)hex, strlen(hex));
        g_seq += 1u;
    }
}

/**
 * @brief Transmit one heartbeat and schedule the next transmit.
 *
 * @param now_us Current monotonic time in microseconds.
 * @return void
 */
static void monitor_tx_tick(uint64_t now_us) {
    monitor_heartbeat();
    monitor_transmit();
    g_next_tx_us = now_us + (uint64_t)PICOKIT_35_ANNUNCIATOR_ACK_TX_INTERVAL_MS * 1000u;
}

/**
 * @brief Drain inbound radio lines and latch every valid notice.
 *
 * @param void No parameters.
 * @return void
 */
static void monitor_rx_tick(void) {
    radio_rcv_t rcv;
    while (radio_line_pump(PICOKIT_35_ANNUNCIATOR_ACK_UART, g_rx_line, &g_rx_len)) {
        if (radio_parse_rcv(g_rx_line, &rcv) == RADIO_RESULT_OK) {
            printf("RX from 0x%04X, %u bytes\n", (unsigned)rcv.sender, (unsigned)rcv.len);
            monitor_notice(rcv.payload);
        }
    }
}

/**
 * @brief Service the heartbeat transmit timer.
 *
 * @param now_us Current monotonic time in microseconds.
 * @return void
 */
static void monitor_service_timers(uint64_t now_us) {
    if (now_us >= g_next_tx_us) {
        monitor_tx_tick(now_us);
    }
}

bool monitor_init(void) {
    bool ok;
    ok = status_led_init() && radio_init(PICOKIT_35_ANNUNCIATOR_ACK_UART);
    ok = ok && ir_remote_init() && button_init();
    monitor_state_init_io();
    monitor_state_init();
    return ok && monitor_finish();
}

void monitor_deinit(void) {
    g_ready = false;
}

bool monitor_step(void) {
    uint64_t now_us;
    if (!g_ready) {
        return false;
    }
    now_us = time_us_64();
    monitor_service_timers(now_us);
    monitor_input_tick();
    monitor_rx_tick();
    return true;
}
