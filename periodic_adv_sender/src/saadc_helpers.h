/*
 * Copyright (c) 2022 - 2023, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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


#include <nrfx_gpiote.h>
#include <nrfx_saadc.h>


/**
 * @brief SAADC channel default configuration for the single-ended mode.
 *
 * This configuration sets up single-ended SAADC channel with the following options:
 * - resistor ladder disabled
 * - gain: 1/6
 * - reference voltage: internal 0.6 V
 * - sample acquisition time: 10 us
 * - burst disabled
 *
 * @param[in] _pin_p Positive input analog pin.
 * @param[in] _index Channel index.
 *
 * @sa nrfx_saadc_channel_t
 */
#define SAADC_CHANNEL_SE(_pin_p, _index)       \
{                                                           \
    .channel_config =                                       \
    {                                                       \
        .resistor_p = NRF_SAADC_RESISTOR_DISABLED,          \
        .resistor_n = NRF_SAADC_RESISTOR_DISABLED,          \
        .gain       = NRF_SAADC_GAIN1_6,                    \
        .reference  = NRF_SAADC_REFERENCE_INTERNAL,         \
        .acq_time   = NRF_SAADC_ACQTIME_10US,               \
        .mode       = NRF_SAADC_MODE_SINGLE_ENDED,          \
        .burst      = NRF_SAADC_BURST_DISABLED,             \
    },                                                      \
    .pin_p          = (nrf_saadc_input_t)_pin_p,            \
    .pin_n          = NRF_SAADC_INPUT_DISABLED,             \
    .channel_index  = _index,                               \
}


/**
 * @brief SAADC channel configuration for the differential mode.
 *
 * This configuration sets up differential SAADC channel with the following options:
 * - resistor ladder disabled
 * - gain: 1/6
 * - reference voltage: internal 0.6 V
 * - sample acquisition time: 10 us
 * - burst disabled
 *
 * @param[in] _pin_p Positive input analog pin.
 * @param[in] _pin_n Negative input analog pin.
 * @param[in] _index Channel index.
 *
 * @sa nrfx_saadc_channel_t
 */
#define SAADC_CHANNEL_DIFFERENTIAL(_pin_p, _pin_n, _index) \
{                                                                       \
    .channel_config =                                                   \
    {                                                                   \
        .resistor_p = NRF_SAADC_RESISTOR_DISABLED,                      \
        .resistor_n = NRF_SAADC_RESISTOR_DISABLED,                      \
        .gain       = NRF_SAADC_GAIN1_6,                                \
        .reference  = NRF_SAADC_REFERENCE_INTERNAL,                     \
        .acq_time   = NRF_SAADC_ACQTIME_10US,                           \
        .mode       = NRF_SAADC_MODE_DIFFERENTIAL,                      \
        .burst      = NRF_SAADC_BURST_DISABLED,                         \
    },                                                                  \
    .pin_p          = (nrf_saadc_input_t)_pin_p,                        \
    .pin_n          = (nrf_saadc_input_t)_pin_n,                        \
    .channel_index  = _index,                                           \
}


/** SAADC channel differential configuration structure for multiple channel use. */
static const nrfx_saadc_channel_t m_multiple_channels_diff[] =
{
    SAADC_CHANNEL_DIFFERENTIAL(NRF_SAADC_INPUT_AIN0, NRF_SAADC_INPUT_AIN1, 0),
    SAADC_CHANNEL_DIFFERENTIAL(NRF_SAADC_INPUT_AIN2, NRF_SAADC_INPUT_AIN3, 1),
    SAADC_CHANNEL_DIFFERENTIAL(NRF_SAADC_INPUT_AIN4, NRF_SAADC_INPUT_AIN5, 2),
    SAADC_CHANNEL_DIFFERENTIAL(NRF_SAADC_INPUT_AIN6, NRF_SAADC_INPUT_AIN7, 3)
};

/** SAADC channel single-ended configuration structure for multiple channel use. */
static const nrfx_saadc_channel_t m_multiple_channels_se[] =
{
    SAADC_CHANNEL_SE(NRF_SAADC_INPUT_AIN0, 0),
    SAADC_CHANNEL_SE(NRF_SAADC_INPUT_AIN2, 1),
    SAADC_CHANNEL_SE(NRF_SAADC_INPUT_AIN4, 2),
    SAADC_CHANNEL_SE(NRF_SAADC_INPUT_AIN6, 3)
};

/** @brief Symbol specifying numbers of multiple channels ( @ref m_multiple_channels) used by SAADC. */
#define CHANNEL_COUNT_SE NRFX_ARRAY_SIZE(m_multiple_channels_se)
#define CHANNEL_COUNT_DIFF NRFX_ARRAY_SIZE(m_multiple_channels_diff)

/** @brief Array specifying GPIO pins used to test the functionality of SAADC. */
//static uint8_t m_out_pins[CHANNEL_COUNT] = {LOOPBACK_PIN_1B, LOOPBACK_PIN_2B, LOOPBACK_PIN_3B};

/** @brief Samples buffer defined with the size of @ref CHANNEL_COUNT symbol to store values from each channel ( @ref m_multiple_channels). */
static nrf_saadc_value_t m_samples_buffer_se[CHANNEL_COUNT_SE];
static nrf_saadc_value_t m_samples_buffer_diff[CHANNEL_COUNT_DIFF];

/** @brief Symbol specifying the number of SAADC samplings to trigger. */
#define SAMPLING_ITERATIONS 1
#define RESISTOR 100


typedef struct saadc_result {
    uint8_t nchannels;
    nrf_saadc_value_t* values;
} saadc_result;


// Enum for selecting SAADC mode
enum mode {
    SE,
    DIFF
};

void sample_se(saadc_result* res);

void sample_diff(saadc_result* res);

void sample_saadc(int mode, saadc_result* res);

uint8_t to_millivolts(nrf_saadc_value_t value);

uint8_t to_volts(nrf_saadc_value_t value);

uint8_t to_microamps(nrf_saadc_value_t value);

uint8_t to_milliamps(nrf_saadc_value_t value);