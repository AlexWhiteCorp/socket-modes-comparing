#include <unistd.h>
#include "consts.h"
#include "common.c"

void fill_string(char str[], int len) {
    for (int i = 0; i < len; i++) {
        str[i] = 'A' + random() % 26;
    }

    str[len - 1] = (char) 0;
}

int main(int argc, char *argv[]) {
    struct Configs configs = getConfigs(argc, argv);

    int serverSock, t;
    char str[MSG_SIZE];

    serverSock = getSocket(configs);

    printf("Trying to connect...\n");

    long long connStart = mtime();
    connectToServer(serverSock, configs);
    long long connEnd = mtime();

    switchBlockingType(serverSock, configs);

    printf("Connected in %lld ms\n", (connEnd - connStart));

    int msgId = 0;
    for (int i = 0; i < MSG_COUNT; i++) {
        fill_string(str, MSG_SIZE);
        if (send(serverSock, str, strlen(str), 0) == -1) {
            i--;
            continue;
        }

        msgId++;

        if(configs.echo) {
            if ((t = recv(serverSock, str, MSG_SIZE, 0)) > 0) {
                str[t] = '\0';
            }
        }

    }

    printf("Sent %d msgs, %d bytes\n", msgId, msgId * (MSG_SIZE - 1));

    long long closeStart = mtime();
    close(serverSock);
    long long closeEnd = mtime();
    printf("Connection closed in %lld ms\n ", (closeEnd - closeStart));

    return 0;
}