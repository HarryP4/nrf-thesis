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

#include "saadc_helpers.h"


static void _setup_se(void) {

    nrfx_err_t status = nrfx_saadc_init(NRFX_SAADC_DEFAULT_CONFIG_IRQ_PRIORITY);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    status = nrfx_saadc_channels_config(m_multiple_channels_se, CHANNEL_COUNT_SE);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    uint32_t channels_mask = nrfx_saadc_channels_configured_get();
    status = nrfx_saadc_simple_mode_set(channels_mask,
                                    NRF_SAADC_RESOLUTION_8BIT,
                                    NRF_SAADC_OVERSAMPLE_DISABLED,
                                    NULL);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    status = nrfx_saadc_buffer_set(m_samples_buffer_se, CHANNEL_COUNT_SE);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    return;
}


static void _setup_diff(void) {

    nrfx_err_t status = nrfx_saadc_init(NRFX_SAADC_DEFAULT_CONFIG_IRQ_PRIORITY);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    status = nrfx_saadc_channels_config(m_multiple_channels_diff, CHANNEL_COUNT_DIFF);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    uint32_t channels_mask = nrfx_saadc_channels_configured_get();
    status = nrfx_saadc_simple_mode_set(channels_mask,
                                    NRF_SAADC_RESOLUTION_8BIT,
                                    NRF_SAADC_OVERSAMPLE_DISABLED,
                                    NULL);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    status = nrfx_saadc_buffer_set(m_samples_buffer_diff, CHANNEL_COUNT_DIFF);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    return;
}


void sample_se() {

    _setup_se();

    nrfx_err_t status = nrfx_saadc_offset_calibrate(NULL);
    NRFX_ASSERT(status == NRFX_SUCCESS);
    NRFX_LOG_INFO("Calibration in the blocking manner finished successfully.");

    NRFX_LOG_INFO("Single-ended Sampling %d / %d", sampling_index, SAMPLING_ITERATIONS);
    NRFX_EXAMPLE_LOG_PROCESS();

    status = nrfx_saadc_mode_trigger();
    NRFX_ASSERT(status == NRFX_SUCCESS);
    int i;
    for (i = 0; i < CHANNEL_COUNT_SE; i++)
    {
        NRFX_LOG_INFO("[CHANNEL %u] Sampled value == %d",
                        m_multiple_channels_se[i].channel_index, m_samples_buffer_se[i]);
    }

    return;
}

void sample_diff() {

    _setup_diff();

    nrfx_err_t status = nrfx_saadc_offset_calibrate(NULL);
    NRFX_ASSERT(status == NRFX_SUCCESS);
    NRFX_LOG_INFO("Calibration in the blocking manner finished successfully.");

    NRFX_LOG_INFO("Differential Sampling %d / %d", sampling_index, SAMPLING_ITERATIONS);
    NRFX_EXAMPLE_LOG_PROCESS();

    status = nrfx_saadc_mode_trigger();
    NRFX_ASSERT(status == NRFX_SUCCESS);
    int i;
    for (i = 0; i < CHANNEL_COUNT_DIFF; i++)
    {
        NRFX_LOG_INFO("[CHANNEL %u] Sampled value == %d",
                        m_multiple_channels_diff[i].channel_index, m_samples_buffer_diff[i]);
    }

    return;
}

void sample_saadc(int mode) {

    if (mode == SE) {
        sample_se();
    } else if (mode == DIFF) {
        sample_diff();
    }

    return;
}