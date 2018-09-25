/* Unified Provisioning Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_wifi.h>

#include <protocomm.h>
#include <protocomm_ble.h>

#include "wifi_prov.h"
#include "wifi_prov_mode_ble.h"

static const char *TAG = "wifi_prov_mode_ble";

extern wifi_prov_t wifi_prov_mode_ble;

static esp_err_t prov_start(protocomm_t *pc, void *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    wifi_prov_mode_ble_config_t *ble_config = (wifi_prov_mode_ble_config_t *) config;

    /* Start protocomm as BLE service */
    if (protocomm_ble_start(pc, ble_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start protocomm BLE service");
        return ESP_FAIL;
    }
    return ESP_OK;
}

static void *new_config(void)
{
    wifi_prov_mode_ble_config_t *ble_config = calloc(1, sizeof(wifi_prov_mode_ble_config_t));
    if (ble_config == NULL) {
        return NULL;
    }

    uint8_t service_uuid[16] = {
        /* LSB <---------------------------------------
         * ---------------------------------------> MSB */
         0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
         0x00, 0x10, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
    };

    memcpy(ble_config->service_uuid, service_uuid, sizeof(service_uuid));
    return ble_config;
}

static void delete_config(void *config)
{
    wifi_prov_mode_ble_config_t *ble_config = (wifi_prov_mode_ble_config_t *) config;
    for (unsigned int i = 0; i < ble_config->nu_lookup_count; i++) {
        free(ble_config->nu_lookup[i].name);
    }
    free(ble_config->nu_lookup);
    free(ble_config);
}

static esp_err_t set_config_service(void *config, const char *service_name, const char *service_key)
{
    wifi_prov_mode_ble_config_t *ble_config = (wifi_prov_mode_ble_config_t *) config;
    strlcpy(ble_config->device_name,  service_name, sizeof(ble_config->device_name));
    return ESP_OK;
}

static esp_err_t set_config_endpoint(void *config, const char *endpoint_name, uint16_t uuid)
{
    wifi_prov_mode_ble_config_t *ble_config = (wifi_prov_mode_ble_config_t *) config;

    char *copy_ep_name = strdup(endpoint_name);
    if (!copy_ep_name) {
        return ESP_ERR_NO_MEM;
    }

    protocomm_ble_name_uuid_t *lookup_table = (
        realloc(ble_config->nu_lookup, (ble_config->nu_lookup_count + 1) * sizeof(protocomm_ble_name_uuid_t)));
    if (!lookup_table) {
        return ESP_ERR_NO_MEM;
    }

    lookup_table[ble_config->nu_lookup_count].name = copy_ep_name;
    lookup_table[ble_config->nu_lookup_count].uuid = uuid;
    ble_config->nu_lookup = lookup_table;
    ble_config->nu_lookup_count += 1;
    return ESP_OK;
}

wifi_prov_t wifi_prov_mode_ble = {
    .prov_start          = prov_start,
    .prov_stop           = protocomm_ble_stop,
    .new_config          = new_config,
    .delete_config       = delete_config,
    .set_config_service  = set_config_service,
    .set_config_endpoint = set_config_endpoint,
    .wifi_mode           = WIFI_MODE_STA
};
