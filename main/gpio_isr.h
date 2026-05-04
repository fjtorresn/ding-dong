/*
 * gpio_isr.h
 *
 *  Created on: 03-05-2026
 *      Author: Francisco
 */

#ifndef MAIN_GPIO_ISR_H_
#define MAIN_GPIO_ISR_H_

#define BUTTON_INPUT_GPIO		18
#define ESP_INTR_FLAG_DEFAULT 	0

void gpio_isr_start(void);


#endif /* MAIN_GPIO_ISR_H_ */
