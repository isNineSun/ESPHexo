#ifndef DDNS_H
#define DDNS_H

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#include "freertos/timers.h"

extern void ddns_update_timer_init(void);

extern bool get_ddns_status(void);

#endif