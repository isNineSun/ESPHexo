idf_component_register(SRCS "oled.c"
                    INCLUDE_DIRS "include"
                    REQUIRES ssd1306 driver DDNS wifi sd_card github_file_get)
