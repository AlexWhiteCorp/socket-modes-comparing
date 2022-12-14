#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "consts.h"

struct Configs {
    unsigned short socketType;
    unsigned short blocking;
    unsigned short sync;
    unsigned short echo;
} Configs_Default = {INET_SOCKET, 1, 1};

struct Connection {
    int id;
    int socket;
    struct Configs configs;
};

struct Configs getConfigs(int argc, char *argv[]) {
    struct Configs configs = Configs_Default;

    for (int i = 1; i + 1 < argc; i += 2) {
        char *argType = argv[i];
        char *argValue = argv[i + 1];

        if (strcmp(argType, "-t") == 0) {
            if (strcmp(argValue, "unix") == 0) {
                configs.socketType = UNIX_SOCKET;
                continue;
            }
            if (strcmp(argValue, "inet") == 0) {
                configs.socketType = INET_SOCKET;
                continue;
            }
            continue;
        }

        if (strcmp(argType, "-b") == 0) {
            if (strcmp(argValue, "true") == 0) {
                configs.blocking = 1;
                continue;
            }
            if (strcmp(argValue, "false") == 0) {
                configs.blocking = 0;
                continue;
            }
            continue;
        }

        if (strcmp(argType, "-s") == 0) {
            if (strcmp(argValue, "true") == 0) {
                configs.sync = 1;
                continue;
            }
            if (strcmp(argValue, "false") == 0) {
                configs.sync = 0;
                continue;
            }
            continue;
        }

        if (strcmp(argType, "-e") == 0) {
            if (strcmp(argValue, "true") == 0) {
                configs.echo = 1;
                continue;
            }
            if (strcmp(argValue, "false") == 0) {
                configs.echo = 0;
                continue;
            }
            continue;
        }

        printf("[Thread main] Unknown config %s %s\n", argType, argValue);
    }

    return configs;
}

long long mtime() {
    struct timeval t;

    gettimeofday(&t, NULL);
    long long mt = (long long) t.tv_sec * 1000 + t.tv_usec / 1000;
    return mt;
}

int mapSocketType(int socketType) {
    if (socketType == UNIX_SOCKET) {
        return AF_UNIX;
    }
    if (socketType == INET_SOCKET) {
        return PF_INET;
    }

    perror("mapSocketType");
    exit(1);
}

int getSocket(struct Configs configs) {
    int sockDesc;
    if ((sockDesc = socket(mapSocketType(configs.socketType), SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    return sockDesc;
}

void bindSocket(int socket, struct Configs configs) {
    if (configs.socketType == UNIX_SOCKET) {
        int len;
        struct sockaddr_un localAddr;

        localAddr.sun_family = AF_UNIX;
        strcpy(localAddr.sun_path, SOCK_PATH);
        len = strlen(localAddr.sun_path) + sizeof(localAddr.sun_family);

        if (bind(socket, (struct sockaddr *) &localAddr, len) == -1) {
            perror("bind");
            exit(1);
        }

        if (listen(socket, 16) == -1) {
            perror("listen");
            exit(1);
        }

        return;
    }
    if (configs.socketType == INET_SOCKET) {
        struct sockaddr_in localAddr;

        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(PORT);
        localAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(socket, (struct sockaddr *) &localAddr, sizeof(localAddr)) < 0) {
            perror("bind");
            exit(1);
        }

        if (listen(socket, 16) == -1) {
            perror("listen");
            exit(1);
        }

        return;
    }

    perror("bindSocket");
    exit(1);
}

int acceptConnection(int serverSocket) {
    struct sockaddr_in remoteAddr;
    int clientSock, t = sizeof(remoteAddr);
    if ((clientSock = accept(serverSocket, (struct sockaddr *) &remoteAddr, (socklen_t*) &t)) == -1) {
        perror("accept");
        exit(1);
    }

    return clientSock;
}

void switchBlockingType(int clientSock, struct Configs configs) {
    if (configs.blocking == 0) {
        printf("[Thread main] Turn ON Non-Blocking mode\n");
        fcntl(clientSock, F_SETFL, O_NONBLOCK);
    }
}

void connectToServer(int serverSocket, struct Configs configs) {
    if (configs.socketType == UNIX_SOCKET) {
        int len;
        struct sockaddr_un remote;

        remote.sun_family = AF_UNIX;
        strcpy(remote.sun_path, SOCK_PATH);
        len = strlen(remote.sun_path) + sizeof(remote.sun_family);

        if (connect(serverSocket, (struct sockaddr *) &remote, len) == -1) {
            perror("connect");
            exit(1);
        }

        return;
    }
    if (configs.socketType == INET_SOCKET) {
        struct sockaddr_in remote;

        remote.sin_family = AF_INET;
        remote.sin_port = htons(PORT);
        inet_pton(AF_INET, HOST, &remote.sin_addr);

        if (connect(serverSocket, (struct sockaddr *) &remote, sizeof(remote)) == -1) {
            perror("connect");
            exit(1);
        }

        return;
    }

    perror("connectToServer");
    exit(1);
}