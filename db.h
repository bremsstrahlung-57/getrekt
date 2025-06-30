#ifndef TODO
#define TODO

#include <sqlite3.h>

void seed_db();
int init_db();
void insert_task(const char *text);
int get_last_id();
void get_todos_in_json(int socket);

#endif // !TODO