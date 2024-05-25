#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>

#include <wchar.h>
#include <stddef.h>
#include <stdlib.h>

#include "esp_http_server.h"
#include "url_decode.h"
#include "json_parse.h"
#include "sd_card.h"
#include "github_file_get.h"

#define IMAGES_RESOURCE 0
#define TEXT_RESOURCE   1
#define MAX_READ_BUFFER 1000

#define MAX_FILE_SIZE   (200*1024) // 200 KB
#define MAX_FILE_SIZE_STR "200KB"

/* Scratch buffer size */
#define SCRATCH_BUFSIZE  8192

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

struct file_server_data {
    /* Base path of file storage */
    char base_path[15 + 1];

    /* Scratch buffer for temporary storage during file transfer */
    char scratch[SCRATCH_BUFSIZE];
};

/// @brief 根请求响应处理
/// @param req 
/// @return esp_err_t
extern esp_err_t html_handler(httpd_req_t *req);

/// @brief HTTP请求响应处理
/// @param req 
/// @return esp_err_t
extern esp_err_t http_handler(httpd_req_t *req);

/// @brief 控制台请求响应
/// @param req 
/// @return esp_err_t
extern esp_err_t http_dashboard_handler(httpd_req_t *req);

/// @brief 控制台导航栏图标请求响应
/// @param req 
/// @return 
extern esp_err_t http_favicon_handler(httpd_req_t *req);

/// @brief 控制台资源混合请求
/// @param req 
/// @return 
extern esp_err_t http_dashboard_multi_handler(httpd_req_t *req);

/// @brief http修改设备参数配置请求
/// @param req 
/// @return 错误码
extern esp_err_t http_modify_handler(httpd_req_t *req);

/// @brief 更新Blog文件
/// @param req 
/// @return 错误码
extern esp_err_t http_updata_github_blog_handler(httpd_req_t *req);

#endif