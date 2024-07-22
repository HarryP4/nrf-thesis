/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "timer_helpers.h"
#include "bt_helpers.h"
#include "saadc_helpers.h"
#include <stdlib.h>
#include <ram_pwrdn.h>
#include <zephyr/pm/device.h>

/*
================================
=#=#=#= Global Variables =#=#=#=
================================
*/

uint8_t backing_store[NUM_SUBEVENTS][PACKET_SIZE];


BUILD_ASSERT(ARRAY_SIZE(backing_store) == ARRAY_SIZE(subevent_data_params));



/*
================================
=#=#=#= HELPER FUNCTIONS =#=#=#=
================================
*/


void saadc_sample() {
	saadc_result res_se = {CHANNEL_COUNT_SE, malloc(sizeof(saadc_result) * CHANNEL_COUNT_SE)};
	sample_saadc(SE, &res_se);
	int i = 0;
	/*
	while(i < res_se.nchannels) {
		backing_store[i][0] = (uint8_t) i;
		backing_store[i][1] = (uint8_t) res_se.values[i];
		printk("Channel %d: %d\n", i, (int) res_se.values[i]);
		i++;
	}*/
	

	backing_store[0][0] = (uint8_t) res_se.values[0]; // ADC channel 0
	backing_store[0][1] = (uint8_t) res_se.values[1]; // ADC channel 1
	backing_store[0][2] = (uint8_t) res_se.values[2]; // ADC channel 2
	backing_store[0][3] = (uint8_t) res_se.values[3]; // ADC channel 3

	while(i < res_se.nchannels) {
		int val = res_se.values[i];
		printk("SINGLE ENDED - Channel %d: %d (%d V, or %d mV)\n", i, (int) val, to_volts(val), to_millivolts(val));
		i++;
	}

	saadc_result res_diff = {CHANNEL_COUNT_SE, malloc(sizeof(saadc_result) * CHANNEL_COUNT_DIFF)};

	sample_saadc(DIFF, &res_diff);

	backing_store[0][4] = (uint8_t) res_diff.values[0]; // ADC channel 0
	backing_store[0][5] = (uint8_t) res_diff.values[1]; // ADC channel 1
	backing_store[0][6] = (uint8_t) res_diff.values[2]; // ADC channel 2
	backing_store[0][7] = (uint8_t) res_diff.values[3]; // ADC channel 3


	i = 0;
	while(i < res_diff.nchannels) {
		// Offset by the number of channels in the single-ended sample
		//backing_store[i + res_se.nchannels][0] = (uint8_t) i;
		//backing_store[i + res_se.nchannels][1] = (uint8_t) res_diff.values[i];
		printk("DIFFERENTIAL - Channel %d: %d\n", i, (int) res_diff.values[i]);
		i++;
	}
}

int main(void) {
	//nrf_uart_disable(NRF_UART0);

//	const auto * qspi_dev = DEVICE_DT_GET(DT_INST(0, nordic_qspi_nor));
//	if (device_is_ready(qspi_dev))
//	{
//		// Put the peripheral into suspended state.
//		pm_device_action_run(qspi_dev, PM_DEVICE_ACTION_SUSPEND);
//	}


	nrfx_timer_t timer = NRFX_TIMER_INSTANCE(TIMER_INST_IDX);
	nrfx_timer_config_t config = NRFX_TIMER_DEFAULT_CONFIG;
	config.bit_width = NRF_TIMER_BIT_WIDTH_32;
	bool flag = 0;
	config.p_context = &flag;
	/*{
		.frequency = NRF_TIMER_FREQ_16MHz,         						///< Frequency.
		.mode = NRF_TIMER_MODE_TIMER,           					    ///< Mode of operation.
		.bit_width = NRF_TIMER_BIT_WIDTH_32,        				  	///< Bit width.
		.interrupt_priority = NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY, 	///< Interrupt priority.
		.p_context = NULL												///< Context passed to interrupt handler.
	};*/
	
	saadc_sample();
	k_sleep(K_MSEC(400));

	bool err = initialise_bt(backing_store);
	if (err) return 1;

	k_sleep(K_MSEC(800));

	timer_setup(&timer, &config);

	while (true) {
		if (flag) {
			saadc_sample();
			update_bufs(backing_store);
			k_sleep(K_MSEC(5000));
			flag = 0;
			//bt_disable();
		}
		k_sleep(K_MSEC(5000));
		//power_down_unused_ram();
	}

	return 0;
}
