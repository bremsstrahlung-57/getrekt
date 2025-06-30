#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sqlite3.h>

#include "todo.h"

sqlite3 *DB;
char *DB_FILE = "data.db";

int init_db()
{
    int rc = sqlite3_open(DB_FILE, &DB);
    if (rc == SQLITE_OK)
    {
        printf("Database opened successfully.\n");
        return 0;
    }
    else
    {
        fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(DB));
        return 1;
    }
}

void seed_db()
{
    if (init_db() == 1)
    {
        perror("Error initialising Database!");
        exit(EXIT_FAILURE);
    };
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

void insert_task(const char *text)
{
    seed_db();
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO todos (text, done) VALUES (?, 0);";
    int rc;
    sqlite3_open(DB_FILE, &DB);
    rc = sqlite3_prepare_v2(DB, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(DB));
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_bind_text(stmt, 1, text, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind text: %s\n", sqlite3_errmsg(DB));
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(DB));
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(DB);
}

int get_last_id()
{
    return sqlite3_last_insert_rowid(DB);
}