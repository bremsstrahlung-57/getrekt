#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "Server.h"
#include "response.h"
#include "todo.h"

void parse_text(char *source, char *dest)
{
    char *text_start = strstr(source, "\"text\":");
    if (!text_start)
    {
        perror("ERROR!, 'text' field not found.");
        exit(EXIT_FAILURE);
    }
    text_start = strchr(text_start + strlen("\"text\":"), '"');
    if (!text_start)
    {
        perror("ERROR!, Opening quote for 'text' value not found.");
        exit(EXIT_FAILURE);
    }
    text_start++;
    char *text_end = strchr(text_start, '"');
    if (!text_end)
    {
        perror("ERROR!, Closing quote for 'text' value not found.");
        exit(EXIT_FAILURE);
    }
    int len = text_end - text_start;
    strncpy(dest, text_start, len);
    dest[len] = '\0';
}

void get_todos(int socket, Todo *todos, int num_todos)
{
    char respone_body[4096] = "[";

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

void post_todo(int socket, Todo *todos, char *buffer, int *todo_count)
{
    char text[512];
    int next_id;
    parse_text(buffer, text);

    if (*todo_count > 100)
    {
        perror("Too many todos");
        exit(EXIT_FAILURE);
    }
    if (*todo_count == 0)
    {
        next_id = 1;
    }
    else
    {
        next_id = todos[*todo_count - 1].id + 1;
    }
    Todo new_todo;
    new_todo.id = next_id;
    strncpy(new_todo.text, text, 255);
    new_todo.done = 0;

    todos[*todo_count] = new_todo;
    (*todo_count)++;

    char response[1024];
    snprintf(response, sizeof(response),
             "{\"id\": %d, \"text\": \"%s\", \"done\": false}",
             new_todo.id, new_todo.text);
    send_response(socket, response);
}

void update_todo(int socket, Todo *todos, char *buffer, char *bufpath, int todo_count)
{
    int id = atoi(bufpath + strlen("/api/todos/"));
    int index = -1;
    for (int i = 0; i < todo_count; i++)
    {
        if (todos[i].id == id)
        {
            index = i;
            break;
        }
    }
    if (index != -1)
    {
        if (strstr(buffer, "\"text\"") && strstr(buffer, "\"done\""))
        {
            char text[255];
            parse_text(buffer, text);
            strncpy(todos[index].text, text, 255);
            if (strstr(buffer, "\"done\": true"))
            {
                todos[index].done = 1;
            }
            else if (strstr(buffer, "\"done\": false"))
            {
                todos[index].done = 0;
            }
            char response[1024];
            snprintf(response, sizeof(response), "{\"id\": %d, \"text\": \"%s\", \"done\": %s}\n", todos[index].id, todos[index].text, todos[index].done ? "true" : "false");
            send_response(socket, response);
        }
        else if (strstr(buffer, "\"text\""))
        {
            char text[255];
            parse_text(buffer, text);
            strncpy(todos[index].text, text, 255);
            char response[1024];
            snprintf(response, sizeof(response), "{\"id\": %d, \"text\": \"%s\", \"done\": %s}\n", todos[index].id, todos[index].text, todos[index].done ? "true" : "false");
            send_response(socket, response);
        }
        else if (strstr(buffer, "\"done\""))
        {
            if (strstr(buffer, "\"done\": true"))
            {
                todos[index].done = 1;
            }
            else if (strstr(buffer, "\"done\": false"))
            {
                todos[index].done = 0;
            }
            char response[1024];
            snprintf(response, sizeof(response), "{\"id\": %d, \"text\": \"%s\", \"done\": %s}\n", todos[index].id, todos[index].text, todos[index].done ? "true" : "false");
            send_response(socket, response);
        }
        else if (strstr(buffer, "{}"))
        {
            char response[1024];
            snprintf(response, sizeof(response), "{\"id\": %d, \"text\": \"%s\", \"done\": %s}\n", todos[index].id, todos[index].text, todos[index].done ? "true" : "false");
            send_response(socket, response);
        }
    }
    else
    {
        char response[100];
        snprintf(response, sizeof(response), "{\"error\": \"Not found\"}\n");
        send_response(socket, response);
    }
}

void delete_todo(int socket, Todo *todos, char *bufpath, int todo_count)
{
    int id = atoi(bufpath + strlen("/api/todos/"));
    int index = -1;
    for (int i = 0; i < todo_count; i++)
    {
        if (todos[i].id == id)
        {
            index = i;
            break;
        }
    }
    if (index != -1)
    {
        for (int i = index; i < todo_count - 1; i++)
        {
            todos[i] = todos[i + 1];
        }
        todo_count--;
        char response[1024];
        snprintf(response, sizeof(response), "{\"deleted\": %d}\n", id);
        send_response(socket, response);
    }
    else
    {
        char response[1024];
        snprintf(response, sizeof(response), "{\"error\": \"Not found\"}\n");
        send_response(socket, response);
    }
}