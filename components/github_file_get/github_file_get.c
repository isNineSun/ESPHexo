#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "github_file_get.h"

// 全局变量用于存储生成的URL
char FILE_REPO_URL[512] = {0};
char FILE_BASE_URL[512] = {0};

#define FILR_BASE_PATH "/blog/public/"
#define FILE_PATH "/blog/gihub_blog.json"

static const char *TAG = "GithubDownload";

#define MAX_RETRY_COUNT 3
#define DELAY_BETWEEN_REQUESTS_MS 1000

typedef struct {
    char filePath[256];
    char fileURL[512];
} download_task_param_t;

static void download_github_file_in_order(void);

char **blob_paths = NULL;
int blob_count = 0;
int num_blobs = 0;

/// @brief 合并文件路径
/// @param base_url 
/// @param file_path 
/// @return 
static char* combine_path(const char* base_url, const char* file_path) {
    size_t base_len = strlen(base_url);
    size_t path_len = strlen(file_path);
    
    char* full_url = (char*)malloc(base_len + path_len + 1);
    if (full_url == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    
    snprintf(full_url, base_len + path_len + 1, "%s%s", base_url, file_path);
    
    return full_url;
}

/// @brief 合并URL
/// @param base_url 
/// @param file_path 
/// @return 
static char* combine_url(const char* base_url, const char* file_path) 
{
    char* encoded_path = url_encode(file_path);
    if (encoded_path == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    
    size_t base_len = strlen(base_url);
    size_t path_len = strlen(encoded_path);
    
    char* full_url = (char*)malloc(base_len + path_len + 1);
    if (full_url == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(encoded_path);
        return NULL;
    }
    
    snprintf(full_url, base_len + path_len + 1, "%s%s", base_url, encoded_path);
    
    free(encoded_path);
    return full_url;
}

/// @brief 根据Json配置文件配置仓库的Url链接
static bool set_repo_urls(void) 
{
    if (strcmp(global_config_json.repo_type, "github") == 0) 
    {
        snprintf(FILE_REPO_URL, sizeof(FILE_REPO_URL),
                 "https://api.github.com/repos/%s/%s/git/trees/master?recursive=1",
                 global_config_json.repo_owner, global_config_json.repo_name);
        snprintf(FILE_BASE_URL, sizeof(FILE_BASE_URL),
                 "https://raw.githubusercontent.com/%s/%s/master/",
                 global_config_json.repo_owner, global_config_json.repo_name);

        return true;
    } 
    else if (strcmp(global_config_json.repo_type, "gitee") == 0) 
    {
        snprintf(FILE_REPO_URL, sizeof(FILE_REPO_URL),
                 "https://gitee.com/api/v5/repos/%s/%s/git/trees/master?recursive=1",
                 global_config_json.repo_owner, global_config_json.repo_name);
        snprintf(FILE_BASE_URL, sizeof(FILE_BASE_URL),
                 "https://gitee.com/%s/%s/raw/master/",
                 global_config_json.repo_owner, global_config_json.repo_name);
            
        return true;
    } 
    else 
    {
        ESP_LOGE(TAG, "Unsupported repo_type: %s\n", global_config_json.repo_type);
        return false;
    }
}

/// @brief http处理函数
/// @param evt 
/// @return 
static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
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
            break;
        case HTTP_EVENT_ON_DATA:
            // Write received data to a file
            FILE* f = fopen(((download_task_param_t*)evt->user_data)->filePath, "a");
            if (f == NULL) {
                ESP_LOGE(TAG, "Failed to open file for writing: %s", ((download_task_param_t*)evt->user_data)->filePath);
                return ESP_FAIL;
            }
            fwrite(evt->data, 1, evt->data_len, f);
            fclose(f);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED\n");
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

/// @brief 下载单个文件的任务
/// @param param 
static void download_file_task(void *param) 
{
    download_task_param_t *task_param = (download_task_param_t *)param;

    sd_card_save_file(((download_task_param_t *)param)->filePath);

    esp_http_client_config_t config = {
        .url = task_param->fileURL,
        .user_data = param,
        .event_handler = http_event_handler,
        .timeout_ms = 10000, // 设置超时时间为10秒
        
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    int retry_count = 0;
    esp_err_t err;
    do {
        err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "File downloaded successfully: %s", task_param->filePath);
            break;
        } else {
            ESP_LOGE(TAG, "Download failed: %s, retrying...", task_param->filePath);
            vTaskDelay(pdMS_TO_TICKS(DELAY_BETWEEN_REQUESTS_MS)); // 延迟1秒
        }
        retry_count++;
    } while (retry_count < MAX_RETRY_COUNT);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to download file after %d attempts: %s", MAX_RETRY_COUNT, task_param->filePath);
    }

    esp_http_client_cleanup(client);

    // 继续下载下一个文件
    download_github_file_in_order();

    // 释放参数并删除任务
    free(task_param);
    vTaskDelete(NULL);
}

