#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sqlite3.h>

#include "todo.h"

sqlite3 *DB;
char *DB_FILE = "data.db";

void init_db()
{
    int rc = sqlite3_open(DB_FILE, &DB);
    if (rc == SQLITE_OK)
    {
        printf("Database opened successfully.\n");
    }
    else
    {
        fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(DB));
    }
}

void seed_db()
{
    const char *sql =
        "CREATE TABLE IF NOT EXISTS todos ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "text TEXT NOT NULL,"
        "done INTEGER NOT NULL DEFAULT 0"
        ");";

    char *err_msg = 0;
    int rc = sqlite3_exec(DB, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to create table: %s\n", err_msg);
        sqlite3_free(err_msg);
        exit(1);
    }
}

// TODO: Insert Todo to SQLite
void insert_task(const char *text)
{
}