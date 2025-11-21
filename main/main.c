#include <stdio.h>
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log_level.h"
#include "hal/gpio_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_http_server.h>
#include <esp_spiffs.h>

#include "wifi_credentials.h"

const char *ssid = WIFI_SSID;
const char *pass = WIFI_PASS;

#define LED_GPIO_PIN GPIO_NUM_4

httpd_handle_t start_webserver(void);



esp_err_t index_handler(httpd_req_t *req)
{
    // Cesta k souboru na SPIFFS
    const char *file_path = "/spiffs/index.html";
    
    // Otevření souboru pro čtení
    FILE *f = fopen(file_path, "r");
    if (f == NULL) {
        // Pokud se soubor nepodaří otevřít (neexistuje, nebo SPIFFS neni mountnuto)
        httpd_resp_send_500(req);
        printf("Chyba: Nepodařilo se otevřít %s\n", file_path);
        return ESP_FAIL;
    }

    // Získání velikosti souboru
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Alokace bufferu pro celý obsah souboru + 1 pro '\0'
    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        fclose(f);
        httpd_resp_send_500(req);
        printf("Chyba: Nedostatek paměti pro alokaci bufferu.\n");
        return ESP_FAIL;
    }

    // Přečtení obsahu souboru do bufferu
    size_t read_bytes = fread(buffer, 1, file_size, f);
    fclose(f);
    buffer[read_bytes] = '\0'; // Ukončení řetězce

    // Nastavení Content-Type hlavičky
    httpd_resp_set_type(req, "text/html");

    // Odeslání obsahu souboru
    httpd_resp_send(req, buffer, read_bytes); 
    
    // Uvolnění alokované paměti
    free(buffer);

    return ESP_OK;
}
esp_err_t css_handler(httpd_req_t *req)
{
    // Cesta k souboru na SPIFFS
    const char *file_path = "/spiffs/styling.css";
    
    // Otevření souboru pro čtení
    FILE *f = fopen(file_path, "r");
    if (f == NULL) {
        // Pokud se soubor nepodaří otevřít (neexistuje, nebo SPIFFS neni mountnuto)
        httpd_resp_send_500(req);
        printf("Chyba: Nepodařilo se otevřít %s\n", file_path);
        return ESP_FAIL;
    }

    // Získání velikosti souboru
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Alokace bufferu pro celý obsah souboru + 1 pro '\0'
    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        fclose(f);
        httpd_resp_send_500(req);
        printf("Chyba: Nedostatek paměti pro alokaci bufferu.\n");
        return ESP_FAIL;
    }

    // Přečtení obsahu souboru do bufferu
    size_t read_bytes = fread(buffer, 1, file_size, f);
    fclose(f);
    buffer[read_bytes] = '\0'; // Ukončení řetězce

    // Nastavení Content-Type hlavičky
    httpd_resp_set_type(req, "text/css");

    // Odeslání obsahu souboru
    httpd_resp_send(req, buffer, read_bytes); 
    
    // Uvolnění alokované paměti
    free(buffer);

    return ESP_OK;
}
esp_err_t js_handler(httpd_req_t *req)
{
    // Cesta k souboru na SPIFFS
    const char *file_path = "/spiffs/index.js";
    
    // Otevření souboru pro čtení
    FILE *f = fopen(file_path, "r");
    if (f == NULL) {
        // Pokud se soubor nepodaří otevřít (neexistuje, nebo SPIFFS neni mountnuto)
        httpd_resp_send_500(req);
        printf("Chyba: Nepodařilo se otevřít %s\n", file_path);
        return ESP_FAIL;
    }

    // Získání velikosti souboru
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Alokace bufferu pro celý obsah souboru + 1 pro '\0'
    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        fclose(f);
        httpd_resp_send_500(req);
        printf("Chyba: Nedostatek paměti pro alokaci bufferu.\n");
        return ESP_FAIL;
    }

    // Přečtení obsahu souboru do bufferu
    size_t read_bytes = fread(buffer, 1, file_size, f);
    fclose(f);
    buffer[read_bytes] = '\0'; // Ukončení řetězce

    // Nastavení Content-Type hlavičky
    httpd_resp_set_type(req, "application/javascript");

    // Odeslání obsahu souboru
    httpd_resp_send(req, buffer, read_bytes); 
    
    // Uvolnění alokované paměti
    free(buffer);

    return ESP_OK;
}
// Handler for turning the LED ON (e.g., accessed via HTTP POST /gpio/on)
esp_err_t gpio_on_handler(httpd_req_t *req)
{
    gpio_set_level(LED_GPIO_PIN, 0); 
    printf("LED/GPIO ON by web request.\n");

    httpd_resp_set_status(req, "204 No Content");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}


