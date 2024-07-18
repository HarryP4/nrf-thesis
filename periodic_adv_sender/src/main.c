/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "timer_helpers.h"
#include "bt_helpers.h"
#include "saadc_helpers.h"
#include <stdlib.h>



/*
================================
=#=#=#= Global Variables =#=#=#=
================================
*/

uint8_t backing_store[NUM_SUBEVENTS][PACKET_SIZE];

BUILD_ASSERT(ARRAY_SIZE(bufs) == ARRAY_SIZE(subevent_data_params));
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
	while(i < CHANNEL_COUNT_SE) {
		printk("Channel %d: %d\n", i, (int) res_se.values[i]);
		i++;
	}
	saadc_result res_diff = {CHANNEL_COUNT_SE, malloc(sizeof(saadc_result) * CHANNEL_COUNT_DIFF)};
	sample_saadc(DIFF, &res_diff);
	i = 0;
	while(i < CHANNEL_COUNT_DIFF) {
		printk("Channel %d: %d\n", i, (int) res_diff.values[i]);
		i++;
	}
}

int main(void) {
	
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
		.p_context = "SAADC samples taken"								///< Context passed to interrupt handler.
	};*/
	

	timer_setup(&timer, &config);

	while (true) {
		if (flag) {
			printk("interrupt\n");
			saadc_sample();
			printk("Starting bt:\n\n");
			bool err = initialise_bt(backing_store);
			if (err) return 1;
			k_sleep(K_MSEC(1500));
			flag = 0;
			bt_disable();
		}
		k_sleep(K_MSEC(100));
	}

	return 0;
}
