idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                REQUIRES 
                "esp_wifi" 
                "esp_driver_gpio" 
                "nvs_sec_provider"
                "nvs_flash"
                "esp_http_server"
                "spiffs"
                "vfs"
            )


                    