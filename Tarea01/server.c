#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    struct sockaddr_in stSockAddr;
    int SocketServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    char buffer[256];
    int n;

    if (-1 == SocketServer) {
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (-1 == bind(SocketServer, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) {
        perror("error bind failed");
        close(SocketServer);
        exit(EXIT_FAILURE);
    }

    if (-1 == listen(SocketServer, 10)) {
        perror("error listen failed");
        close(SocketServer);
        exit(EXIT_FAILURE);
    }

    for (;;) {
        int SocketClient = accept(SocketServer, NULL, NULL);

        if (0 > SocketClient) {
            perror("error accept failed");
            close(SocketServer);
            exit(EXIT_FAILURE);
        }

        while (1) {
            bzero(buffer, 256);
            n = read(SocketClient, buffer, 255);

            printf("Mensaje recibido: %s\n", buffer);

            if (strcmp(buffer, "chau\n") == 0) {
                break;
            }

            printf("Respuesta para el cliente: ");
            fgets(buffer, sizeof(buffer), stdin);

            n = write(SocketClient, buffer, strlen(buffer));
            if (n < 0) {
                perror("ERROR writing to socket");
                break;
            }
        }

        shutdown(SocketClient, SHUT_RDWR);
        close(SocketClient);
        break;
    }

    close(SocketServer);
    return 0;
}
