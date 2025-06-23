#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "Server.c"

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
        else
        {
            send_response(new_socket, "404 - Doesn't Exist");
        }

        //} char *response = "HTTP/1.1 200 OK\r\nDate: Fri, 20 Jun 2025 17:10:33 GMT\r\nServer: Custom-C-Server/1.0 (Linux)\r\nContent-Length: 13\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nHello, World!";
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