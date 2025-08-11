#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Server.h"
#include "response.h"
#include "todo.h"

void send_response(int socket, char* data) {
    int content_length = strlen(data);
    char response[2048];

    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        content_length, data);

    write(socket, response, strlen(response));
}

void parse_text(char* source, char* dest) {
    char* text_start = strstr(source, "\"text\":");
    if (!text_start) {
        perror("ERROR!, 'text' field not found.");
        exit(EXIT_FAILURE);
    }
    text_start = strchr(text_start + strlen("\"text\":"), '"');
    if (!text_start) {
        perror("ERROR!, Opening quote for 'text' value not found.");
        exit(EXIT_FAILURE);
    }
    text_start++;
    char* text_end = strchr(text_start, '"');
    if (!text_end) {
        perror("ERROR!, Closing quote for 'text' value not found.");
        exit(EXIT_FAILURE);
    }
    int len = text_end - text_start;
    strncpy(dest, text_start, len);
    dest[len] = '\0';
}

void get_update(char* buffer, char* bufpath, char* dest, int* id, int* done) {
    *id = atoi(bufpath + strlen("/api/todos/"));
    char text[255];

    if (strstr(buffer, "\"text\"") && strstr(buffer, "\"done\"")) {
        parse_text(buffer, text);
        strncpy(dest, text, 255);

        if (strstr(buffer, "\"done\":true")) {
            *done = 1;
        } else if (strstr(buffer, "\"done\":false")) {
            *done = 0;
        }
    }

    else if (strstr(buffer, "\"text\"")) {
        parse_text(buffer, text);
        strncpy(dest, text, 255);
    }

    else if (strstr(buffer, "\"done\"")) {

        if (strstr(buffer, "\"done\":true")) {
            *done = 1;
        }

        else if (strstr(buffer, "\"done\":false")) {
            *done = 0;
        }
    }
}