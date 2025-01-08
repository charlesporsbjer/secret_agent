static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    static char response_buffer[2048];
    static int response_offset = 0;

    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (evt->data_len + response_offset < sizeof(response_buffer)) {
                memcpy(response_buffer + response_offset, evt->data, evt->data_len);
                response_offset += evt->data_len;
            }
            break;

        case HTTP_EVENT_ON_FINISH:
            response_buffer[response_offset] = '\0';
            ESP_LOGI(TAG, "Full response:\n%s", response_buffer);

            cJSON *json = cJSON_Parse(response_buffer);
            if (json == NULL) {
                if (strstr(response_buffer, "-----BEGIN CERTIFICATE-----")) {