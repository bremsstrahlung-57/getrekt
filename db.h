#ifndef TODO
#define TODO

#include <sqlite3.h>

int init_db();
void seed_db();
void close_db();
int get_last_id();
int get_todos_in_json(int socket);
int insert_task(const char *text);
int json_todo_by_id(int socket, int id);
int delete_todo_by_id(int id, int socket);
int update_todo_by_id(int id, char *text, int done);

#endif // !TODO