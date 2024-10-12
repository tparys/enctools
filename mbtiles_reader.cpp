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
    printf("MBTiles open!\n");

    for (int z = 6; z < 8; z++)
    {
	int ntiles = (int)pow(2, z);
	for (int y = 0; y < ntiles; y++)
	{
	    for (int x = 0; x < ntiles; x++)
	    {

		// Prepare SQL fetch statement
		const char *sql = "SELECT tile_data FROM tiles WHERE tile_column = ? AND tile_row = ? and zoom_level = ?;";
		sqlite3_stmt *res = nullptr;
		if (sqlite3_prepare_v2(db, sql, -1, &res, 0))
		{
		    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
		    sqlite3_close(db);
		    return 1;
		}
		
		// Bind values to SQL statement
		sqlite3_bind_int(res, 1, x);
		sqlite3_bind_int(res, 2, y);
		sqlite3_bind_int(res, 3, z);
		
		// Check for response
		int step = sqlite3_step(res);
		if (step == SQLITE_ROW)
		{
		    // Get pointer, size
		    const void *blob_ptr = sqlite3_column_blob(res, 0);
		    int blob_size = sqlite3_column_bytes(res, 0);
		    printf("Got %p (%d bytes)\n", blob_ptr, blob_size);
		    
		    char outpath[256];
		    sprintf(outpath, "%d_%d_%d.png", z, y, x);
		    FILE *outhandle = fopen(outpath, "wb");
		    fwrite(blob_ptr, 1, blob_size, outhandle);
		    fclose(outhandle);
		}
		else
		{
		    printf("No data returned ...\n");
		}
		
		sqlite3_finalize(res);
	    }
	}
    }

    // Cleanup
    sqlite3_close(db);
    return 0;

    /*

    // Start MicroHTTPD
    MHD_Daemon *daemon = MHD_start_daemon(
	MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD,
	PORT, NULL, NULL,
	&answer_to_connection, NULL, MHD_OPTION_END);
    if (daemon == nullptr)
    {
	perror("Cannot start MicroHTTPD");
	return 1;
    }

    // Wait for user input, cleanup
    (void)getchar ();

    // Cleanup and exit
    
    MHD_stop_daemon (daemon);
    return 0;
    */
}
