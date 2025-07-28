#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <liburing.h>

int send2serv(struct io_uring *ring, int sockfd, const char *buf, size_t len) {
    size_t sent = 0;

    char *buff_copy = malloc(len);
    memcpy(buff_copy, buf, len);

    while (sent < len) {
        struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
        if (!sqe) {
            perror("io_uring_get_sqe for send");
            break; // Exit loop if we can't get an SQE
        }
        
        io_uring_prep_send(sqe, sockfd, buf + sent, len - sent, 0);
        io_uring_submit(ring);

        struct io_uring_cqe *cqe;
        io_uring_wait_cqe(ring, &cqe);
        int ret = cqe->res;
        io_uring_cqe_seen(ring, cqe);

        if (ret <= 0) return ret;
        sent += ret;
    }
    free(buff_copy);
    return sent;
}