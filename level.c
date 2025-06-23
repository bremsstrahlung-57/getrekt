#include "Server.c"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void send_response(int socket, char *data)
{
    int content_length = strlen(data);
    char response[2048];

    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/plain\r\n"
             "Content-Length: %d\r\n"
             "Access-Control-Allow-Origin: *\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             content_length, data);

    write(socket, response, strlen(response));
}

const char *get_mime_type(const char *path)
{
    const char *ext = strchr(path, '.');
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

        if (strstr(buffer, "GET /status"))
        {
            char *res = "{\"status\": \"running\",\n\"server\": \"C-Server\",\n\"version\": \"1.0\"}";
            send_response(new_socket, res);
        }
        else if (strstr(buffer, "GET /about"))
        {
            char *res = "About Page";
            send_response(new_socket, res);
        }
        else if (strstr(buffer, "GET /home") || strstr(buffer, "GET / "))
        {
            char *res = "Home page";
            send_response(new_socket, res);
        }
        else if (strstr(buffer, "GET /public"))
        {
            char path[256];
            int path_size = sizeof(path);
            parse_path(buffer, path, path_size);
            printf("Requested path: %s\n", path);
        }
        else
        {
            send_response(new_socket, "404 - Doesn't Exist");
        }

        // char *response = "HTTP/1.1 200 OK\r\nDate: Fri, 20 Jun 2025 17:10:33 GMT\r\nServer: Custom-C-Server/1.0 (Linux)\r\nContent-Length: 13\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nHello, World!";
        // write(new_socket, response, strlen(response));

        close(new_socket);
    }
};

int main()
{
    struct Server server = server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8080, 10, launch);
    server.launch(&server);
    return 0;
}