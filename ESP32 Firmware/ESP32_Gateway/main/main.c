#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#define UART_PORT UART_NUM_2
#define UART_TX GPIO_NUM_17
#define UART_RX GPIO_NUM_16

#define WIFI_SSID      "KOLLONS"
#define WIFI_PASSWORD  "agnes3277"

#define THINGSPEAK_API_KEY "P80C6WZHP4TPKVGC"


static const char *TAG = "TinyML_Gateway";

#define BUF_SIZE 256

#define BUF_SIZE 256

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }

    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        printf("WiFi Disconnected...Reconnecting\n");
        esp_wifi_connect();
    }

    else if (event_base == IP_EVENT &&
             event_id == IP_EVENT_STA_GOT_IP)
    {
        printf("WiFi Connected\n");
    }
}

void wifi_init(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL));

    wifi_config_t wifi_config =
    {
        .sta =
        {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(
            WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config));

    ESP_ERROR_CHECK(
        esp_wifi_start());

    printf("Connecting WiFi...\n");
}

void upload_to_thingspeak(float temp,
                          float pressure,
                          int gas,
                          int vibration,
                          float probability,
                          int anomaly)
{
    char url[256];

    snprintf(url,
             sizeof(url),
             "http://api.thingspeak.com/update?api_key=%s"
             "&field1=%.2f"
             "&field2=%.2f"
             "&field3=%d"
             "&field4=%d"
             "&field5=%.6f"
             "&field6=%d",
             THINGSPEAK_API_KEY,
             temp,
             pressure,
             gas,
             vibration,
             probability,
             anomaly);

    esp_http_client_config_t config =
    {
        .url = url,
        .method = HTTP_METHOD_GET,
    };

    esp_http_client_handle_t client =
        esp_http_client_init(&config);

    esp_err_t err =
        esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        printf("ThingSpeak Upload Success\n");
    }
    else
    {
        printf("ThingSpeak Upload Failed\n");
    }

    esp_http_client_cleanup(client);
}

typedef struct
{
    float temperature;
    float pressure;
    int gas;
    int vibration;
    float probability;
    int anomaly;

} ReportData_t;

QueueHandle_t reportQueue;

void uart_task(void *arg)
{
    uint8_t data[BUF_SIZE];

    TickType_t last_upload = 0;
    while (1)
    {
        int len = uart_read_bytes(
            UART_PORT,
            data,
            BUF_SIZE - 1,
            pdMS_TO_TICKS(1000));

        if (len > 0)
        {
            data[len] = '\0';

            char *line = strtok((char *)data, "\r\n");

            while (line != NULL)
            {
                float temp, pressure, probability;
                int gas, vibration, anomaly;

                if (sscanf(line,
                           "%f,%f,%d,%d,%f,%d",
                           &temp,
                           &pressure,
                           &gas,
                           &vibration,
                           &probability,
                           &anomaly) == 6)
                {
                    ReportData_t report;

                    report.temperature = temp;
                    report.pressure = pressure;
                    report.gas = gas;
                    report.vibration = vibration;
                    report.probability = probability;
                    report.anomaly = anomaly;

                    xQueueSend(reportQueue, &report, portMAX_DELAY);

                    if ((xTaskGetTickCount() - last_upload) > pdMS_TO_TICKS(20000))
                        {
                            last_upload = xTaskGetTickCount();

                            upload_to_thingspeak(
                                temp,
                                pressure,
                                gas,
                                vibration,
                                probability,
                                anomaly);
                        }
                }

                line = strtok(NULL, "\r\n");
            }
        }
    }
}

void send_to_thingspeak(ReportData_t *report)
{
    char post_data[256];

    sprintf(post_data,
            "api_key=%s&field1=%.2f&field2=%.2f&field3=%d&field4=%d&field5=%.6f&field6=%d",
            THINGSPEAK_API_KEY,
            report->temperature,
            report->pressure,
            report->gas,
            report->vibration,
            report->probability,
            report->anomaly);

    esp_http_client_config_t config =
    {
        .url = "http://api.thingspeak.com/update",
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);

    esp_http_client_set_header(client,
                               "Content-Type",
                               "application/x-www-form-urlencoded");

    esp_http_client_set_post_field(client,
                                   post_data,
                                   strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        printf("ThingSpeak Upload OK\n");
    }
    else
    {
        printf("ThingSpeak Upload Failed\n");
    }

    esp_http_client_cleanup(client);
}

void thingspeak_task(void *arg)
{
    ReportData_t report;

    while (1)
    {
        if (xQueueReceive(reportQueue,
                          &report,
                          portMAX_DELAY) == pdTRUE)
        {
            printf("\n========== SENSOR REPORT ==========\n");
            printf("Temperature : %.2f C\n", report.temperature);
            printf("Pressure    : %.2f hPa\n", report.pressure);
            printf("Gas         : %d\n", report.gas);
            printf("Vibration   : %d\n", report.vibration);
            printf("Probability : %.6f\n", report.probability);
            printf("Anomaly     : %d\n", report.anomaly);
            printf("===================================\n");

            send_to_thingspeak(&report);

            vTaskDelay(pdMS_TO_TICKS(15000));
        }
    }
}

void app_main(void)
{

    wifi_init();

    reportQueue = xQueueCreate(10, sizeof(ReportData_t));

    if (reportQueue == NULL)
    {
        printf("Queue creation failed!\n");
        while (1);
    }

    uart_config_t uart_config =
    {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_driver_install(
            UART_PORT,
            BUF_SIZE * 2,
            0,
            0,
            NULL,
            0);

    uart_param_config(
            UART_PORT,
            &uart_config);

    uart_set_pin(
            UART_PORT,
            UART_TX,
            UART_RX,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE);

    printf("\nUART Receiver Started\n");

    xTaskCreate(
        uart_task,
        "UART Task",
        4096,
        NULL,
        5,
        NULL);

    xTaskCreate(
        thingspeak_task,
        "ThingSpeak Task",
        4096,
        NULL,
        4,
        NULL);
}