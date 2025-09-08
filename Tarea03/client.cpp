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
#include <format>
using namespace std;

void showMenu(){
    cout<<"---------------------------------"<<endl;
    cout<<"1.Change your nickname"<<endl;
    cout<<"2.Send message to someone"<<endl;
    cout<<"3.Send message to everyone"<<endl;
    cout<<"4.Show participant's list"<<endl;
    cout<<"5.Disconnect"<<endl;
     cout<<"---------------------------------"<<endl;
}

void recibirMensajes(int socketCliente) {
    char buffer[1024];

    while (true) {
        int n = read(socketCliente, buffer, 1); 
        if (n <= 0) {
            cout << "Desconectado del servidor." << endl;
            close(socketCliente);
            exit(0);
        }
        char tipo = buffer[0];

        if (tipo == 'M') {
            read(socketCliente, buffer, 2);
            int tamNick = stoi(string(buffer, 2));
            
            read(socketCliente, buffer, tamNick);
            string nick = string(buffer, tamNick);
            
            read(socketCliente, buffer, 3);
            int tamMsg = stoi(string(buffer, 3));
            
            read(socketCliente, buffer, tamMsg);
            string msg = string(buffer, tamMsg);

            cout << "\n[" << nick << " dice]: " << msg << endl;

        } else if (tipo == 'T') { // Mensaje privado
            read(socketCliente, buffer, 2);
            int tamNick = stoi(string(buffer, 2));
            
            read(socketCliente, buffer, tamNick);
            string nick = string(buffer, tamNick);
            
            read(socketCliente, buffer, 3);
            int tamMsg = stoi(string(buffer, 3));
            
            read(socketCliente, buffer, tamMsg);
            string msg = string(buffer, tamMsg);

            cout << "\n[MENSAJE PRIVADO de " << nick << "]: " << msg << endl;

        } else if (tipo == 'L') { // Lista de participantes
            read(socketCliente, buffer, 2);
            int cantClientes = stoi(string(buffer, 2));
            cout << "\n--- Participantes Conectados (" << cantClientes << ") ---" << endl;
            for (int i = 0; i < cantClientes; ++i) {
                read(socketCliente, buffer, 2);
                int tamNick = stoi(string(buffer, 2));
                
                read(socketCliente, buffer, tamNick);
                string nick = string(buffer, tamNick);
                cout << "- " << nick << endl;
            }
            cout << "-------------------------------------" << endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Uso: " << argv[0] << " <ip-servidor> <puerto>\n";
        return 1;
    }

    string ip = argv[1];
    int puerto = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in stSockAddr{};
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(puerto);
    stSockAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(sock, (sockaddr*)&stSockAddr, sizeof(stSockAddr)) == -1) {
        cerr << "Error conectando al servidor\n";
        return 1;
    }

    cout << "Conectado al servidor!\n";

    thread t(recibirMensajes, sock);
    t.detach();

    int entrada;
    while (true) {

        showMenu();
        cin >> entrada;
        string cont, tamaño, enviar, destin,tamañoDestin;
        char tipo;
        if(entrada== 1){
            tipo='n';
            cout<<"Enter your new nickname: ";
            cin>>cont;
            int tamNick = cont.size();
            tamaño = (tamNick < 10) ? "0" + to_string(tamNick) : to_string(tamNick);
            enviar = string(1, tipo) + tamaño + cont;
        }
        else if(entrada== 2){
            tipo='t';
            cout<<"Enter destination: ";
            cin>>destin;
            cout<<endl<<"Enter your message: ";
            cin.ignore();
            getline(cin,cont);
            int tamDest = destin.size();
            int tamMsg = cont.size();

            tamañoDestin = (tamDest < 10) ? "0" + to_string(tamDest) : to_string(tamDest);

            if (tamMsg < 10) tamaño = "00" + to_string(tamMsg);
            else if (tamMsg < 100) tamaño = "0" + to_string(tamMsg);
            else tamaño = to_string(tamMsg);

            enviar = string(1, tipo) +tamañoDestin+ destin + tamaño + cont;
        }
        else if(entrada== 3){
            tipo='m';
            cout<<"Enter your message to everyone: ";
            cin.ignore();
            getline(cin,cont);
            int tamMsg = cont.size();

            if (tamMsg < 10) tamaño = "00" + to_string(tamMsg);
            else if (tamMsg < 100) tamaño = "0" + to_string(tamMsg);
            else tamaño = to_string(tamMsg);

            enviar = string(1, tipo) + tamaño + cont;

        }
        else if(entrada== 4){
            tipo='l';
            enviar = string(1, tipo);
        }
        else if(entrada== 5){
            tipo='x';
            enviar = string(1, tipo);
            write(sock, enviar.c_str(), enviar.size());
            break;
        }
        cout<< enviar << endl;
        write(sock, enviar.c_str(), enviar.size());
    }

    close(sock);
    return 0;
}
