#include <nrfx_timer.h>


/** @brief Symbol specifying timer instance to be used. */
#define TIMER_INST_IDX 1
#define DESIRED_MS_DELAY 25000

void timer_handler(nrf_timer_event_t event_type, void * p_context);
void timer_setup(nrfx_timer_t* timer, nrfx_timer_config_t* config);