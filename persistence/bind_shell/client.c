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

//Catch Ctrl+C
static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

int main(int argc, char *argv[])
{
    int status, received, client_fd;
    struct sockaddr_in serv_addr;
    char command[BUFFER_SIZE];
    char buffer[BUFFER_SIZE] = { 0 };
    char *ip_addr = argv[1];
    unsigned short PORT = (unsigned short)atoi(argv[2]);

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while(keepRunning) {
        
        printf(CYAN "Malsploits> " RESET);
        if (fgets(command, BUFFER_SIZE, stdin) == NULL) {
            printf("Invalid Command");
        }

        send(client_fd, command, strlen(command), 0);

        memset(buffer, 0, BUFFER_SIZE);
        received = read(client_fd, buffer, BUFFER_SIZE - 1);
        printf("%s", buffer);
    }

    // closing the connected socket
    close(client_fd);
    return 0;
}
