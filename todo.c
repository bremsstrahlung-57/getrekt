#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "Server.h"
#include "response.h"
#include "todo.h"

void get_todos(int socket, Todo *todos, int num_todos)
{
    char list_of_tasks[26400];
    int current_len = 0;
    current_len += snprintf(list_of_tasks + current_len, sizeof(list_of_tasks) - current_len, "[");

    for (int i = 0; i < num_todos; i++)
    {
        if (i > 0)
        {
            current_len += snprintf(list_of_tasks + current_len, sizeof(list_of_tasks) - current_len, ",");
        }
        const char *done_str = todos[i].done ? "true" : "false";
        current_len += snprintf(list_of_tasks + current_len, sizeof(list_of_tasks) - current_len,
                                "{\"id\": %d, \"task\": \"%s\", \"complete\": %s}",
                                todos[i].id, todos[i].text, done_str);

        if (current_len >= sizeof(list_of_tasks) - 100)
        {
            fprintf(stderr, "Warning: Approaching buffer limit in get_todos. Truncating.\n");
            break;
        }
    }
    current_len += snprintf(list_of_tasks + current_len, sizeof(list_of_tasks) - current_len, "]");
    send_response(socket, list_of_tasks);
}

//TODO: POST Request
// void post_todo(int socket, Todo *todos, char *buffer)
// {
//     char *todo;
//     sscanf("{\"text\": \"%s\"}", todo);
// }