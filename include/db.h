#ifndef TODO
#define TODO

#include <sqlite3.h>

int init_db();
void seed_db();
void close_db();
int delete_todo_by_id(int id);
int json_todo_by_id(int socket);
int get_todos_in_json(int socket);
int insert_task(const char* text);
int update_todo_by_id(int id, char* text, int done);

#endif // !TODO