#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "nvs_flash.h"

#include "sd_card.h"
#include "wifi.h"
#include "http_server.h"

#ifdef USE_OLED
#include "oled.h"
#endif

#include "json_parse.h"

static const char *TAG = "Blog_Server";

void init_global_context(void)
{
    memset(&global_config_json, 0x00, sizeof(global_config_json));
}

void app_main() 
{
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_ERROR_CHECK(init_sd_card());
    ESP_LOGI(TAG, "SD card initialized");

    #ifdef USE_OLED
    ssd1306_oled_init();
    #endif

    init_global_context();

    if(get_global_config_from_json("/blog/bloxer_config.json"))
    {
        wifi_init(global_config_json.wifi_ssid, global_config_json.wifi_password);

        start_webserver();
    }

    // 等待
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // // 停止HTTP服务器
    // httpd_stop(server);
}
