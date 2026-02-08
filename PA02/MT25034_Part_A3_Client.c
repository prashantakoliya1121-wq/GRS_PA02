#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/errqueue.h>
#include <time.h>
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
    const char *host;
    int port;
    size_t msg_size;
    int duration;
} thread_args_t;

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

static void fill_message_fields(Message *msg, const size_t sizes[8]) {
    memset(msg->field1, 'A', sizes[0]);
    memset(msg->field2, 'B', sizes[1]);
    memset(msg->field3, 'C', sizes[2]);
    memset(msg->field4, 'D', sizes[3]);
    memset(msg->field5, 'E', sizes[4]);
    memset(msg->field6, 'F', sizes[5]);
    memset(msg->field7, 'G', sizes[6]);
    memset(msg->field8, 'H', sizes[7]);
}

void *send_messages(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(args->port);
    if (inet_pton(AF_INET, args->host, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock);
        return NULL;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        close(sock);
        return NULL;
    }

    size_t sizes[8];
    compute_field_sizes(args->msg_size, sizes);

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

    time_t end_time = time(NULL) + args->duration;
    while (time(NULL) < end_time) {
        fill_message_fields(&msg, sizes);

        if (sendmsg(sock, &msg_hdr, MSG_ZEROCOPY) < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (recvmsg(sock, &msg_hdr, 0) < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
    }

    close(sock);
    free_message(&msg);
    return NULL;
}

int main(int argc, char *argv[]) {
    const char *host = "127.0.0.1";
    int port = PORT;
    int threads = 1;
    size_t msg_size = 128;
    int duration = 10;

    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = atoi(argv[2]);
    }
    if (argc > 3) {
        threads = atoi(argv[3]);
    }
    if (argc > 4) {
        msg_size = (size_t)atoi(argv[4]);
    }
    if (argc > 5) {
        duration = atoi(argv[5]);
    }

    pthread_t *thread_ids = malloc(sizeof(pthread_t) * (size_t)threads);
    thread_args_t *args = malloc(sizeof(thread_args_t) * (size_t)threads);
    if (!thread_ids || !args) {
        perror("malloc failed");
        free(thread_ids);
        free(args);
        return -1;
    }

    for (int i = 0; i < threads; i++) {
        args[i].host = host;
        args[i].port = port;
        args[i].msg_size = msg_size;
        args[i].duration = duration;
        pthread_create(&thread_ids[i], NULL, send_messages, &args[i]);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(thread_ids);
    free(args);
    return 0;
}