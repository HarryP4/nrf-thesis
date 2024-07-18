#include <nrfx_timer.h>
#include <nrfx_log.h>


/** @brief Symbol specifying timer instance to be used. */
#define TIMER_INST_IDX 1
	
void timer_handler(nrf_timer_event_t event_type, void * p_context);
void timer_setup(nrfx_timer_t* timer, nrfx_timer_config_t* config);