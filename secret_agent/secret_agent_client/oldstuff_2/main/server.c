#include "server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "printer_helper.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/err.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "nvs_flash.h"
#include "string.h"

static void server_task(void *p)
{
    server_init_param_t *param = (server_init_param_t *)p;
    mbedtls_net_context listen_fd, client_fd;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config config;
    mbedtls_x509_crt server_cert;
    mbedtls_pk_context pkey;

    int status;

    nvs_handle_t nvs_handle;

    char cert_buf[4096];
    size_t cert_len;
    char key_buf[4096];
    size_t key_len;

    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;

    PRINTFC_SERVER("Waiting for wifi and ip");
    xEventGroupWaitBits(param->wifi_event_group, BIT0 | BIT1, pdFALSE, pdTRUE, portMAX_DELAY);

    PRINTFC_SERVER("Server is starting");
    mbedtls_net_init(&listen_fd);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&config);
    mbedtls_x509_crt_init(&server_cert);
    mbedtls_pk_init(&pkey);

    PRINTFC_SERVER("Server is loading certificate from nvs");
    esp_err_t err = nvs_open_from_partition(END_OF_LINE_PARTITION, CERTIFICATE_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
    {
        PRINTFC_SERVER("Error when open eol cert %x", err);
        vTaskDelete(NULL);
        return;
    }

    err = nvs_get_str(nvs_handle, CERTIFICATE_NAME, cert_buf, &cert_len);
    if (err != ESP_OK)
    {
        PRINTFC_SERVER("Error when open certificate %x", err);
        vTaskDelete(NULL);
        return;
    }

    PRINTFC_SERVER("Parsing certificate");
    status = mbedtls_x509_crt_parse(&server_cert, (const unsigned char *)cert_buf, cert_len);
    if (status != 0)
    {
        PRINTFC_SERVER("Error when parsing certificate %x", status);
        vTaskDelete(NULL);
        return;
    }

    PRINTFC_SERVER("Opening private key");

    err = nvs_get_str(nvs_handle, KEY_NAME, key_buf, &key_len);
    if (err != ESP_OK)
    {
        PRINTFC_SERVER("Error when open key %x", err);
        vTaskDelete(NULL);
        return;
    }
    nvs_close(nvs_handle);

    PRINTFC_SERVER("Seeding RNG...");
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    const char *seed = "ssl_server";
    status = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)seed, strlen(seed));
    if (status != 0)
    {
        PRINTFC_SERVER("Failed to seed RNG: %d", status);
        vTaskDelete(NULL);
        return;
    }

    mbedtls_ssl_conf_rng(&config, mbedtls_ctr_drbg_random, &ctr_drbg);

    PRINTFC_SERVER("Parsing server private key...");
    const unsigned char password[] = CONFIG_KEY_PASSWORD;   // lägg in i kconfig
    status = mbedtls_pk_parse_key(&pkey, (unsigned char *)key_buf, key_len, password, sizeof(password), mbedtls_ctr_drbg_random, &ctr_drbg); // Use a valid RNG function
    if (status != 0)
    {
        PRINTFC_SERVER("Failed to load server private key: %d", status);
        vTaskDelete(NULL);
        return;
    }

    PRINTFC_SERVER("Setting up SSL configuration...");
    status = mbedtls_ssl_config_defaults(&config, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    if (status != 0)
    {
        PRINTFC_SERVER("Failed to set up SSL configuration: %x", status);
        vTaskDelete(NULL);
        return;
    }

    mbedtls_ssl_conf_authmode(&config, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&config, server_cert.next, NULL);
    status = mbedtls_ssl_conf_own_cert(&config, &server_cert, &pkey);
    if (status != 0)
    {
        PRINTFC_SERVER("Failed to set up own certificate: %x", status);
        vTaskDelete(NULL);
        return;
    }

    status = mbedtls_ssl_setup(&ssl, &config);
    if (status != 0)
    {
        char error_buf[100];
        mbedtls_strerror(status, error_buf, 100);
        PRINTFC_SERVER("Failed to set up SSL: %s", error_buf);
        vTaskDelete(NULL);
        return;
    }

    PRINTFC_SERVER("Binding to the port...");
    status = mbedtls_net_bind(&listen_fd, NULL, PORT, MBEDTLS_NET_PROTO_TCP);
    if (status != 0)
    {
        PRINTFC_SERVER("Failed to bind to the port: %d", status);
        vTaskDelete(NULL);
        return;
    }

    while (1)
    {
        PRINTFC_SERVER("Waiting for incoming connections...");
        status = mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL);
        if (status != 0)
        {
            PRINTFC_SERVER("Failed to accept client connection: %d", status);
            continue; // Continue to accept other connections
        }

        PRINTFC_SERVER("Client connected, performing SSL handshake....");
        mbedtls_ssl_init(&ssl);
        status = mbedtls_ssl_setup(&ssl, &config);
        if (status != 0) 
        {
            PRINTFC_SERVER("Failed to set up SSL context: %d", status);
            mbedtls_net_free(&client_fd);
            continue; // Continue accepting new clients.
        } 

        PRINTFC_SERVER("Setting Basic I/O for SSL/TLS...");
        mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

        PRINTFC_SERVER("Performing SSL handshake...");
        status = mbedtls_ssl_handshake(&ssl);
        if (status != 0)
        {
            PRINTFC_SERVER("SSL handshake failed: %d", status);
            mbedtls_ssl_free(&ssl);
            mbedtls_net_free(&client_fd);
            continue; 
        }

        while (1) 
        {
            // Exchange messages (rps)
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
        PRINTFC_SERVER("Cleaning up SSL session and client connection.");
        mbedtls_ssl_close_notify(&ssl);
        mbedtls_ssl_free(&ssl);
        mbedtls_net_free(&client_fd);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);
}

void server_start(server_init_param_t *param)
{
    if (xTaskCreate(server_task, "server task", 16384, param, 5, NULL) != pdPASS)
    {
        PRINTFC_SERVER("Failed to create server task");
    }
}