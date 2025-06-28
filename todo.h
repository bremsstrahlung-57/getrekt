#ifndef TODO_H
#define TODO_H

typedef struct Todo
{
    int id;
    char text[256];
    int done;
}Todo;

void get_todos(int socket, Todo *todos, int num_todos);
void post_todo(int socket, Todo *todos, char *buffer, int todo_count);
void update_todo(int socket, Todo *todos, char *buffer, char *bufpath, int todo_count);
void delete_todo(int socket, Todo *todos, char *bufpath, int todo_count);

#endif