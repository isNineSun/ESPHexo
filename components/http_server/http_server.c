#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include "esp_http_server.h"

#include "http_handler.h"

/* Initialize HTTP server */
httpd_handle_t start_webserver(void) 
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.stack_size = 25600;
    config.uri_match_fn = httpd_uri_match_wildcard;

    // Start the httpd server
    if (httpd_start(&server, &config) == ESP_OK) 
    {
        // Set URI handlers
        httpd_uri_t uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = html_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &uri);

        httpd_uri_t dashboard_uri = {
            .uri       = "/Dashboard",
            .method    = HTTP_GET,
            .handler   = http_dashboard_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &dashboard_uri);

        httpd_uri_t favicon_uri = {
            .uri       = "/favicon.ico",
            .method    = HTTP_GET,
            .handler   = http_favicon_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &favicon_uri);

        httpd_uri_t modify_uri = {
            .uri       = "/bloxer_modify*",
            .method    = HTTP_GET,
            .handler   = http_modify_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &modify_uri);

        httpd_uri_t dashboard_style_uri = {
            .uri       = "/bloxer_*",
            .method    = HTTP_GET,
            .handler   = http_dashboard_multi_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &dashboard_style_uri);

        httpd_uri_t upload_uri = {
            .uri       = "/upload/",
            .method    = HTTP_POST,
            .handler   = http_updata_github_blog_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &upload_uri);


        httpd_uri_t default_uri = {
            .uri       = "/*",
            .method    = HTTP_GET,
            .handler   = http_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &default_uri);

    }
    return server;
}

/* Stop HTTP server */
void stop_webserver(httpd_handle_t server) {
    httpd_stop(server);
}