
#include "http_handler.h"

#define MAX_URL_PARAMS 10

typedef struct {
    char name[30];
    char value[50];
} url_Param;

static const char *TAG = "http_Handler";

/// @brief 查询路径是否是一个文件夹
/// @param path 
/// @return -true 是文件夹 -false 不是文件夹
static bool filepath_is_directory(const char *path) {
    struct stat file_stat;
    if (stat(path, &file_stat) != 0) {
        // 获取文件信息失败
        ESP_LOGE(TAG, "Failed to stat dir : %s", path);
        return false;
    }

    // 检查文件类型是否是文件夹
    return S_ISDIR(file_stat.st_mode);
}

/// @brief 网络请求类型
/// @param req 
/// @param filename 
/// @return content类型
static int set_content_type_from_file(httpd_req_t *req, const char *filename)
{
    if (IS_FILE_EXT(filename, ".pdf")) 
    {
        httpd_resp_set_type(req, "application/pdf");
        return 1;
    } else if (IS_FILE_EXT(filename, ".html")) 
    {
        httpd_resp_set_type(req, "text/html");
        return 1;
    } else if (IS_FILE_EXT(filename, ".jpeg")) 
    {
        httpd_resp_set_type(req, "image/jpeg");
        return 0;
    } else if (IS_FILE_EXT(filename, ".png")) 
    {
        httpd_resp_set_type(req, "image/png");
        return 0;
    } else if (IS_FILE_EXT(filename, ".ico")) 
    {
        httpd_resp_set_type(req, "image/x-icon");
        return 0;
    } else if (IS_FILE_EXT(filename, ".css")) 
    {
        httpd_resp_set_type(req, "text/css");
        return 1;
    } else if (IS_FILE_EXT(filename, ".js")) 
    {
        httpd_resp_set_type(req, "application/javascript");
        return 1;
    } 
    /* This is a limited set only */
    /* For any other type always set as plain text */
    httpd_resp_set_type(req, "text/plain");
    return 1;
}

