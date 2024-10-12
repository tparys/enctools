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
#include <sqlite3.h>

#define PORT 8888

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
    sqlite3 *db = (sqlite3*)cls;

    // Prepare SQL query
    const char *sql = "SELECT tile_data FROM tiles WHERE tile_column = ? AND tile_row = ? and zoom_level = ?;";
    sqlite3_stmt *res = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &res, 0))
    {
	const char *msg = "Failed to prepare SQL query";
	return request_reply(connection, MHD_HTTP_INTERNAL_SERVER_ERROR,
			     msg, strlen(msg));
    }

    // Bind values to SQL statement
    sqlite3_bind_int(res, 1, x);
    sqlite3_bind_int(res, 2, y);
    sqlite3_bind_int(res, 3, z);

    // Check for response
    MHD_Result rc;
    if (sqlite3_step(res) == SQLITE_ROW)
    {
	// Get pointer, size
	const void *blob_ptr = sqlite3_column_blob(res, 0);
	int blob_size = sqlite3_column_bytes(res, 0);
	rc = request_reply(connection, MHD_HTTP_OK,
			   blob_ptr, blob_size);
    }
    else
    {
	// Not found
	const char *msg = "Tile not found";
	rc = request_reply(connection, MHD_HTTP_NOT_FOUND,
			   msg, strlen(msg));
    }

    // Cleanup and return
    sqlite3_finalize(res);
    return rc;
}

int main(int argc, char **argv)
{
    // Open SQLite DB
    sqlite3 *db = nullptr;
    if (sqlite3_open_v2(argv[1], &db,
			SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX, nullptr))
    {
	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	sqlite3_close(db);
	return 1;
    }

    // Start MHD
    MHD_Daemon *daemon = MHD_start_daemon(MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD,
					  PORT, NULL, NULL,
					  &request_handler, db, MHD_OPTION_END);
    if (daemon == nullptr)
    {
	sqlite3_close(db);
	return 1;
    }

    // Wait for input
    (void) getchar ();

    // Cleanup
    MHD_stop_daemon (daemon);
    sqlite3_close(db);
    return 0;
}
