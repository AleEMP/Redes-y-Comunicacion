#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <iostream>
#include <string>

using namespace std;

int SocketClient;
bool PMCliente = true;
bool PMServer = true;
string nombreCliente = "Cliente: ";
string nombreServer = "Tú: ";
bool conexion_activa = true;
void FunTreads() {
    char buffer[256];
    int n;

    while (1) {
        bzero(buffer, 256);
        n = read(SocketClient, buffer, 255);

        if (n <= 0) {
            printf("Conexión cerrada por el cliente\n");
            conexion_activa = false; 
            break;
        }

        buffer[n] = '\0';

        char tipo = buffer[0];
        int mensIni;
        int tamMensj;
        char len_str[4];

        if (tipo == 'n') {
            tamMensj = 2;

            strncpy(len_str, buffer + 1, tamMensj);
            len_str[tamMensj] = '\0';
            mensIni = 1 + tamMensj;
        }
        else if (tipo == 'm') {
            tamMensj = 3;

            strncpy(len_str, buffer + 1, tamMensj);
            len_str[tamMensj] = '\0';
            mensIni = 1 + tamMensj;
        } 

        int tam_esperado = atoi(len_str);
        char *mensaje = buffer + mensIni;

        char mensaje_limpio[256];
        strcpy(mensaje_limpio, mensaje);
        mensaje_limpio[strcspn(mensaje_limpio, "\r\n")] = '\0';

        if (PMCliente) {
            nombreCliente = string(mensaje_limpio) + ": ";
            PMCliente = false;
        }
        else{
            cout << nombreCliente << mensaje_limpio << endl;
        }
        
        if (strcmp(mensaje_limpio, "chau") == 0) {
            conexion_activa = false; 
            break;
        }
    }

    close(SocketClient);
    return;
}

int main(void)
{
    struct sockaddr_in stSockAddr;
    int SocketServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    char buffer[256];
    int n;
    
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (-1 == bind(SocketServer, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) {
        perror("error bind failed");
        close(SocketServer);
        exit(EXIT_FAILURE);
    }
    listen(SocketServer, 10);
    SocketClient = accept(SocketServer, NULL, NULL);
    

    thread Tread(FunTreads);
    Tread.detach();
    while (conexion_activa) {
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        char bufferFinal[256];
        int len = strlen(buffer);

        if (PMServer) {
            char tam_str[3];
            sprintf(tam_str, "%02d", len);
            snprintf(bufferFinal, sizeof(bufferFinal), "n%s%s\n", tam_str, buffer);
            nombreServer = string(buffer) + ": ";
            PMServer = false;
        } 
        else {
            char tam_str[4];
            sprintf(tam_str, "%03d", len);
            snprintf(bufferFinal, sizeof(bufferFinal), "m%s%s\n", tam_str, buffer);
        }
        if(conexion_activa)
            n = write(SocketClient, bufferFinal, strlen(bufferFinal));
        else break;
        if (strcmp(buffer, "chau") == 0) {
            cout << "Conversación terminada." << endl;
            break;
        }
        
        if (n < 0) {
            perror("ERROR writing to socket");
            break;
        }
    }

    close(SocketClient);
    close(SocketServer);
    return 0;
}
