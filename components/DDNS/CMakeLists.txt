idf_component_register(SRCS "DDNS.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi esp_http_client mbedtls esp-tls wifi oled json_parse)
