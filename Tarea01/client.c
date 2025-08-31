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
    char buffer[256];
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n;
 
    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000); //PORT FOR THE SERVER
    Res = inet_pton(AF_INET, "10.0.2.15", &stSockAddr.sin_addr); //IP ADRESS FOR THE SERVER
 
    if (0 > Res)
    {
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    else if (0 == Res)
    {
      perror("char string (second parameter does not contain valid ipaddress");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    } 
    bzero(buffer,256);
    while (1) {
    printf("Escribe mensaje: ");
    fgets(buffer, sizeof(buffer), stdin);
    n = write(SocketFD, buffer, strlen(buffer));
    if (n < 0) {
        perror("error writing to socket");
        break;
    }
    if (strcmp(buffer, "chau\n") == 0) {
        break;
    }
    bzero(buffer, 256);
    n = read(SocketFD, buffer, 255);
    if (n <= 0) {
        perror("error reading from socket");
        break;
    }

    printf("Respuesta del servidor: %s\n", buffer);
}
    shutdown(SocketFD, SHUT_RDWR);
 
    close(SocketFD);
    return 0;
  }
