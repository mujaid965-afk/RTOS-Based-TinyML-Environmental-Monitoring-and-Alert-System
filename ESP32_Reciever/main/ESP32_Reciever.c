#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

// --- HARDWARE UART REGISTER CONFIGURATION ---
#define UART_NUM            UART_NUM_2
#define TXD_PIN             (GPIO_NUM_17)
#define RXD_PIN             (GPIO_NUM_16)
#define RX_BUF_SIZE         (1024)

// --- NETWORK GATEWAY WIRELESS PROFILES ---
#define WIFI_SSID           "KOLLONS"           
#define WIFI_PASS           "agnes3277" // <-- Put your real Wi-Fi password here

// --- THINGSPEAK CLOUD CONFIGURATION ---
#define THINGSPEAK_API_KEY  "P80C6WZHP4TPKVGC" // <-- Put your real ThingSpeak Write API Key here
#define WIFI_CONNECTED_BIT  BIT0

static const char *TAG_UART = "ESP32_ReceiverNode";
static const char *TAG_WIFI = "ESP32_WiFi_Gateway";
static const char *TAG_HTTP = "ESP32_Cloud_Uploader";

static EventGroupHandle_t s_wifi_event_group;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGW(TAG_WIFI, "Connection link dropped. Re-connecting...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG_WIFI, "Handshake complete! Assigned Network IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * @brief Asynchronous cloud worker task utilizing raw LwIP TCP sockets
 */
void upload_to_cloud_task(void *pvParameters)
{
    uint32_t prediction =
        (uint32_t)(uintptr_t)pvParameters;

    ESP_LOGI(
        TAG_HTTP,
        "Uploading prediction=%lu",
        prediction);

    const struct addrinfo hints =
    {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM
    };

    struct addrinfo *res;

    int err =
        getaddrinfo(
            "api.thingspeak.com",
            "80",
            &hints,
            &res);

    if(err != 0 || res == NULL)
    {
        ESP_LOGE(TAG_HTTP,
                 "DNS Failed");

        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG_HTTP,
             "DNS Success");

    int s =
        socket(
            res->ai_family,
            res->ai_socktype,
            0);

    if(s < 0)
    {
        ESP_LOGE(TAG_HTTP,
                 "Socket Failed");

        freeaddrinfo(res);

        vTaskDelete(NULL);
        return;
    }

    if(connect(
           s,
           res->ai_addr,
           res->ai_addrlen) != 0)
    {
        ESP_LOGE(TAG_HTTP,
                 "Connect Failed");

        close(s);
        freeaddrinfo(res);

        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG_HTTP,
             "Connected");

    freeaddrinfo(res);

    char request[512];

    int len =
        snprintf(
            request,
            sizeof(request),
            "GET /update?api_key=%s&field1=%lu HTTP/1.1\r\n"
            "Host: api.thingspeak.com\r\n"
            "Connection: close\r\n\r\n",
            THINGSPEAK_API_KEY,
            prediction);

    write(
        s,
        request,
        len);

    ESP_LOGI(
        TAG_HTTP,
        "HTTP Request Sent");

    close(s);

    vTaskDelete(NULL);
}

void init_uart_bridge(void) 
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_driver_install(UART_NUM, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI(TAG_UART, "UART2 Inter-Node Bridge initialized.");
}

void init_wifi_station(void)
{
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG_WIFI, "Wi-Fi Stack Started.");
}

void esp32_rx_task(void *arg)
{
    uint8_t current_byte = 0;
    uint8_t packet_state = 0;
    uint8_t prediction_payload = 0;

    ESP_LOGI(TAG_UART,
             "State Machine Parser Active");

    while(1)
    {
        int length =
            uart_read_bytes(
                UART_NUM,
                &current_byte,
                1,
                pdMS_TO_TICKS(100));

        if(length > 0)
        {
            switch(packet_state)
            {
                case 0:

                    if(current_byte == 0xAA)
                    {
                        packet_state = 1;
                    }

                    break;

                case 1:

                    prediction_payload = current_byte;
                    packet_state = 2;

                    break;

                case 2:

                    if(current_byte == 0xBB)
                    {
                        ESP_LOGI(
                            TAG_UART,
                            "[VALID FRAME] Prediction=%d",
                            prediction_payload);

                        EventBits_t bits =
                            xEventGroupGetBits(
                                s_wifi_event_group);

                        if(bits &
                           WIFI_CONNECTED_BIT)
                        {
                            static TickType_t last_upload = 0;

                            if((xTaskGetTickCount() - last_upload)
                                    > pdMS_TO_TICKS(15000))
                            {
                                last_upload =
                                    xTaskGetTickCount();
                            xTaskCreate(
                                upload_to_cloud_task,
                                "cloud_tx",
                                4096,
                                (void *)(uintptr_t)
                                prediction_payload,
                                4,
                                NULL);
                            }
                        }
                    }

                    packet_state = 0;

                    break;
            }
        }
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    init_uart_bridge();
    init_wifi_station();

    xTaskCreatePinnedToCore(esp32_rx_task, "esp32_rx_task", 4096, NULL, 5, NULL, 1);
}