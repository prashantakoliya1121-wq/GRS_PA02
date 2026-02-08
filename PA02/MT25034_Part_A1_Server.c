#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sys/uio.h>

#define PORT 8082
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

static int send_all(int sock, const void *buf, size_t len) {
    const char *p = (const char *)buf;
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(sock, p + sent, len - sent, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

static int recv_all(int sock, void *buf, size_t len) {
    char *p = (char *)buf;
    size_t recvd = 0;
    while (recvd < len) {
        ssize_t n = recv(sock, p + recvd, len - recvd, 0);
        if (n <= 0) {
            return -1;
        }
        recvd += (size_t)n;
    }
    return 0;
}

void *handle_client(void *arg) {
    client_args_t *cargs = (client_args_t *)arg;
    int sock = cargs->sock;
    size_t msg_size = cargs->msg_size;
    free(cargs);

    size_t sizes[8];
    compute_field_sizes(msg_size, sizes);

    Message msg;
    char *buffer = malloc(msg_size);
    if (allocate_message(&msg, sizes) < 0 || !buffer) {
        perror("malloc failed");
        close(sock);
        free_message(&msg);
        free(buffer);
        return NULL;
    }

    while (1) {
        if (recv_all(sock, buffer, msg_size) < 0) {
            break;
        }

        size_t offset = 0;
        memcpy(msg.field1, buffer + offset, sizes[0]); offset += sizes[0];
        memcpy(msg.field2, buffer + offset, sizes[1]); offset += sizes[1];
        memcpy(msg.field3, buffer + offset, sizes[2]); offset += sizes[2];
        memcpy(msg.field4, buffer + offset, sizes[3]); offset += sizes[3];
        memcpy(msg.field5, buffer + offset, sizes[4]); offset += sizes[4];
        memcpy(msg.field6, buffer + offset, sizes[5]); offset += sizes[5];
        memcpy(msg.field7, buffer + offset, sizes[6]); offset += sizes[6];
        memcpy(msg.field8, buffer + offset, sizes[7]);

        if (send_all(sock, buffer, msg_size) < 0) {
            break;
        }

    }

    free(buffer);
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
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
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