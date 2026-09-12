#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/shell.h"
#include "../include/input.h"
#include "../include/parser.h"
#include "../include/process.h"
#include "../include/builtin.h"

int main() {
    char *line;
    char **tokens;

    printf("\n");
    printf("%s Version %s\n", SHELL_NAME, VERSION);
    printf("\n");

    while (1) {
        printf("myshell> ");
        line = read_line();

        if (strlen(line) != 0) {
            tokens = parse_line(line);
            if (execute_builtin(tokens) == 0) {
                execute(tokens);
            }
            free_tokens(tokens);
        }

        free(line);
    }

    printf("Goodbye!\n");
 
   return 0;
}
