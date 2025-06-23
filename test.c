#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "Server.c"

void send_json(int socket, char *json_data)
{
    int content_length = strlen(json_data);
    char response[2048];

    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Date: Fri, 20 Jun 2025 17:10:33 GMT\r\n"
             "Server: Custom-C-Server/1.0 (Linux)\r\n"
             "Content-Length: %d\r\n"
             "Content-Type: application/json\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             content_length, json_data);

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

void send_response(int socket, int status, const char *status_text, const char *content_type, const char *body, size_t body_size)
{
    char header[1024];
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 %d %s\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %zu\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              status, status_text, content_type, body_size);

    write(socket, header, header_len);
    if (body && body_size > 0)
    {
        write(socket, body, body_size);
    }
}

void send_404(int socket)
{
    const char *body = "<html><body><h1>404 not found</h1><body></html>";
    send_response(socket, 404, "Not Found", "text/html", body, strlen(body));
}

void parse_request_path(const char *request, char *path, size_t path_size)
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
        strcpy(path, "/");
        return;
    }

    size_t len = end - start;
    if (len >= path_size)
        len = path_size - 1;

    strncpy(path, start, len);
    path[len] = '\0';
}

void launch(struct Server *server)
{
    char buffer[30000];
    int address_length = sizeof(server->address);
    int new_socket;

    while (1)
    {
        printf("==== WAITING FOR CONNECTION ====\n");
        new_socket = accept(server->socket, (struct sockaddr *)&server->address, (socklen_t *)&address_length);

        if (new_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        read(new_socket, buffer, sizeof(buffer) - 1);
        printf("Request: \n%s\n", buffer);

        char path[256];
        parse_request_path(buffer, path, sizeof(path));
        printf("Requested path: %s\n", path);

        char filepath[512] = "public";
        if (strcmp(path, "/") == 0)
        {
            strcat(filepath, "/index.html");
        }
        else
        {
            strcat(filepath, path);
        }

        printf("Looking for file: %s\n", filepath);

        size_t file_size;
        char *file_content = read_file(filepath, &file_size);

        if (file_content)
        {
            const char *mime_type = get_mime_type(filepath);
            send_response(new_socket, 200, "OK", mime_type, file_content, file_size);
            free(file_content);
            printf("Served: %s (%s, %zu bytes)\n", filepath, mime_type, file_size);
        }
        else
        {
            send_404(new_socket);
            printf("File not found: %s\n", filepath);
        }
        close(new_socket);
    }
}

int main()
{
    struct Server server = server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8080, 10, launch);
    server.launch(&server);
    return 0;
}