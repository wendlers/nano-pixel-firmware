/**
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_delay.h"
#include "boards.h"
#include "app_uart.h"

#include "app.h"

#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

static void timer_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{
	log_init();
	timer_init();

	NRF_LOG_INFO("*** BLE Nano v2 is alive - ws2812b BLE ***\n");

	App app;
    app.init();
    app.runForever();
}
