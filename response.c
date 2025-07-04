#include "Server.h"
#include "response.h"
#include "todo.h"
#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sqlite3.h>

void serve_file(int socket, int status, const char *status_text, const char *content_type, const char *body, size_t body_size)
{
    char header[1024];
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 %d %s\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %zu\r\n"
                              "Access-Control-Allow-Origin: *\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              status, status_text, content_type, body_size);

    write(socket, header, header_len);
    if (body && body_size > 0)
    {
        write(socket, body, body_size);
    }
}

const char *get_mime_type(const char *path)
{
    const char *ext = strrchr(path, '.');
    if (!ext)
        return "text/plain";

    if (strcmp(ext, ".html") == 0)
        return "text/html";
    if (strcmp(ext, ".css") == 0)
        return "text/css";
    if (strcmp(ext, ".js") == 0)
        return "application/javascript";
    if (strcmp(ext, ".json") == 0)
        return "application/json";
    if (strcmp(ext, ".png") == 0)
        return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(ext, ".ico") == 0)
        return "image/x-icon";

    return "text/plain";
}

void parse_path(const char *request, char *path, size_t path_size)
{
    const char *start = strchr(request, ' ');
    if (!start)
    {
        strcpy(path, "/");
        return;
    }
    start++;
    const char *end = strchr(start, ' ');
    if (!end)
    {
        size_t len = strlen(start);
        if (len >= path_size)
        {
            len = path_size - 1;
        }
        strncpy(path, start, len);
        path[len] = '\0';
        return;
    }
    size_t len = end - start;
    if (len >= path_size)
    {
        len = path_size - 1;
    }
    strncpy(path, start, len);
    path[len] = '\0';
}

char *read_file(const char *filename, size_t *file_size)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
        return NULL;

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(*file_size + 1);
    if (!buffer)
    {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, *file_size, file);
    buffer[*file_size] = '\0';
    fclose(file);

    return buffer;
    free(buffer);
}

void send_404(int socket)
{
    char filepath[512] = "./public/404.html";
    const char *mime_type = get_mime_type(filepath);
    size_t file_size;
    char *file_content = read_file(filepath, &file_size);
    serve_file(socket, 404, "Not Found", mime_type, file_content, file_size);
    free(file_content);
}

