/* Feel free to use this example code in any way
   you see fit (Public Domain) */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <encviz/enc_renderer.h>

#define PORT 8888

// FIXME - Sloppy global
encviz::render_style style = {
    // Background color
    {{ 255, 255, 255 }},

    // Layer styles
    {
        {
            { "LNDARE",
              { 0, 192, 0 },
              { 0, 255, 0 },
              1,
              5
            },
            { "SLCONS",
              { 0, 0, 0 },
              { 0, 0, 0 },
              1,
              5
            },
            { "DEPCNT",
              {},
              { 128, 128, 128 },
              1,
              0
            },
            { "SOUNDG",
              {},
              {},
              0,
              0
            }
        }
    }
};

MHD_Result request_reply(MHD_Connection *conn, int code, const void *data, int len)
{
    MHD_Response *resp = MHD_create_response_from_buffer(len, (void*)data, MHD_RESPMEM_MUST_COPY);
    MHD_Result ret = MHD_queue_response(conn, code, resp);
    MHD_destroy_response(resp);
    printf(" - HTTP %d\n", code);
    return ret;
}

MHD_Result request_handler(void *cls, struct MHD_Connection *connection,
			   const char *url, const char *method,
			   const char *version, const char *upload_data,
			   size_t *upload_data_size, void **req_cls)
{
    // Parse URL
    printf("URL: %s\n", url);
    int x, y, z;
    if (sscanf(url, "/%d/%d/%d", &z, &y, &x) != 3)
    {
	const char *msg = "Invalid URL";
	return request_reply(connection, MHD_HTTP_BAD_REQUEST,
			     msg, strlen(msg));
    }

    // WMS queries use TMS tile scheme, where MBTiles uses XYZ, with inverted Y axis
    int ntiles = (int)pow(2, z);
    y = ntiles - y - 1;

    // Get passed SQLite DB
    encviz::enc_renderer *enc_rend = (encviz::enc_renderer*)cls;

    // Render requested tile
    std::vector<uint8_t> out_bytes;
    printf("Tile X=%d, Y=%d, Z=%d\n", x, y, z);
    enc_rend->render(out_bytes, x, y, z, style);

    // Respond
    return request_reply(connection, MHD_HTTP_OK,
                         out_bytes.data(), out_bytes.size());
}

int main(int argc, char **argv)
{
    // Check args
    if (argc < 2)
    {
        printf("Usage: enc_index path/to/ENC_ROOT [x y z]\n");
        return 1;
    }

    // Global GDAL Initialization
    GDALAllRegister();

    // ENC renderer context
    encviz::enc_renderer enc_rend(256, 1e8);
    enc_rend.load_charts(argv[1]);

    // Start MHD
    MHD_Daemon *daemon = MHD_start_daemon(MHD_USE_AUTO | MHD_USE_THREAD_PER_CONNECTION,
					  PORT, NULL, NULL,
					  &request_handler, &enc_rend, MHD_OPTION_END);
    if (daemon == nullptr)
    {
	return 1;
    }

    // Wait for input
    (void)getchar();

    // Cleanup
    MHD_stop_daemon (daemon);
    return 0;
}
