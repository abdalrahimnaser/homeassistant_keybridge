#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "usb/usb_host.h"
#include "usb/hid_host.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

static const char *TAG = "hid_min";



static esp_mqtt_client_handle_t mqtt_client = NULL;
static TimerHandle_t publish_timer = NULL;
static int message_count = 0;


// Simple MQTT event handler
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        break;
    }
}

static void mqtt_app_start(void)
{
    // Simple MQTT client configuration
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://192.168.1.13:1883",
        .credentials.username = "mqtt-user",
        .credentials.authentication.password = "123456789",
    };

    #if CONFIG_BROKER_URL_FROM_STDIN
        char line[128];

        if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
            int count = 0;
            printf("Please enter url of mqtt broker\n");
            while (count < 128) {
                int c = fgetc(stdin);
                if (c == '\n') {
                    line[count] = '\0';
                    break;
                } else if (c > 0 && c < 127) {
                    line[count] = c;
                    ++count;
                }
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            mqtt_cfg.broker.address.uri = line;
            printf("Broker url: %s\n", line);
        } else {
            ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
            abort();
        }
    #endif /* CONFIG_BROKER_URL_FROM_STDIN */

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}




static void usb_host_task(void *arg)
{
    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    ESP_ERROR_CHECK(usb_host_install(&host_config));
    ESP_LOGI(TAG, "USB Host installed");

    while (true) {
        uint32_t event_flags = 0;
        usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            ESP_ERROR_CHECK(usb_host_device_free_all());
        }
    }
}

// static void hid_interface_cb(hid_host_device_handle_t hid_device_handle,
//                              const hid_host_interface_event_t event,
//                              void *arg)
// {
//     switch (event) {
//     case HID_HOST_INTERFACE_EVENT_INPUT_REPORT: {
//         uint8_t buf[64];
//         size_t len = 0;
//         char message[64];

//         if (hid_host_device_get_raw_input_report_data(hid_device_handle, buf, sizeof(buf), &len) == ESP_OK) {
//             // Intentionally do nothing with the received keys for now
//             // esp_mqtt_client_publish(mqtt_client, "/esp32/messages", ????); ????
            
//         }

//         break;
//     }
//     case HID_HOST_INTERFACE_EVENT_DISCONNECTED:
//         (void)hid_host_device_close(hid_device_handle);
//         ESP_LOGI(TAG, "HID device disconnected");
//         break;
//     default:
//         break;
//     }
// }


static char keycode_to_ascii(uint8_t modifier, uint8_t keycode)
{
    // Check if Shift is pressed (left or right)
    bool shift_pressed = (modifier & 0x02) || (modifier & 0x20);

    // Map letters
    if (keycode >= 0x04 && keycode <= 0x1D) {
        return shift_pressed ? ('A' + (keycode - 0x04)) : ('a' + (keycode - 0x04));
    }

    // Map numbers, symbols, and numpad keys
    switch (keycode) {
        // Top Row Numbers & Symbols
        case 0x1E: return shift_pressed ? '!' : '1';
        case 0x1F: return shift_pressed ? '@' : '2';
        case 0x20: return shift_pressed ? '#' : '3';
        case 0x21: return shift_pressed ? '$' : '4';
        case 0x22: return shift_pressed ? '%' : '5';
        case 0x23: return shift_pressed ? '^' : '6';
        case 0x24: return shift_pressed ? '&' : '7';
        case 0x25: return shift_pressed ? '*' : '8';
        case 0x26: return shift_pressed ? '(' : '9';
        case 0x27: return shift_pressed ? ')' : '0';

        // Punctuation & Other Keys
        case 0x2C: return ' ';  // Spacebar
        case 0x2D: return shift_pressed ? '_' : '-';
        case 0x2E: return shift_pressed ? '+' : '=';
        case 0x33: return shift_pressed ? ':' : ';';
        case 0x34: return shift_pressed ? '"' : '\'';
        case 0x36: return shift_pressed ? '<' : ',';
        case 0x37: return shift_pressed ? '>' : '.';
        case 0x38: return shift_pressed ? '?' : '/';
        
        // --- ADDED SECTION FOR NUMPAD ---
        case 0x59: return '1';
        case 0x5A: return '2';
        case 0x5B: return '3';
        case 0x5C: return '4';
        case 0x5D: return '5';
        case 0x5E: return '6';
        case 0x5F: return '7';
        case 0x60: return '8';
        case 0x61: return '9';
        case 0x62: return '0';
        case 0x55: return '*';
        case 0x56: return '-';
        case 0x57: return '+';
        case 0x63: return '.';

        default: return 0; // Not a printable character
    }
}




