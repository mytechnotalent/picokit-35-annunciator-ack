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
// File:    monitor.h
// Desc:    Declares the latching annunciator state machine acknowledged from
//          the button or the infrared remote.
// Created: 2026

#ifndef MONITOR_H
#define MONITOR_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Onboard heartbeat LED on and off time in microseconds.
 */
#define MONITOR_HEARTBEAT_BLINK_US 50000u

/**
 * @brief Plaintext gateway notice that latches the annunciator.
 */
#define MONITOR_NOTICE "ALARM"

/**
 * @brief NEC command code that acknowledges and clears the annunciator.
 */
#define MONITOR_KEY_ACK 0x45u

/**
 * @brief Latched annunciator state value reported in the heartbeat.
 */
#define MONITOR_STATE_ON 1u

/**
 * @brief Cleared annunciator state value reported in the heartbeat.
 */
#define MONITOR_STATE_OFF 0u

/**
 * @brief Initialize the annunciator monitor state machine.
 *
 * Configures the red, yellow, and green annunciator lamps, the onboard
 * heartbeat LED, the push-button input, the VS1838B infrared receiver,
 * and the RYLR998 UART, derives the field key, and clears the latch.
 *
 * @param void No parameters.
 * @return bool true when all submodules initialized.
 */
bool monitor_init(void);

/**
 * @brief Clear the monitor-ready flag.
 *
 * Test and recovery hook that returns the state machine to the
 * uninitialized policy state.
 *
 * @param void No parameters.
 * @return void
 */
void monitor_deinit(void);

/**
 * @brief Execute one monitor state-machine tick.
 *
 * Latches the annunciator from a plaintext gateway notice, acknowledges
 * it from the button or an infrared key, transmits the authenticated
 * heartbeat on the telemetry interval, and pumps inbound +RCV lines.
 *
 * @param void No parameters.
 * @return bool true when the tick completed without a policy error.
 */
bool monitor_step(void);

#endif // MONITOR_H
