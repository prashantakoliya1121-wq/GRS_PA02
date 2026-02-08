#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/uio.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Message structure with dynamically allocated fields
typedef struct {
    char *field1;
    char *field2;
    char *field3;
    char *field4;
    char *field5;
    char *field6;
    char *field7;
    char *field8;
} Message;

typedef struct {
    int sock;
    size_t msg_size;
} client_args_t;

static void compute_field_sizes(size_t msg_size, size_t sizes[8]) {
    size_t base = msg_size / 8;
    size_t rem = msg_size % 8;
    for (int i = 0; i < 8; i++) {
        sizes[i] = base + (i < (int)rem ? 1 : 0);
    }
}

static int allocate_message(Message *msg, const size_t sizes[8]) {
    msg->field1 = malloc(sizes[0]);
    msg->field2 = malloc(sizes[1]);
    msg->field3 = malloc(sizes[2]);
    msg->field4 = malloc(sizes[3]);
    msg->field5 = malloc(sizes[4]);
    msg->field6 = malloc(sizes[5]);
    msg->field7 = malloc(sizes[6]);
    msg->field8 = malloc(sizes[7]);

    if (!msg->field1 || !msg->field2 || !msg->field3 || !msg->field4 ||
        !msg->field5 || !msg->field6 || !msg->field7 || !msg->field8) {
        return -1;
    }
    return 0;
}

static void free_message(Message *msg) {
    free(msg->field1);
    free(msg->field2);
    free(msg->field3);
    free(msg->field4);
    free(msg->field5);
    free(msg->field6);
    free(msg->field7);
    free(msg->field8);
}

static void setup_iovec(struct iovec iov[8], Message *msg, const size_t sizes[8]) {
    iov[0].iov_base = msg->field1; iov[0].iov_len = sizes[0];
    iov[1].iov_base = msg->field2; iov[1].iov_len = sizes[1];
    iov[2].iov_base = msg->field3; iov[2].iov_len = sizes[2];
    iov[3].iov_base = msg->field4; iov[3].iov_len = sizes[3];
    iov[4].iov_base = msg->field5; iov[4].iov_len = sizes[4];
    iov[5].iov_base = msg->field6; iov[5].iov_len = sizes[5];
    iov[6].iov_base = msg->field7; iov[6].iov_len = sizes[6];
    iov[7].iov_base = msg->field8; iov[7].iov_len = sizes[7];
}

void *handle_client(void *arg) {
    client_args_t *cargs = (client_args_t *)arg;
    int sock = cargs->sock;
    size_t msg_size = cargs->msg_size;
    free(cargs);

    size_t sizes[8];
    compute_field_sizes(msg_size, sizes);

    Message msg;
    if (allocate_message(&msg, sizes) < 0) {
        perror("malloc failed");
        close(sock);
        free_message(&msg);
        return NULL;
    }

    struct msghdr msg_hdr = {0};
    struct iovec iov[8];
    setup_iovec(iov, &msg, sizes);
    msg_hdr.msg_iov = iov;
    msg_hdr.msg_iovlen = 8;

    while (1) {
        if (recvmsg(sock, &msg_hdr, 0) <= 0) {
            break;
        }
        if (sendmsg(sock, &msg_hdr, 0) < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
    }

    free_message(&msg);

    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int port = PORT;
    size_t msg_size = 128;

    if (argc > 1) {
        port = atoi(argv[1]);
    }
    if (argc > 2) {
        msg_size = (size_t)atoi(argv[2]);
    }

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        pthread_t thread_id;
        client_args_t *cargs = malloc(sizeof(client_args_t));
        if (!cargs) {
            perror("malloc failed");
            close(new_socket);
            continue;
        }
        cargs->sock = new_socket;
        cargs->msg_size = msg_size;
        pthread_create(&thread_id, NULL, handle_client, cargs);
        pthread_detach(thread_id);
    }

    return 0;
}