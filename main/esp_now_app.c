/*
 * esp_now_app.c
 *
 *  Created on: 28-05-2026
 *      Author: Francisco
 */

#include <string.h>
#include "esp_now_app.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "tasks_common.h"

static const char *TAG = "esp_now_app";

static EventGroupHandle_t local_system_events = NULL;

static uint8_t s_bell_mac[ESP_NOW_ETH_ALEN] = {0x88, 0x56, 0x21, 0xf0, 0x61, 0x68};

static QueueHandle_t s_espnow_queue = NULL;

static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    uint8_t * mac_addr = recv_info->src_addr;
    //uint8_t * des_addr = recv_info->des_addr;

    if (mac_addr == NULL || data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    ESP_LOGI(TAG, "Received data from " MACSTR ", data length: %d", MAC2STR(mac_addr), len);
	xEventGroupSetBits(local_system_events, RING_PRESSED_BIT);
}

static esp_err_t esp_now_app_init(void)
{
	ESP_ERROR_CHECK(esp_now_init());
	// ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));
	ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));
	/* Set primary master key. */ 
	// ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK) );

	/* Add broadcast peer information to peer list. */
	esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
	if (peer == NULL) {
		ESP_LOGE(TAG, "Malloc peer information fail");
		vQueueDelete(s_espnow_queue);
		s_espnow_queue = NULL;
		esp_now_deinit();
		return ESP_FAIL;
	}
	memset(peer, 0, sizeof(esp_now_peer_info_t));
	peer->channel = 0;
	peer->ifidx = WIFI_IF_STA;
	peer->encrypt = false;
	memcpy(peer->peer_addr, s_bell_mac, ESP_NOW_ETH_ALEN);
	ESP_ERROR_CHECK(esp_now_add_peer(peer));
	free(peer);
	return ESP_OK;
}

esp_err_t esp_now_app_start(EventGroupHandle_t system_events)
{
	local_system_events = system_events;

	xEventGroupWaitBits(local_system_events, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

	esp_now_app_init();

	return ESP_OK;
}