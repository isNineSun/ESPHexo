idf_component_register(SRCS "github_file_get.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_client sd_card json_parse url_decode oled)
