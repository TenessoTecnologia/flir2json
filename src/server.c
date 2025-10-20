#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>

#define PORT 8080

static int handle_request(void *cls, struct MHD_Connection *connection,
                          const char *url, const char *method,
                          const char *version, const char *upload_data,
                          size_t *upload_data_size, void **con_cls)
{
    const char *response_text = "{\"status\":\"ok\",\"message\":\"FLIR JSON API is running!\"}";
    struct MHD_Response *response;
    int ret;

    response = MHD_create_response_from_buffer(strlen(response_text),
                                               (void *)response_text,
                                               MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, "Content-Type", "application/json");
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

int main()
{
    struct MHD_Daemon *daemon;

    printf("🚀 Starting FLIR JSON API on 0.0.0.0:%d...\n", PORT);

    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD,
                              PORT,
                              NULL, NULL,
                              &handle_request, NULL,
                              MHD_OPTION_END);

    if (daemon == NULL)
    {
        fprintf(stderr, "❌ Failed to start HTTP server.\n");
        return 1;
    }

    // Mantém o servidor ativo
    while (1)
    {
        sleep(60);
    }

    MHD_stop_daemon(daemon);
    return 0;
}
