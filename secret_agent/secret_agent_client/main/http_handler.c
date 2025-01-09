#include "http_handler.h"

static const char *TAG = "HTTP_HANDLER";

esp_err_t http_event_handler(esp_http_client_event_t *evt){

    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id){

        case: HTTP_EVENT_ERROR:
            ESP_LOGW(TAG, "HTTP_EVENT_ERROR");
            break;





    }





}