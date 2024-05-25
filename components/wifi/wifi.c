#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_netif.h"

#include "wifi.h"
#include "ddns.h"

static const char *TAG = "wifi_station";
static int s_retry_num = 0;
static esp_netif_t *s_network_sta_netif = NULL;
static SemaphoreHandle_t s_semph_get_ip_addrs = NULL;
static SemaphoreHandle_t s_semph_get_ip6_addrs = NULL;

static char ipv4_str[16] = {0};
static char ipv6_str[42] = {0};

const char *network_ipv6_addr_types_to_str[6] = {
    "ESP_IP6_ADDR_IS_UNKNOWN",
    "ESP_IP6_ADDR_IS_GLOBAL",
    "ESP_IP6_ADDR_IS_LINK_LOCAL",
    "ESP_IP6_ADDR_IS_SITE_LOCAL",
    "ESP_IP6_ADDR_IS_UNIQUE_LOCAL",
    "ESP_IP6_ADDR_IS_IPV4_MAPPED_IPV6"
};

char* wifi_get_ipv4_address_str(void)
{
    return ipv4_str;
}

char* wifi_get_ipv6_address_str(void)
{
    return ipv6_str;
}

/// @brief 通过描述检查网络接口（network interface）是否为指定的接口
/// @param prefix 
/// @param netif 
/// @return 
bool network_is_our_netif(const char *prefix, esp_netif_t *netif)
{
    return strncmp(prefix, esp_netif_get_desc(netif), strlen(prefix) - 1) == 0;
}

static void network_handler_on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    s_retry_num++;
    if (s_retry_num > 6) {
        ESP_LOGI(TAG, "WiFi Connect failed %d times, stop reconnect.", s_retry_num);
        /* let network_wifi_sta_do_connect() return */
        if (s_semph_get_ip_addrs) {
            xSemaphoreGive(s_semph_get_ip_addrs);
        }
        if (s_semph_get_ip6_addrs) {
            xSemaphoreGive(s_semph_get_ip6_addrs);
        }
        return;
    }
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        return;
    }
    ESP_ERROR_CHECK(err);
}

/// @brief wifi连接的处理函数
/// @param esp_netif 
/// @param event_base 
/// @param event_id 
/// @param event_data 
static void network_handler_on_wifi_connect(void *esp_netif, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    esp_netif_create_ip6_linklocal(esp_netif);
}

/// @brief 获取ip后触发的处理函数
/// @param arg 
/// @param event_base 
/// @param event_id 
/// @param event_data 
static void network_handler_on_sta_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    s_retry_num = 0;
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    if (!network_is_our_netif(NETWORK_NETIF_DESC_STA, event->esp_netif)) {
        return;
    }
    ESP_LOGI(TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    sprintf(ipv4_str, IPSTR, IP2STR(&event->ip_info.ip));//更新设备ipv4地址状态
    if (s_semph_get_ip_addrs) {
        xSemaphoreGive(s_semph_get_ip_addrs);
    } else {
        ESP_LOGI(TAG, "- IPv4 address: " IPSTR ",", IP2STR(&event->ip_info.ip));
    }
}

/// @brief 获取ipv6后触发的处理函数,若要获取公网IPV6，需要在menuconfig中打开IPv6 SLAAC
/// @param arg 
/// @param event_base 
/// @param event_id 
/// @param event_data 
static void network_handler_on_sta_got_ipv6(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
    if (!network_is_our_netif(NETWORK_NETIF_DESC_STA, event->esp_netif)) {
        return;
    }
    esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&event->ip6_info.ip);
    ESP_LOGI(TAG, "Got IPv6 event: Interface \"%s\" address: " IPV6STR ", type: %s", esp_netif_get_desc(event->esp_netif),
             IPV62STR(event->ip6_info.ip), network_ipv6_addr_types_to_str[ipv6_type]);

    if(ipv6_type == ESP_IP6_ADDR_IS_GLOBAL)
    {
        sprintf(ipv6_str, IPV6STR, IPV62STR(event->ip6_info.ip));//更新设备ipv6地址字符串存储
        ddns_update_timer_init();
    }

    if (ipv6_type == ESP_IP6_ADDR_IS_LINK_LOCAL) 
    {
        if (s_semph_get_ip6_addrs) {
            xSemaphoreGive(s_semph_get_ip6_addrs);
        } else {
            ESP_LOGI(TAG, "- IPv6 address: " IPV6STR ", type: %s", IPV62STR(event->ip6_info.ip), network_ipv6_addr_types_to_str[ipv6_type]);
        }
    }
}

/// @brief wifi连接处理
/// @param wifi_config 
/// @param wait 
/// @return 
esp_err_t network_wifi_sta_do_connect(wifi_config_t wifi_config, bool wait)
{
    if (wait) {
        s_semph_get_ip_addrs = xSemaphoreCreateBinary();
        if (s_semph_get_ip_addrs == NULL) {
            return ESP_ERR_NO_MEM;
        }
        s_semph_get_ip6_addrs = xSemaphoreCreateBinary();
        if (s_semph_get_ip6_addrs == NULL) {
            vSemaphoreDelete(s_semph_get_ip_addrs);
            return ESP_ERR_NO_MEM;
        }
    }
    s_retry_num = 0;

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &network_handler_on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &network_handler_on_sta_got_ip, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &network_handler_on_wifi_connect, s_network_sta_netif));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &network_handler_on_sta_got_ipv6, NULL));

    ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_err_t ret = esp_wifi_connect();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi connect failed! ret:%x", ret);
        return ret;
    }
    if (wait) {
        ESP_LOGI(TAG, "Waiting for IP(s)");
        xSemaphoreTake(s_semph_get_ip_addrs, portMAX_DELAY);
        xSemaphoreTake(s_semph_get_ip6_addrs, portMAX_DELAY);
        if (s_retry_num > 6) {
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}

/// @brief wifi初始化
/// @param ssid 
/// @param password 
void wifi_init(char *wifi_ssid, char *wifi_password) 
{
    // 初始化 TCP/IP 适配器 网络接口netif（network interface）初始化
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //s_network_sta_netif = esp_netif_create_default_wifi_sta();
    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    esp_netif_config.if_desc = NETWORK_NETIF_DESC_STA;
    esp_netif_config.route_prio = 128;
    s_network_sta_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    esp_wifi_set_default_wifi_sta_handlers();

    //wifi 配置初始化
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0x00, sizeof(wifi_config_t));
    strcpy((char*)wifi_config.sta.ssid, wifi_ssid);
    strcpy((char*)wifi_config.sta.password, wifi_password);

    // wifi_config_t wifi_config = {
    //     .sta = {
    //         .ssid = wifi_ssid,
    //         .password = wifi_password,
    //     },
    // };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    network_wifi_sta_do_connect(wifi_config, true);
}