/// @brief 下载仓库中的单个文件
/// @param filename 
/// @param fileurl 
static void download_github_repo_file(const char* filename, const char* fileurl) {
    download_task_param_t *param = malloc(sizeof(download_task_param_t));
    if (param == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for task parameter");
        return;
    }

    strcpy(param->filePath, filename);
    strcpy(param->fileURL, fileurl);

    xTaskCreate(download_file_task, "download_file_task", 8192, param, 5, NULL);
}

/// @brief 按顺序下载Github仓库中的文件
/// @param  
static void download_github_file_in_order(void) {
    if (blob_count < num_blobs) {
        char* full_url = combine_url(FILE_BASE_URL, blob_paths[blob_count]);
        char* full_path = combine_path(FILR_BASE_PATH, blob_paths[blob_count]);
        printf("Blob %d: %s\n", blob_count + 1, full_url);

        #ifdef USE_OLED
        ssd1306_show_file_download_progress_timer_hdlr(false);
        #endif

        free(blob_paths[blob_count]);  // Free each path string
        blob_count++;

        download_github_repo_file(full_path, full_url);
        free(full_url);
        free(full_path);
    } 
    else 
    {
        #ifdef USE_OLED
        ssd1306_show_file_download_progress_timer_hdlr(true);
        #endif
        ESP_LOGI(TAG, "All files downloaded successfully");
    }
}

/// @brief 获取Github仓库的文件列表
/// @param  
void get_github_file_json_index(void) 
{
    ESP_LOGI(TAG, "<----------------------------------------Start Get Github File---------------------------------------->\n");

    if(!set_repo_urls())
    {
        return;
    }

    github_file_client req_data;
    strcpy(req_data.FilePath, FILE_PATH);
    req_data.req_type = GET_INDEX;

    esp_http_client_config_t config = {
        .url = FILE_REPO_URL,
        .user_data = (void *)&req_data,
        .event_handler = http_event_handler,
        .timeout_ms = 10000, // 设置超时时间为10秒
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    FILE* f = fopen(FILE_PATH, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to create file for writing\n");
        return;
    }
    fclose(f);

    int retry_count = 0;
    esp_err_t err;
    do {
        err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "File downloaded successfully\n");
            blob_paths = read_github_json_info_file(FILE_PATH, &num_blobs);
            if (blob_paths != NULL) {
                blob_count = 0;
                download_github_file_in_order();
            }
            break;
        } else {
            ESP_LOGE(TAG, "File download failed, retrying...\n");
            vTaskDelay(pdMS_TO_TICKS(DELAY_BETWEEN_REQUESTS_MS)); // 延迟1秒
        }
        retry_count++;
    } while (retry_count < MAX_RETRY_COUNT);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to download file index after %d attempts\n", MAX_RETRY_COUNT);
    }

    esp_http_client_cleanup(client);
}