static void hid_interface_cb(hid_host_device_handle_t hid_device_handle,
                             const hid_host_interface_event_t event,
                             void *arg)
{
    switch (event) {
    case HID_HOST_INTERFACE_EVENT_INPUT_REPORT: {
        // A standard keyboard report is 8 bytes
        uint8_t buf[8];
        size_t len = 0;

        if (hid_host_device_get_raw_input_report_data(hid_device_handle, buf, sizeof(buf), &len) == ESP_OK) {
            
            // We need a static variable to track the last key to avoid repeats when holding a key down.
            static uint8_t last_keycode = 0;

            // Assuming a standard 8-byte report, keycode is in buf[2]
            uint8_t modifier = buf[0];
            uint8_t keycode = buf[2];

            // 1. Process only new key presses (keycode is not 0 and is different from the last one)
            // 2. A keycode of 0 means all keys are released.
            if (keycode != 0 && keycode != last_keycode) {
                
                // Convert the scan code to ASCII
                char ascii_char = keycode_to_ascii(modifier, keycode);

                // If it's a valid printable character, publish it
                if (ascii_char != 0) {
                    // Create a null-terminated string payload
                    char payload[2] = {ascii_char, '\0'};

                    ESP_LOGI(TAG, "Key Pressed: '%c', publishing to MQTT...", ascii_char);
                    
                    // Publish the single character. Length is 1.
                    // NOTE: Assumes 'mqtt_client' is a valid, initialized, and accessible client handle.
                    esp_mqtt_client_publish(mqtt_client,
                                            "/esp32/messages",
                                            payload,
                                            1,  // Data length is 1 for a single char
                                            1,  // QoS level 1
                                            0); // Retain flag 0
                }
            }
            // Update the last keycode state for the next report
            last_keycode = keycode;
        }
        break;
    }
    case HID_HOST_INTERFACE_EVENT_DISCONNECTED:
        (void)hid_host_device_close(hid_device_handle);
        ESP_LOGI(TAG, "HID device disconnected");
        break;
    default:
        break;
    }
}










static void hid_device_cb(hid_host_device_handle_t hid_device_handle,
                          const hid_host_driver_event_t event,
                          void *arg)
{
    if (event == HID_HOST_DRIVER_EVENT_CONNECTED) {
        hid_host_dev_params_t params;
        ESP_ERROR_CHECK(hid_host_device_get_params(hid_device_handle, &params));

        const hid_host_device_config_t dev_cfg = {
            .callback = hid_interface_cb,
            .callback_arg = NULL,
        };
        ESP_ERROR_CHECK(hid_host_device_open(hid_device_handle, &dev_cfg));

        if (params.sub_class == HID_SUBCLASS_BOOT_INTERFACE) {
            (void)hid_class_request_set_protocol(hid_device_handle, HID_REPORT_PROTOCOL_BOOT);
            if (params.proto == HID_PROTOCOL_KEYBOARD) {
                (void)hid_class_request_set_idle(hid_device_handle, 0, 0);
            }
        }

        ESP_ERROR_CHECK(hid_host_device_start(hid_device_handle));
        ESP_LOGI(TAG, "HID device started (proto=%d)", params.proto);
    }
}

void app_main(void)
{

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());


    
    // Start USB host task
    xTaskCreatePinnedToCore(usb_host_task, "usb_host", 4096, xTaskGetCurrentTaskHandle(), 2, NULL, 0);

    // Wait for notification from usb_host_task to proceed
    ulTaskNotifyTake(false, 1000);

    // Install HID host (with its internal background task)
    const hid_host_driver_config_t hid_cfg = {
        .create_background_task = true,
        .task_priority = 5,
        .stack_size = 4096,
        .core_id = 0,
        .callback = hid_device_cb,
        .callback_arg = NULL,
    };
    ESP_ERROR_CHECK(hid_host_install(&hid_cfg));
    ESP_LOGI(TAG, "HID Host installed. Connect a USB keyboard.");

    // Wait for WiFi to connect
    vTaskDelay(pdMS_TO_TICKS(5000));
    

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    // Start the simple MQTT application
    mqtt_app_start();

}


