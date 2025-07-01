#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sqlite3.h>

#include "todo.h"
#include "db.h"

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

void json_todo_by_id(int socket, int id)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT text, done FROM todos WHERE id = ?;";
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

    rc = sqlite3_bind_int(stmt, 1, id);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind id: %s\n", sqlite3_errmsg(DB));
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        const char *text = (const char *)sqlite3_column_text(stmt, 0);
        int done = sqlite3_column_int(stmt, 1);
        char json_response[1024];
        sprintf(json_response,
                "{\"id\":%d,\"task\":\"%s\",\"done\":%s}",
                id,
                text,
                done ? "true" : "false");
        send_response(socket, json_response);
    }
    else if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "No data found for ID (%d): %s\n", id, sqlite3_errmsg(DB));
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(stderr, "Error stepping: %s\n", sqlite3_errmsg(DB));
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(DB);
}

void get_task_from_id(int id, char *dest)
{
    sqlite3_stmt *select_stmt;
    const char *sql_select = "SELECT text FROM todos WHERE id = ?;";
    int rc;

    rc = sqlite3_prepare_v2(DB, sql_select, -1, &select_stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare select statement: %s\n", sqlite3_errmsg(DB));
        sqlite3_finalize(select_stmt);
        sqlite3_close(DB);
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_bind_int(select_stmt, 1, id);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind id for select: %s\n",
                sqlite3_errmsg(DB));
        sqlite3_finalize(select_stmt);
        sqlite3_close(DB);
        exit(EXIT_FAILURE);
    }
    rc = sqlite3_step(select_stmt);

    if (rc == SQLITE_ROW) {
        const char *text = (const char *)sqlite3_column_text(select_stmt, 0);
        if (text) {
            strcpy(dest, text);
        } else {
            dest[0] = '\0';
        }
    } else {
        dest[0] = '\0';
    }

    sqlite3_finalize(select_stmt);
}

void get_todos_in_json(int socket)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, text, done FROM todos;";
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

    char json_response[8192] = "[\n";
    int delim = 1;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char *text = sqlite3_column_text(stmt, 1);
        int done = sqlite3_column_int(stmt, 2);
        char item[512];

        sprintf(item,
                "{\"id\":%d,\"task\":\"%s\",\"done\":%s}",
                id,
                text,
                done ? "true" : "false");

        if (!delim)
            strcat(json_response, ",");
        delim = 0;

        strcat(json_response, item);
        if (rc == SQLITE_DONE)
        {
            fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(DB));
            break;
        }
    }

    strcat(json_response, "\n]\n");
    send_response(socket, json_response);

    sqlite3_finalize(stmt);
    sqlite3_close(DB);
}

void update_todo_by_id(int id, char *text, int done)
{
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE todos SET text = ?, done = ? WHERE id = ?;";
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

    if (text[0] == '\0' || text == NULL)
    {
        get_task_from_id(id, text);
    }

    rc = sqlite3_bind_text(stmt, 1, text, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind text: %s\n", sqlite3_errmsg(DB));
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_bind_int(stmt, 2, done);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind done: %s\n", sqlite3_errmsg(DB));
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        exit(EXIT_FAILURE);
    }

    rc = sqlite3_bind_int(stmt, 3, id);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to bind id: %s\n", sqlite3_errmsg(DB));
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
    text[0] = '\0';
}

void delete_todo_by_id(int id)
{
    return;
}