// Handler for turning the LED OFF (e.g., accessed via HTTP POST /gpio/off)
esp_err_t gpio_off_handler(httpd_req_t *req)
{
    gpio_set_level(LED_GPIO_PIN, 1); 
    printf("LED/GPIO OFF by web request.\n");

    httpd_resp_set_status(req, "204 No Content");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}


// ===============================================
// 2. HTTP SERVER STARTUP AND REGISTRATION
// ===============================================

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // URI Handler Registration
    httpd_uri_t uri_on = {
        .uri       = "/gpio/on",
        .method    = HTTP_POST,
        .handler   = gpio_on_handler,
        .user_ctx  = NULL
    };
    httpd_uri_t uri_off = {
        .uri       = "/gpio/off",
        .method    = HTTP_POST,
        .handler   = gpio_off_handler,
        .user_ctx  = NULL
    };
    httpd_uri_t uri_main = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL
    };
    httpd_uri_t uri_css = {
        .uri       = "/styling.css",
        .method    = HTTP_GET,
        .handler   = css_handler,
        .user_ctx  = NULL
    };
    httpd_uri_t uri_js = {
        .uri       = "/index.js",
        .method    = HTTP_GET,
        .handler   = js_handler,
        .user_ctx  = NULL
    };

    if (httpd_start(&server, &config) == ESP_OK) {
        printf("HTTP Server started on port: %d\n", config.server_port);
        
        httpd_register_uri_handler(server, &uri_on);
        httpd_register_uri_handler(server, &uri_off);
        httpd_register_uri_handler(server, &uri_main);
        httpd_register_uri_handler(server, &uri_js);
        httpd_register_uri_handler(server, &uri_css);
    }
    return server;
}

// =============================================s==
// 3. EVENT HANDLER AND WIFI/GPIO SETUP
// ===============================================


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // Start connection attempt after Wi-Fi stack initialization
        esp_wifi_connect();
        printf("Wi-Fi station started. Connecting...\n");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        // Try to reconnect upon disconnection
        printf("Disconnected from Wi-Fi. Retrying to connect...\n");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        // Got IP address = successful connection
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("Successfully connected to Wi-Fi!\n");
        printf("Got IP address: %d.%d.%d.%d\n", IP2STR(&event->ip_info.ip));
        
        // Start the web server after obtaining an IP address
        esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
printf("SPIFFS mount: %s\n", esp_err_to_name(ret));

size_t total = 0, used = 0;
esp_spiffs_info(NULL, &total, &used);

printf("SPIFFS total = %d bytes, used = %d bytes\n", total, used);

FILE *f = fopen("/spiffs/index.html", "r");
printf("open result: %p\n", f);

    

    if(ret != ESP_OK)
    {
        if(ret == ESP_FAIL)
        {
        printf("failed to mount or format file system\n");
        }
        else if (ret == ESP_ERR_NOT_FOUND) 
        {
        printf("failed to find spiffs partition\n");
        }
        else 
        {
        printf("failed to init spiffs (%s)\n",esp_err_to_name(ret));
        }
        return;
    }else{
        printf("asi dobry\n");
    }


        start_webserver();
    }
}

void app_main(void)
{
    // 1. Initialize NVS (Non-Volatile Storage)
    // NVS is required to store Wi-Fi and other configuration data.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize TCP/IP stack and Event Group
    ESP_ERROR_CHECK(esp_netif_init()); // Initializes the TCP/IP stack
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Creates the default event loop

    // 3. Create Wi-Fi Station (STA)
    esp_netif_create_default_wifi_sta();

    // 4. Initialize Wi-Fi driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 5. Register event handler
    // This function will call 'event_handler' upon events like WIFI_EVENT_STA_START and IP_EVENT_STA_GOT_IP.
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    // 6. Set Wi-Fi configuration (SSID and password)
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
            // Optional: You can add .listen_interval, .pmf_cfg, etc., here.
            .threshold.authmode = WIFI_AUTH_WPA2_PSK, // Set minimum authentication mode
        },
    };
    // Copy SSID and password from const variables to the config structure
    memcpy(wifi_config.sta.ssid, ssid, strlen(ssid));
    memcpy(wifi_config.sta.password, pass, strlen(pass));


    // 7. Set Wi-Fi mode to STA and apply configuration
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // 8. Start Wi-Fi and connect
    ESP_ERROR_CHECK(esp_wifi_start());
    // esp_wifi_connect() is called inside the event_handler after start.
    
    // Initialize GPIO pin for output
    gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_OUTPUT);

   
}