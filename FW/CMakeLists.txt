idf_component_register(SRCS "firmware_main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES usb mqtt esp_wifi nvs_flash esp_netif esp_event esp_system freertos esp_timer protocol_examples_common
                       )
