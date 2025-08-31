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

int SocketFD;
bool PMServer = true;   
bool PMCliente = true;  
string nombreServer = "Server: ";
string nombreCliente = "Tú: ";
bool conexion_activa = true;

void FunTreads() {
    char buffer[256];
    int n;

    while (1) {
        bzero(buffer, 256);
        n = read(SocketFD, buffer, 255);

        if (n <= 0) {
            printf("Conexión cerrada por el servidor\n");
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

        if (PMServer) {
            nombreServer = string(mensaje_limpio) + ": ";
            PMServer = false;
        }
        else{
            cout << nombreServer << mensaje_limpio << endl;
        }

        if (strcmp(mensaje_limpio, "chau") == 0) {
            conexion_activa = false; 
            break;
        }
    }

    close(SocketFD);
    return;
}

int main(void)
{
    struct sockaddr_in stSockAddr;
    int n;

    SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000);

    inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);
    connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
       
    thread Tread(FunTreads);

    char buffer[256];

    while (conexion_activa) {

        fgets(buffer, sizeof(buffer), stdin);

        buffer[strcspn(buffer, "\n")] = '\0';
        char bufferFinal[256];
        int len = strlen(buffer);

        if (PMCliente) {
            char tam_str[3];
            sprintf(tam_str, "%02d", len);
            snprintf(bufferFinal, sizeof(bufferFinal), "n%s%s\n", tam_str, buffer);
            nombreCliente = string(buffer) + ": ";
            PMCliente = false;
        } 
        else {
            char tam_str[4];
            sprintf(tam_str, "%03d", len);
            snprintf(bufferFinal, sizeof(bufferFinal), "m%s%s\n", tam_str, buffer);
        }
        if(conexion_activa)
            n = write(SocketFD, bufferFinal, strlen(bufferFinal));
        else break;

        if (strcmp(buffer, "chau") == 0) {
            cout << "Conversación terminada." << endl;
            break;
        }
        
        if (n < 0) {
            perror("error writing to socket");
            break;
        }
    }

    close(SocketFD);
    Tread.join();
    return 0;
}
