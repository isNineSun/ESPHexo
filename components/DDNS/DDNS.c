#include <stdio.h>
#include <stdbool.h>

#include "wifi.h"
#include "ddns.h"

#ifdef USE_OLED
#include "oled.h"
#endif

#include "json_parse.h"

static const char *TAG = "DuckDNS";

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

// 定义Duck DNS更新URL
#define DUCK_DNS_UPDATE_URL "https://www.duckdns.org/update?domains=%s&token=%s&ip=127.0.0.2&ipv6=%s"

// Duck DNS域名和Token
#define DUCK_DNS_DOMAIN "isbloxer"
#define DUCK_DNS_TOKEN "b2517264-3d2a-4874-a74a-8356569fc9ac"

static char response_buffer[MAX_HTTP_RECV_BUFFER];
static bool ddns_status;
TimerHandle_t ddns_timer;

/// @brief 获取当前DDNS状态
/// @param  
/// @return 
bool get_ddns_status(void)
{
    return ddns_status;
}

static bool check_if_ddns_update_successed(char* resp)
{
    //确定第一行
    const char *newline_pos = strchr(resp, '\n');
    if (newline_pos == NULL) {
        newline_pos = resp + strlen(resp);
    }

    // 检查是否包含 "OK"
    if (strstr(resp, "OK") != NULL) 
    {
        return true;
    } 
    else
    {
        return false;
    }
}

static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            ESP_LOGI(TAG, "url respones: %s", (char*)evt->data);
            ddns_status = check_if_ddns_update_successed((char*)evt->data);

            #ifdef USE_OLED
            ssd1306_show_device_status_timer_hdlr(NULL);
            #endif

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
            esp_http_client_set_header(evt->client, "From", "user@example.com");
            esp_http_client_set_header(evt->client, "Accept", "text/html");
            esp_http_client_set_redirection(evt->client);
            break;
    }
    return ESP_OK;
}


/// @brief 发送HTTPS GET请求更新Duck DNS记录
/// @param xTimer 定时器指针
static void update_duck_dns(TimerHandle_t xTimer) 
{
    char update_url[256];
    if(strcmp(global_config_json.ddns_domain, "-") == 0 || strcmp(global_config_json.ddns_token, "-") == 0)
    {
        ESP_LOGI(TAG, "NO DDNS...");
        return;//无DDNS信息，不进行DDNS
    }
    snprintf(update_url, sizeof(update_url), DUCK_DNS_UPDATE_URL, global_config_json.ddns_domain, global_config_json.ddns_token, wifi_get_ipv6_address_str());

    ESP_LOGI(TAG, "DDNS url: %s", update_url);
    esp_http_client_config_t config = {
        .url = update_url,
        .event_handler = _http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .user_data = response_buffer,
        .buffer_size = MAX_HTTP_RECV_BUFFER,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %"PRIu64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);

    xTimerStop(xTimer, 0);
}

/// @brief DDNS更新请求定时器初始化
/// @param sec 定时器间隔，单位：秒（s）
void ddns_update_timer_init(void)
{
    ESP_LOGI(TAG, "Update DDNS...");
    // 初始化定时器
    ddns_timer = xTimerCreate("DDNS_Timer",        // 定时器名称
                              2000 / portTICK_PERIOD_MS,
                              pdTRUE,            // 循环定时器
                              (void *)0,         
                              update_duck_dns); // 定时器回调函数

    if (ddns_timer == NULL) {
        //nothing
    }

    xTimerStart(ddns_timer, 0);
}