void launch(struct Server *server)
{
    char buffer[30000];
    int address_length = sizeof(server->address);
    int new_socket;

    while (1)
    {
        printf("\n==== WAITING FOR CONNECTION ====\n");
        new_socket = accept(server->socket, (struct sockaddr *)&server->address, (socklen_t *)&address_length);

        if (new_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        read(new_socket, buffer, sizeof(buffer) - 1);
        printf("\n%s\n", buffer);

        char method[10], bufpath[64];
        char *start = &buffer[0];
        if (!start)
        {
            send_response(new_socket, "HTTP/1.1 400 Bad Request\r\n\r\n");
            close(new_socket);
            continue;
        }
        char *end = strchr(start, ' ');
        if (!end)
        {
            send_response(new_socket, "HTTP/1.1 400 Bad Request\r\n\r\n");
            close(new_socket);
            continue;
        }

        int len = end - start;
        strncpy(method, start, len);
        method[len] = '\0';
        start = strchr(end, '/');
        if (!start)
        {
            send_response(new_socket, "HTTP/1.1 400 Bad Request\r\n\r\n");
            close(new_socket);
            continue;
        }
        end = strchr(start, ' ');
        if (!end)
        {
            send_response(new_socket, "HTTP/1.1 400 Bad Request\r\n\r\n");
            close(new_socket);
            continue;
        }

        len = end - start;
        strncpy(bufpath, start, len);
        bufpath[len] = '\0';

        printf("[DEBUG]\nMethod: %s\nPath: %s\n", method, bufpath);

        if (strstr(bufpath, "/public"))
        {

            char path[512];
            int path_size = sizeof(path);
            parse_path(buffer, path, path_size);
            printf("Requested path: %s\n", path);

            char filepath[1024] = ".";
            strcat(filepath, path);

            printf("Looking for file: %s\n", filepath);

            size_t file_size;
            char *file_content = read_file(filepath, &file_size);

            if (file_content)
            {
                const char *mime_type = get_mime_type(filepath);
                serve_file(new_socket, 200, "OK", mime_type, file_content, file_size);
                free(file_content);
                printf("Served: %s (%s, %zu bytes)\n", filepath, mime_type, file_size);
            }
            else
            {
                printf("File not found: %s\n", filepath);
                send_404(new_socket);
            }
        }
        else if (strcmp(bufpath, "/") == 0)
        {
            char filepath[512] = "./public/index.html";
            size_t file_size;
            char *file_content = read_file(filepath, &file_size);
            if (file_content)
            {
                const char *mime_type = get_mime_type(filepath);
                serve_file(new_socket, 200, "OK", mime_type, file_content, file_size);
                free(file_content);
                printf("Served: %s (%s, %zu bytes)\n", filepath, mime_type, file_size);
            }
            else
            {
                send_404(new_socket);
                printf("File not found: %s\n", filepath);
            }
        }
        else if (strcmp(method, "OPTIONS") == 0)
        {
            const char *preflight_response =
                "HTTP/1.1 204 No Content\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
                "Access-Control-Allow-Headers: Content-Type\r\n"
                "Content-Length: 0\r\n"
                "Connection: close\r\n"
                "\r\n";
            write(new_socket, preflight_response, strlen(preflight_response));
        }
        else if (strcmp(method, "GET") == 0 && (strcmp(bufpath, "/api/todos") == 0))
        {
            if (get_todos_in_json(new_socket))
            {
                send_response(new_socket, "{\"error\": 404 Not Found}\n");
            };
        }
        else if (strcmp(method, "POST") == 0 && (strcmp(bufpath, "/api/todos") == 0))
        {
            char text[1024];
            parse_text(buffer, text);
            if (insert_task(text))
            {
                send_response(new_socket, "{\"error\": 500 Internal Server Error}\n");
            };
            if (json_todo_by_id(new_socket))
            {
                send_response(new_socket, "{\"error\": 404 Not Found}\n");
            };
        }
        else if (strcmp(method, "DELETE") == 0 && strncmp(bufpath, "/api/todos/", 11) == 0)
        {
            int id = atoi(bufpath + strlen("/api/todos/"));
            if (json_todo_by_id(new_socket))
            {
                send_response(new_socket, "{\"error\": 404 Not Found}\n");
            };
            if (delete_todo_by_id(id))
            {
                send_response(new_socket, "{\"error\": 500 Internal Server Error}\n");
            };
        }
        else if (strcmp(method, "PUT") == 0 && strncmp(bufpath, "/api/todos/", 11) == 0)
        {
            int id;
            int done;
            char text[256];
            get_update(buffer, bufpath, text, &id, &done);
            if (update_todo_by_id(id, text, done))
            {
                send_response(new_socket, "{\"error\": 404 Not Found}\n");
            };
            if (json_todo_by_id(new_socket))
            {
                send_response(new_socket, "{\"error\": 404 Not Found}\n");
            };
        }
        else if (strcmp(method, "POST") == 0 && strcmp(bufpath, "/echo") == 0)
        {

            int content_len;
            sscanf(buffer, "POST /echo HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/8.5.0\r\nAccept: */*\r\nContent-Type: application/json\r\nContent-Length: %d", &content_len);

            const char *content = strchr(buffer, '{');
            char response[100];
            char name[50];
            char *start = strstr(content, "\"name\": \"");
            if (start != NULL)
            {
                start += strlen("\"name\": \"");
            }
            char *end = strchr(start, '"');
            if (end != NULL)
            {
                size_t len = end - start;
                strncpy(name, start, len);
                name[len] = '\0';
            }
            sprintf(response, "{\"echo\": \"%s\"}", name);
            send_response(new_socket, response);
        }
        else
        {
            send_404(new_socket);
        }
        close(new_socket);
    }
};