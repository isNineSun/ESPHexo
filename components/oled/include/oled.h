#ifndef SSD1306_CTRL_H
#define SSD1306_CTRL_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/timers.h"
#include "github_file_get.h"

extern void ssd1306_oled_init(void);

extern void ssd1306_show_device_status_timer_hdlr(TimerHandle_t xTimer);

extern void ssd1306_show_file_download_progress_timer_hdlr(bool dowload_compeleted);

#endif