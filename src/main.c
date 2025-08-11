#include "Server.h"
#include "db.h"
#include "response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    seed_db();
    struct Server server = server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8080, 10, launch);
    server.launch(&server);
    close_db();
    return 0;
}