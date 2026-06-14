/*
 * esp_now_app.h
 *
 *  Created on: 28-05-2026
 *      Author: Francisco
 */
#ifndef ESP_NOW_APP_H
#define ESP_NOW_APP_H

#include "esp_now.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define ESPNOW_MAXDELAY 	512
#define ESPNOW_QUEUE_SIZE 	6
#define WIFI_CHANNEL 		0
#define ESPNOW_DATA_SIZE 	4

typedef enum {
    ESPNOW_SEND,
    ESPNOW_RECV,
} espnow_event_id_t;

/**
 * Data sent through ESP_NOW
 */
typedef struct {
	uint8_t dest_mac[6];
	uint8_t payload[ESPNOW_DATA_SIZE]; // Real payload of ESPNOW data.
	int len;
} __attribute__((packed)) espnow_data_t;

/**
* Data sent by SEND_CB to the queue
*/
typedef struct {
	uint8_t mac_addr[6];
	esp_now_send_status_t status;
} espnow_send_event_t;


/**
* Data sent by RECV_CB to the queue
*/
typedef struct {
	uint8_t mac_addr[6];
    uint8_t *data;
    int data_len;
} espnow_recv_event_t;

typedef union {
	espnow_send_event_t send_data;
    espnow_recv_event_t recv_data;
} espnow_event_cb_t;

/**
 * Event sent to the queue
 */
typedef struct{
	espnow_event_id_t id;
	espnow_event_cb_t event_cb;
} espnow_event_t;

esp_err_t esp_now_app_start(EventGroupHandle_t system_events);

#endif // ESP_NOW_APP_H