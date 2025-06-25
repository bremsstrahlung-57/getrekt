#ifndef RESPONSE_H
#define RESPONSE_H

#include "Server.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void send_response(int socket, char *data);
void serve_file(int socket, int status, const char *status_text, const char *content_type, const char *body, size_t body_size);
const char *get_mime_type(const char *path);
void parse_path(const char *request, char *path, size_t path_size);
char *read_file(const char *filename, size_t *file_size);
void send_404(int socket);
void launch(struct Server *server);

#endif