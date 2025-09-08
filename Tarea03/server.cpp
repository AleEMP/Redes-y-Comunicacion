#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <unordered_map>
using namespace std;

unordered_map<string, int> clientes_conectados;

void manejarCliente(int client_socket) 
{
    while (true) {
         char buffer[1024];
        //fill(buffer,buffer+sizeof(buffer),0);
        int n = read(client_socket, buffer, 1);
        buffer[n] = '\0';
        cout<<buffer<<endl;
        if (n <= 0) 
        {
            for (const auto& n : clientes_conectados){
                if(n.second==client_socket){
                    cout << clientes_conectados[n.first]  << " se desconectó." << endl;
                    clientes_conectados.erase(n.first);
                }
            }
            close(client_socket);
            return;
        }
       
        string recibido(buffer);
        char tipo = recibido[0];
        char tipoResp;
        if (tipo == 'n') 
        {
            n=read(client_socket, buffer, 2);
            buffer[n] = '\0';
            cout<<buffer<<endl;

            recibido=buffer;
            int tam = stoi(recibido);
            
            n=read(client_socket, buffer, tam);
            buffer[n] = '\0';
            cout<<buffer<<endl;
            string nickCliente = buffer;

            for (const auto& n : clientes_conectados){
                if(n.second==client_socket){
                    clientes_conectados.erase(n.first);
                }
            }
            
            clientes_conectados[nickCliente]= client_socket;

            cout << "Se cambió nick: " << buffer << endl; 
        } 
        else if (tipo == 'm') 
        {
            n=read(client_socket, buffer, 3);
            buffer[n] = '\0';
            cout<<buffer<<endl;

            
            recibido=buffer;
            int tam = stoi(recibido);
            string nickCliente;

            n=read(client_socket, buffer, tam);
            buffer[n] = '\0';
            cout<<buffer<<endl;

            string msg = buffer;

            for (const auto& n : clientes_conectados){
                if(n.second==client_socket){
                    nickCliente= n.first;
                    break;
                }
            }

            cout << nickCliente << ": " << msg << endl;

            tipoResp='M';
            int tamMSource = nickCliente.size();
            int tamMsg= msg.size();
            string tamañoSource,tamaño;
            tamañoSource = (tamMSource < 10) ? "0" + to_string(tamMSource) : to_string(tamMSource);
            
            if (tamMsg < 10) tamaño = "00" + to_string(tamMsg);
            else if (tamMsg < 100) tamaño = "0" + to_string(tamMsg);
            else tamaño = to_string(tamMsg);

            string enviar = tipoResp + tamañoSource +  nickCliente+ tamaño + msg;

            for (const auto& n : clientes_conectados){
                if(n.second!=client_socket){
                    write(n.second, enviar.c_str(), enviar.size());
                }
            }
        }
        else if (tipo == 't') {
            n=read(client_socket, buffer, 2);
            buffer[n] = '\0';
            cout<<buffer<<endl;

            recibido=buffer;
            int tam = stoi(recibido);

            string destination;
            n=read(client_socket, buffer, tam);
            buffer[n] = '\0';
            cout<<buffer<<endl;

            destination=buffer;

            n=read(client_socket, buffer, 3);

            buffer[n] = '\0';
            cout<<buffer<<endl;

            recibido=buffer;
            int tamMsg = stoi(recibido);

            n=read(client_socket, buffer, tamMsg);
            buffer[n] = '\0';
            cout<<buffer<< "n: "<< n <<endl;

            string msg = buffer;

            string nickCliente;
            for (const auto& n : clientes_conectados){
                if(n.second==client_socket){
                    nickCliente= n.first;
                    break;
                }
            }
            
            tipoResp='T';
            int tamMSource = nickCliente.size();
            string tamañoSource,tamaño;
            tamañoSource = (tamMSource < 10) ? "0" + to_string(tamMSource) : to_string(tamMSource);
            
            if (tamMsg < 10) tamaño = "00" + to_string(tamMsg);
            else if (tamMsg < 100) tamaño = "0" + to_string(tamMsg);
            else tamaño = to_string(tamMsg);

            string enviar = tipoResp + tamañoSource +  nickCliente+ tamaño + msg;

            write(clientes_conectados[destination], enviar.c_str(), enviar.size());

        }
        else if (tipo == 'l') {
            tipoResp='L';
            int tamMSource=clientes_conectados.size();
            string cantClient = (tamMSource < 10) ? "0" + to_string(tamMSource) : to_string(tamMSource);
            string enviar = tipoResp + cantClient; 
            for (const auto& n : clientes_conectados){
                string cantNombClient = (n.first.size() < 10) ? "0" + to_string(n.first.size()) : to_string(n.first.size());
                enviar= enviar + cantNombClient + n.first;
            } 
             write(client_socket, enviar.c_str(), enviar.size());
        }
        else if (tipo == 'x') {
            string nick_a_eliminar;
            for (const auto& par : clientes_conectados) {
                if (par.second == client_socket) {
                    nick_a_eliminar = par.first;
                    break;
                }
             }

            if (!nick_a_eliminar.empty()) {
                cout << "Cliente " << nick_a_eliminar << " desconectado" << endl;
                clientes_conectados.erase(nick_a_eliminar);
            }
            close(client_socket);
            return;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <puerto>\n";
        return 1;
    }

    int puerto = atoi(argv[1]);

    struct sockaddr_in stSockAddr;
    int servidor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(puerto);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (-1 == bind(servidor, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) {
        perror("error bind failed");
        close(servidor);
        exit(EXIT_FAILURE);
    }

    listen(servidor, 10);
    cout << "Servidor escuchando en puerto " << puerto << "...\n";

    while (true) {
        sockaddr_in direccionCliente;
        socklen_t tam = sizeof(direccionCliente);
        //int clienteSock = accept(servidor, (sockaddr*)&direccionCliente, &tam);
        int clienteSock =accept(servidor, NULL, NULL);
        cout << "Nuevo cliente conectado!\n";

        thread t(manejarCliente, clienteSock);
        t.detach();
    }

    close(servidor);
    return 0;
}
