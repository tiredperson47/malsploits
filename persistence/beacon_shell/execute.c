#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "include/commands.h"

#define BUFFER_SIZE 8192
#define GREEN "\e[1;32m"
#define CYAN "\e[1;36m"
#define RED "\e[1;31m"
#define RESET "\033[0m"

char *shell(const char *command) {
    FILE *pipe = popen(command, "r");
    if (!pipe) return NULL;
    char buf[BUFFER_SIZE];

    char *result = NULL;
    size_t size = 0;
    
    while (fgets(buf, sizeof(buf), pipe) != NULL) {
        size_t len = strlen(buf);
        char *temp = realloc(result, size + len + 1);
        if (!temp) {
            free(result);
            pclose(pipe);
            return NULL;
        }
        result = temp;
        strcpy(result + size, buf);
        size += len;
    }

    pclose(pipe);
    return result;
}

char *execute(const char *binary) {
    char *output = (RED "Execute is in dev\n" RESET);
    return output;
}