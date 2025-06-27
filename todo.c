#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "Server.h"
#include "response.h"
#include "todo.h"

void get_todos(int socket, Todo *todos, int num_todos)
{
    char respone_body[4096] = "[\n";
    for (int i = 0; i < num_todos; ++i)
    {
        char item[512];
        sprintf(item,
                "{\"id\":%d,\"text\":\"%s\",\"done\":%s}%s",
                todos[i].id,
                todos[i].text,
                todos[i].done ? "true" : "false",
                (i < num_todos - 1) ? "," : "");
        strcat(respone_body, item);
    }
    strcat(respone_body, "]\n");
    send_response(socket, respone_body);
}

void post_todo(int socket, Todo *todos, char *buffer)
{
    char *todo;
    int ID;

    todo = strchr(buffer, '{');
    ID = (sizeof(todos) / 264);

    char *start = strchr(todo, ':');
    start = strchr(start, '"');
    start++;
    char *end = strrchr(start, '"');
    size_t len = end - start;

    strncpy(todos[ID].text, start, len);

    todos[ID].text[len] = '\0';
    todos[ID].id = ID;
    todos[ID].done = 0;
}

void update_todo(int socket, Todo *todos, int id)
{
    todos[id].done = 1;
}

void delete_todo(int socket, Todo *todos, int id)
{
    todos[id].id = 0;
    todos[id].done = 0;
    strcpy(todos[id].text, "");
}