/// @brief 根请求响应处理
/// @param req 
/// @return esp_err_t
esp_err_t html_handler(httpd_req_t *req) 
{
    FILE *html_file = fopen("/blog/public/index.html", "r");
    if (html_file == NULL) {
        ESP_LOGE(TAG, "Failed to open HTML file");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char line[256];
    while (fgets(line, sizeof(line), html_file)) {
        httpd_resp_sendstr_chunk(req, line);
    }

    fclose(html_file);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/// @brief 网站图片请求处理
/// @param req 
/// @param file_path 
static void http_images_resp_process(httpd_req_t *req, char* file_path)
{
    // 从 SD 卡中读取 JPEG 图片数据
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open image file");
        return;
    }
    ESP_LOGI(TAG, "open source file: %s", file_path);

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 分配缓冲区并读取文件内容
    char *image_data = (char *)malloc(file_size); //最大150KB空间
    if (image_data == NULL) {
        fclose(file);
        ESP_LOGE(TAG, "Failed to allocate memory:%d", file_size);
        return;
    }
    fread(image_data, file_size, file_size, file);
    fclose(file);

    // 设置响应类型为 image/jpeg
    httpd_resp_set_type(req, "image/png");

    // 发送 JPEG 图片数据作为响应
    httpd_resp_send(req, image_data, file_size);

    // 释放分配的内存
    free(image_data);
}

/// @brief 网站资源请求处理
/// @param req 
/// @param file_path 
static void http_resource_resp_process(httpd_req_t *req, char* file_path)
{
    // 从 SD 卡中读取文件资源
    FILE *file = fopen(file_path, "r");
    if (file == NULL) 
    {
        ESP_LOGE(TAG, "Failed to open resource file: %s", file_path);
        httpd_resp_send_404(req);
        return;
    }
    ESP_LOGI(TAG, "open source file: %s", file_path);

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(file_size >= 120000)
    {
        // 读取网页资源文件内容并发送给客户端
        char line[8192];
        while (fgets(line, sizeof(line), file)) 
        {
            httpd_resp_sendstr_chunk(req, line);
        }

        fclose(file);
        httpd_resp_send_chunk(req, NULL, 0);
    }
    else
    {
        // 分配缓冲区并读取文件内容
        char *image_data = (char *)malloc(file_size); //最大120KB空间
        if (image_data == NULL) {
            fclose(file);
            ESP_LOGE(TAG, "Failed to allocate memory:[%d]%s", file_size, file_path);
            return;
        }
        fread(image_data, file_size, file_size, file);
        fclose(file);

        // 发送resource数据作为响应
        httpd_resp_send(req, image_data, file_size);

        // 释放分配的内存
        free(image_data);
    }
    
}

/// @brief HTTP请求响应处理
/// @param req 
/// @return esp_err_t
esp_err_t http_handler(httpd_req_t *req) 
{
    // 打开SD卡中的网页资源
    const char *uri = req->uri;
    char file_path[8192];
    char* decoded_url = url_decode(uri);

    snprintf(file_path, sizeof(file_path), "/blog/public%s", decoded_url);

    //获取文件信息查询文件类型
    if(filepath_is_directory(file_path))
    {
        DIR *dir = opendir(file_path);
        if (!dir) {
            ESP_LOGE(TAG, "[2]Failed to stat dir : %s", file_path);
            /* Respond with 404 Not Found */
            httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Directory does not exist");
            return ESP_FAIL;
        }
        closedir(dir);

        strcat(file_path, "index.html");
    }

    //设置网络请求资源类型
    if(set_content_type_from_file(req, file_path))
    {
        http_resource_resp_process(req, file_path);
    }
    else
    {
        http_images_resp_process(req, file_path);
    }

    return ESP_OK;
}

/// @brief 控制台请求响应
/// @param req 
/// @return esp_err_t
esp_err_t http_dashboard_handler(httpd_req_t *req) 
{
    extern const unsigned char dashboard_start[] asm("_binary_bloxer_dashboard_html_start");
    extern const unsigned char dashboard_end[]   asm("_binary_bloxer_dashboard_html_end");
    const size_t dashboard_size = (dashboard_end - dashboard_start);

    httpd_resp_send_chunk(req, (const char *)dashboard_start, dashboard_size);

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/// @brief 控制台导航栏图标请求响应
/// @param req 
/// @return 
esp_err_t http_favicon_handler(httpd_req_t *req) 
{
    extern const unsigned char favicon_start[] asm("_binary_bloxer_favicon_ico_start");
    extern const unsigned char favicon_end[]   asm("_binary_bloxer_favicon_ico_end");
    const size_t favicon_size = (favicon_end - favicon_start);

    httpd_resp_send_chunk(req, (const char *)favicon_start, favicon_size);

    httpd_resp_set_type(req, "image/x-icon");

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t http_dashboard_style_handler(httpd_req_t *req) 
{
    extern const unsigned char dashboard_style_start[] asm("_binary_bloxer_dashboard_css_start");
    extern const unsigned char dashboard_style_end[]   asm("_binary_bloxer_dashboard_css_end");
    const size_t dashboard_style_size = (dashboard_style_end - dashboard_style_start);

    httpd_resp_set_type(req, "text/css");

    httpd_resp_send_chunk(req, (const char *)dashboard_style_start, dashboard_style_size);
    return ESP_OK;
}

static esp_err_t http_script_handler(httpd_req_t *req) 
{
    extern const unsigned char script_start[] asm("_binary_bloxer_script_js_start");
    extern const unsigned char script_end[]   asm("_binary_bloxer_script_js_end");
    const size_t script_size = (script_end - script_start);

    httpd_resp_set_type(req, "application/javascript");

    httpd_resp_send_chunk(req, (const char *)script_start, script_size);
    return ESP_OK;
}

static esp_err_t http_setting_script_handler(httpd_req_t *req) 
{
    extern const unsigned char setting_script_start[] asm("_binary_bloxer_dashboard_setting_js_start");
    extern const unsigned char setting_script_end[]   asm("_binary_bloxer_dashboard_setting_js_end");
    const size_t setting_script_size = (setting_script_end - setting_script_start);

    httpd_resp_set_type(req, "application/javascript");

    httpd_resp_send_chunk(req, (const char *)setting_script_start, setting_script_size);
    return ESP_OK;
}

static esp_err_t http_json_handler(httpd_req_t *req) 
{
    const char *uri = req->uri;
    char file_path[8192];
    char* decoded_url = url_decode(uri);

    snprintf(file_path, sizeof(file_path), "/blog/%s", decoded_url);

    httpd_resp_set_type(req, "application/json");
    http_resource_resp_process(req, file_path);

    return ESP_OK;
}

static esp_err_t http_setting_handler(httpd_req_t *req) 
{
    extern const unsigned char dashboard_setting_start[] asm("_binary_bloxer_dashboard_setting_html_start");
    extern const unsigned char dashboard_setting_end[]   asm("_binary_bloxer_dashboard_setting_html_end");
    const size_t dashboard_setting_size = (dashboard_setting_end - dashboard_setting_start);

    httpd_resp_set_type(req, "text/html");

    httpd_resp_send_chunk(req, (const char *)dashboard_setting_start, dashboard_setting_size);
    return ESP_OK;
}

/// @brief 控制台资源混合请求
/// @param req 
/// @return 
esp_err_t http_dashboard_multi_handler(httpd_req_t *req) 
{
    char fileNames[50] = {0};

    char *fileNameStart = strrchr(req->uri, '/');
    if (fileNameStart != NULL) 
    {
        fileNameStart++; // 跳过 '/'
        strcpy(fileNames, fileNameStart);
    } 
    else 
    {
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Open multiple resource %s", fileNames);
    if (strcmp(fileNames, "bloxer_dashboard.css") == 0) 
    {
        http_dashboard_style_handler(req);
    } 
    else if (strcmp(fileNames, "bloxer_script.js") == 0) 
    {
        http_script_handler(req);
    } 
    else if (strcmp(fileNames, "bloxer_dashboard_setting.js") == 0)
    {
        http_setting_script_handler(req);
    } 
    else if (strcmp(fileNames, "bloxer_config.json") == 0) 
    {
        http_json_handler(req);
    } 
    else if (strcmp(fileNames, "bloxer_dashboard_setting.html") == 0) 
    {
        http_setting_handler(req);
    } 
    else 
    {
        ESP_LOGE(TAG, "Invalid File\n");
    }

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/// @brief 解析参数段中的参数名和参数值
/// @param param 参数段
/// @param name 参数名缓冲区
/// @param value 参数值缓冲区
static void parse_param(const char *param, char *name, char *value) {
    const char *equal_sign = strchr(param, '=');
    if (equal_sign != NULL) 
    {
        // 复制参数名
        strncpy(name, param, equal_sign - param);
        name[equal_sign - param] = '\0'; // 添加字符串结束符
        // 复制参数值
        strcpy(value, equal_sign + 1);
    } 
    else 
    {
        return;
    }
}

/// @brief 解析Url链接中的参数
/// @param url url链接字符串
/// @param params 参数
/// @param count 参数数量
static void parse_url_params(const char *url, url_Param *params, int *count) 
{
    *count = 0; // 初始化参数计数器为0
    ESP_LOGI(TAG, "URL:%s", url);
    char* url_parm = strstr(url, "?");
    if (url_parm == NULL) {
        ESP_LOGE(TAG, "No parameters found in URL");
        return;
    }
    url_parm++;

    char *token = strtok(url_parm, "&"); // 按照 '&' 分割 URL 参数

    while (token != NULL && *count < MAX_URL_PARAMS) 
    {
        parse_param(token, params[*count].name, params[*count].value);
        (*count)++;
        token = strtok(NULL, "&"); // 继续解析下一个参数
    }
}

/// @brief http修改设备参数配置请求
/// @param req 
/// @return 错误码
esp_err_t http_modify_handler(httpd_req_t *req) 
{
    url_Param params[MAX_URL_PARAMS] = {0};
    int count;
    int i=0;

    const char* url = url_decode(req->uri);
    parse_url_params(url, params, &count);

    for(i=0; i<count; i++)
    {
        ESP_LOGI(TAG, "Name:%s,Value:%s", params[i].name, params[i].value);
        modify_json_attribute("/blog/bloxer_config.json", params[i].name, params[i].value);
    }

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/// @brief 删除Blog文件
/// @param req 
/// @return 错误码
esp_err_t http_delete_local_blog_handler(httpd_req_t *req) 
{
    delete_directory_recursive("/blog/public");

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}


/// @brief 更新Blog文件
/// @param req 
/// @return 错误码
esp_err_t http_updata_github_blog_handler(httpd_req_t *req) 
{
    get_github_file_json_index();

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}