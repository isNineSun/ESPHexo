#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ssd1306.h"
#include "oled.h"
#include "wifi.h"
#include "ddns.h"
#include "sd_card.h"

#define tag "SSD1306"

static void ssd1306_show_device_status_timer_init(uint8_t sec);

const unsigned char bloxer_logo[] = {
	// 'hexo_logo, 128x64px
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x01, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0f, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x1f, 0xff, 0xff, 0xff, 0xfe, 0x01, 0xe1, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x01, 0xe1, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x81, 0xe1, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x81, 0xe1, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x81, 0xe1, 0xe1, 0xf9, 0xf3, 0xcf, 0xf0, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x81, 0xff, 0xe3, 0xfc, 0xf7, 0xdf, 0xf8, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x81, 0xff, 0xe7, 0xfe, 0xff, 0x9f, 0xf8, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf8, 0xff, 0xe3, 0xff, 0x81, 0xff, 0xe7, 0x9e, 0x7f, 0x1e, 0x78, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x81, 0xff, 0xe7, 0xfe, 0x3f, 0x3e, 0x3c, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x81, 0xe1, 0xe7, 0xfe, 0x3f, 0x3c, 0x3c, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x81, 0xe1, 0xe7, 0x80, 0x7f, 0x9e, 0x78, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x81, 0xe1, 0xe7, 0xfc, 0xff, 0x9f, 0xf8, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x81, 0xe1, 0xe3, 0xfc, 0xf7, 0xdf, 0xf8, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x81, 0xe1, 0xe1, 0xfd, 0xf3, 0xcf, 0xf0, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x00, 0x03, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x00, 0x03, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x00, 0x03, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x00, 0x03, 0xff, 0x80, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x81, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x81, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x81, 0xff, 0x9e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x81, 0xe7, 0x9e, 0x0f, 0x8f, 0x1e, 0x3e, 0x1f, 0xc0, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x81, 0xe7, 0x9e, 0x3f, 0xcf, 0xbe, 0x7f, 0x1f, 0xc0, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x81, 0xff, 0x1e, 0x7f, 0xe7, 0xfc, 0xff, 0x9f, 0xc0, 0x00, 
	0x00, 0x7f, 0xf0, 0x7f, 0xc3, 0xff, 0x81, 0xff, 0x1e, 0x7f, 0xf7, 0xfc, 0xf7, 0x9f, 0xc0, 0x00, 
	0x00, 0x7f, 0xf8, 0xff, 0xc3, 0xff, 0x81, 0xff, 0x9e, 0x78, 0xf3, 0xf9, 0xff, 0x9e, 0x00, 0x00, 
	0x00, 0x7f, 0xfd, 0xff, 0xf7, 0xff, 0x81, 0xef, 0x9e, 0x78, 0xf1, 0xf1, 0xff, 0x9e, 0x00, 0x00, 
	0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x81, 0xe7, 0xde, 0x78, 0xf3, 0xf9, 0xff, 0x9e, 0x00, 0x00, 
	0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x81, 0xef, 0xde, 0x7d, 0xf3, 0xfd, 0xf1, 0x1e, 0x00, 0x00, 
	0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x81, 0xff, 0x9e, 0x7f, 0xe7, 0xfc, 0xff, 0x1e, 0x00, 0x00, 
	0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x81, 0xff, 0x9e, 0x3f, 0xef, 0xbe, 0xff, 0x1e, 0x00, 0x00, 
	0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x81, 0xfe, 0x1e, 0x1f, 0x8f, 0x1e, 0x3f, 0x1e, 0x00, 0x00, 
	0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0f, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x07, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

SSD1306_t dev;
TimerHandle_t clearTimer;
TimerHandle_t statusTimer;

/// @brief 定时器回调函数，用于清屏
/// @param xTimer 
static void clearScreenTimerCallback(TimerHandle_t xTimer) {
    ssd1306_clear_screen(&dev, false);
    ssd1306_show_device_status_timer_init(120);
}

/// @brief 开机展示Logo
/// @param dev 
static void ssd1306_boot_Logo_Display()
{
    // 显示图片
    ssd1306_bitmaps(&dev, 0, 0, (uint8_t*)bloxer_logo, 128, 64, false);

    // 初始化定时器
    clearTimer = xTimerCreate("ClearTimer",        // 定时器名称
                              2000 / portTICK_PERIOD_MS,  // 定时器周期，2秒
                              pdFALSE,            // 单次定时器
                              (void *)0,          // 定时器ID，这里没用到，设置为0
                              clearScreenTimerCallback); // 定时器回调函数

    if (clearTimer == NULL) {
        //nothing
    }

    xTimerStart(clearTimer, 0);
}

/// @brief 屏幕显示设备状态更新函数
void ssd1306_show_device_status_timer_hdlr(TimerHandle_t xTimer)
{
    //本地IP
    ssd1306_display_text(&dev, 0, "Local IP:       ", 16, true);
	ssd1306_display_text(&dev, 1, wifi_get_ipv4_address_str(), 16, true);

    //DDNS 状态
    char ddns_status[16] = {0};
    sprintf(ddns_status, "DDNS: %s", get_ddns_status()?"OK":"Fail");
    ssd1306_display_text(&dev, 3, ddns_status, 10, false);

    //Blog文件是否准备好
    char Blog_status[16] = {0};
    sprintf(Blog_status, "Blog: %s",  Check_if_the_file_exists("/blog/public/index.html")?"Ready!^_^":"Not Ready");
    ssd1306_display_text(&dev, 5, Blog_status, strlen(Blog_status), false);
}


/// @brief 在SSD1306上绘制进度条
/// @param dev 
/// @param page 
/// @param start_col 
/// @param end_col 
/// @param progress 
static void draw_progress_bar(SSD1306_t *dev, int page, int start_col, int end_col, int progress) 
{
    // 确保进度值在 0 到 100 之间
    if (progress < 0) progress = 0;
    if (progress > 100) progress = 100;

    // 计算进度条长度
    int bar_length = ((end_col - start_col + 1) * progress) / 100;

    // 创建一个临时缓冲区，用于存储进度条数据
    uint8_t buffer[128];
    memset(buffer, 0, sizeof(buffer));

    // 填充进度条部分
    for (int i = 0; i < bar_length; i++) 
    {
        buffer[i] = 0xFF;
    }

    // 显示进度条
    ssd1306_display_image(dev, page, start_col, buffer, end_col - start_col + 1);
}

/// @brief 屏幕下载进度管理
/// @param xTimer 
void ssd1306_show_file_download_progress_timer_hdlr(bool dowload_compeleted)
{
    if(dowload_compeleted)
    {
        ssd1306_clear_screen(&dev, false);

        ssd1306_show_device_status_timer_hdlr(NULL);
    }
    else
    {
        //文件下载进度
        char download_progress[16] = {0};
        sprintf(download_progress, "DL: %d/%d      ",  blob_count, num_blobs);
        ssd1306_display_text(&dev, 5, download_progress, 16, false);
        draw_progress_bar(&dev, 6, 1, 127, ((blob_count * 100)/num_blobs));
    }
}

/// @brief 屏幕设备状态更新timer初始化
/// @param sec 更新间隔，单位：秒(S)
static void ssd1306_show_device_status_timer_init(uint8_t sec)
{
    ssd1306_show_device_status_timer_hdlr(NULL);

    // 初始化定时器
    statusTimer = xTimerCreate("ClearTimer",        // 定时器名称
                              (sec * 1000) / portTICK_PERIOD_MS,
                              pdTRUE,            // 循环定时器
                              (void *)0,         
                              ssd1306_show_device_status_timer_hdlr); // 定时器回调函数

    if (statusTimer == NULL) {
        //nothing
    }

    xTimerStart(statusTimer, 0);
}

/// @brief ssd1306初始化屏幕
/// @param  
void ssd1306_oled_init(void)
{
	ESP_LOGI(tag, "INTERFACE is i2c");
	ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);


	ESP_LOGI(tag, "Panel is 128x64");
	ssd1306_init(&dev, 128, 64);


	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	
    ssd1306_boot_Logo_Display();
}
