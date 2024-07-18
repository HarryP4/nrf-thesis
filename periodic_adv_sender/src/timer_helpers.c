#include "timer_helpers.h"
#include <zephyr/irq.h>


void timer_handler(nrf_timer_event_t event_type, void * p_context) {
	bool* flag = (bool*) p_context;
	if(!*flag) *flag = 1;
}


void timer_setup(nrfx_timer_t* timer, nrfx_timer_config_t* config) {

	nrfx_err_t status = nrfx_timer_init(timer, config, timer_handler);
    NRFX_ASSERT(status == NRFX_SUCCESS);


#if defined(__ZEPHYR__)
    IRQ_DIRECT_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_TIMER_INST_GET(TIMER_INST_IDX)), IRQ_PRIO_LOWEST,
                       NRFX_TIMER_INST_HANDLER_GET(TIMER_INST_IDX), 0);
#endif

	nrfx_timer_clear(timer);

	// Creating variable desired_ticks to store the output of nrfx_timer_ms_to_ticks function
	uint32_t desired_ticks = nrfx_timer_ms_to_ticks(timer, 5000);

	// Setting the timer channel NRF_TIMER_CC_CHANNEL0 in the extended compare mode to stop the timer and
	// trigger an interrupt if internal counter register is equal to desired_ticks.
	nrfx_timer_extended_compare(timer, NRF_TIMER_CC_CHANNEL1, desired_ticks,
								NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK, true);

	nrfx_timer_enable(timer);

	NRFX_LOG_INFO("Timer status: %s", nrfx_timer_is_enabled(timer) ? "enabled" : "disabled");

}