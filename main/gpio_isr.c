/*
 * gpio_isr.c
 *
 *  Created on: 03-05-2026
 *      Author: Francisco
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
//#include "freertos/semphr.h"
#include "driver/gpio.h"

#include "gpio_isr.h"

static QueueHandle_t gpio_evt_queue = NULL;
//static SemaphoreHandle_t gpio_evt_smph = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}
 
void gpio_isr_start(void) {
	gpio_config_t io_conf = {};
	
	//disable interrupt
	io_conf.intr_type = GPIO_INTR_NEGEDGE;
	//set as output mode
	io_conf.mode = GPIO_MODE_INPUT;
	//bit mask of the pins that you want to set,e.g.GPIO18/19
	io_conf.pin_bit_mask = (1ULL << BUTTON_INPUT_GPIO);
	
	//disable pull-down mode
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	//disable pull-up mode
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	//configure GPIO with the given settings
	
	gpio_config(&io_conf);

	//create a queue to handle gpio event from isr
	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
	//start gpio task
	xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

	//install gpio isr service
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	//hook isr handler for specific gpio pin
	gpio_isr_handler_add(BUTTON_INPUT_GPIO, gpio_isr_handler, (void*) BUTTON_INPUT_GPIO);


	//printf("Minimum free heap size: %"PRIu32" bytes\n", esp_get_minimum_free_heap_size());

	//while (1);
}

