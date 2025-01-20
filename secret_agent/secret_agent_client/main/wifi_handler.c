#include "wifi_handler.h"
#include "printer_helper.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include "lwip/err.h"
#include "lwip/sys.h"

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
   
    wifi_init_param_t *param = (wifi_init_param_t *)arg;
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        PRINTFC_WIFI_HANDLER("WiFi started with SSID: %s and PASS: %s", \
         ((wifi_init_param_t *)param)->ssid, \
         ((wifi_init_param_t *)param)->password);        
       esp_err_t err =  esp_wifi_connect();
         if(err != ESP_OK)
         {
             PRINTFC_WIFI_HANDLER("Failed to connect to WiFi");         
         }
         else
         {
             PRINTFC_WIFI_HANDLER("Connecting to WiFi");
         }
        break;
        
        PRINTFC_WIFI_HANDLER("WiFi disconnected, retrying");
       // xEventGroupClearBits(param->wifi_event_group, WIFI_CONNECTED_BIT);
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_wifi_connect();     
        break;
    case WIFI_EVENT_STA_CONNECTED:
        PRINTFC_WIFI_HANDLER("WiFi connected");
        xEventGroupSetBits(param->wifi_event_group, WIFI_CONNECTED_BIT);
        break;

    case IP_EVENT_STA_GOT_IP:
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        char ip_str[16];
        esp_ip4addr_ntoa(&event->ip_info.ip, ip_str, sizeof(ip_str));
        PRINTFC_WIFI_HANDLER("Got IP: %s", ip_str);
        xEventGroupSetBits(param->wifi_event_group, WIFI_HAS_IP_BIT);
        break;

    default:
        PRINTFC_WIFI_HANDLER("Unknown event: %ld", event_id);
        break;
    }
   
}

void wifi_init_start(wifi_init_param_t *param)
{


    PRINTFC_WIFI_HANDLER("Using ssid: %s%s%s", green, param->ssid, reset);  // varför är dom här här?
    PRINTFC_WIFI_HANDLER("Using password: %s%s%s", green, param->password, reset); /// ?

    PRINTFC_WIFI_HANDLER("Init network interface");
    ESP_ERROR_CHECK(esp_netif_init());

    esp_netif_t *netif = esp_netif_create_default_wifi_sta();
    assert(netif != NULL); //Varför är du här? viktigt?

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, param, NULL);
    PRINTFC_WIFI_HANDLER("Wifi event handler registered");
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, param, NULL));
    PRINTFC_WIFI_HANDLER("IP event handler registered");

    wifi_config_t wifi_config = {
        .sta = {
            .pmf_cfg = {
                .required = false,  
            },
        },
    };
    
    memcpy(wifi_config.sta.ssid, param->ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, param->password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    PRINTFC_WIFI_HANDLER("Wifi is starting");
}