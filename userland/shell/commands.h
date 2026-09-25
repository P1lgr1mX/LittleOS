#ifndef COMMANDS_H
#define COMMANDS_H

typedef int (*command_func_t)(int argc, char **argv);

struct builtin_cmd {
    const char *name;
    const char *description;
    command_func_t func;
};

int cmd_help(int argc, char **argv);
int cmd_clear(int argc, char **argv);
int cmd_echo(int argc, char **argv);
int cmd_about(int argc, char **argv);

extern const struct builtin_cmd builtin_commands[];
extern const int num_builtin_commands;

#endif /* COMMANDS_H */
