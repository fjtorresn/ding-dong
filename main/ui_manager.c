/*
 * ui_manager.c
 *
 *  Created on: 03-05-2026
 *      Author: Francisco
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "tasks_common.h"
#include "ui_manager.h"


static const char *TAG = "UI_MANAGER";

static EventGroupHandle_t local_system_events = NULL;

static uint8_t s_led_state = 0;

static SemaphoreHandle_t gpio_evt_smph = NULL;


static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	static uint32_t last_registered_time = 0;
	
	uint32_t current_time = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
	
	if (current_time - last_registered_time > DEBOUNCE_TIME_MS) {
		xSemaphoreGiveFromISR(gpio_evt_smph, NULL);
		last_registered_time = current_time;
	}
}

static void ui_manager_task(void* arg)
{
    for (;;) {
        if (xSemaphoreTake(gpio_evt_smph, portMAX_DELAY)) {
			gpio_set_level(BLINK_OUTPUT_GPIO, s_led_state);
			s_led_state = !s_led_state;
			ESP_LOGI(TAG, "Botón presionado");
			xEventGroupSetBits(local_system_events, RING_PRESSED_BIT);
        }
    }
}
 
void ui_manager_start(EventGroupHandle_t system_events) {

	// Configuration for the LED
	gpio_reset_pin(BLINK_OUTPUT_GPIO);
    gpio_set_direction(BLINK_OUTPUT_GPIO, GPIO_MODE_OUTPUT);

	// Configuration for the button
	gpio_config_t io_conf = {};
	
	io_conf.intr_type = GPIO_INTR_NEGEDGE;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pin_bit_mask = (1ULL << BUTTON_INPUT_GPIO);
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE;	
	gpio_config(&io_conf);

	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	gpio_isr_handler_add(BUTTON_INPUT_GPIO, gpio_isr_handler, (void*) BUTTON_INPUT_GPIO);

	// Inter process communication
	gpio_evt_smph = xSemaphoreCreateBinary();
	local_system_events = system_events;

	// Task
	xTaskCreatePinnedToCore(ui_manager_task, "ui_manager_task", UI_MANAGER_TASK_STACK_SIZE, NULL, UI_MANAGER_TASK_PRIORITY, NULL, UI_MANAGER_TASK_CORE_ID);

}

