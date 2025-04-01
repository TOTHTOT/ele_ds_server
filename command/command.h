#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <readline/readline.h>
#include <readline/history.h>

extern char **command_completion(const char *text, int start, int end);
extern char *command_generator(const char *text, int state);
extern void execute_command(char *cmd);

#endif /* __COMMAND_H__ */
