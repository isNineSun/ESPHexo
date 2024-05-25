idf_component_register(SRCS "http_server.c" "http_handler.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_server url_decode json_parse sd_card github_file_get
                    EMBED_FILES 
                    "bloxer_dashboard.html"
                    "bloxer_dashboard_setting.html"
                    "bloxer_dashboard.css"
                    "bloxer_favicon.ico"
                    "bloxer_script.js"
                    "bloxer_dashboard_setting.js")
