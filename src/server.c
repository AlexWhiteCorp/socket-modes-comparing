#include <printf.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "consts.h"
#include "server_thread.c"

int main(int argc, char *argv[]) {
    struct Configs configs = getConfigs(argc, argv);

    int serverSock, connId = 0;

    serverSock = getSocket(configs);

    printf("### Start Server ###\n");
    bindSocket(serverSock, configs);

    for (;;) {
        printf("[Thread main] Waiting for a connection...\n");

        int clientSock = acceptConnection(serverSock);
        printf("[Thread main] Connected\n");

        switchBlockingType(clientSock, configs);

        struct Connection *conn = malloc(sizeof(struct Connection));
        conn->id = ++connId;
        conn->socket = clientSock;
        conn->configs = configs;
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, server_thread, conn);
    }

    return 0;
}