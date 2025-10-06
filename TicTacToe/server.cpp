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
vector<int> clientesJugando;
string simbolo = "o";
const int tamTablero = 3;
string tablero[tamTablero * tamTablero];
int movimientos = 0;

void reiniciarJuego();
bool revisarVictoria(const string& simbolo_jugador);
bool revisarEmpate();

void reiniciarJuego() {
    for (int i = 0; i < tamTablero * tamTablero; i++) {
        tablero[i] = "_";
    }
    movimientos = 0;
    clientesJugando.clear();
    cout << "\nJuego reiniciado. Esperando nuevos jugadores." << endl;
}

bool revisarVictoria(const string& simbolo_jugador) {
    // filas
    for (int i = 0; i < tamTablero; ++i) {
        bool fila_completa = true;
        for (int j = 0; j < tamTablero; ++j) {
            if (tablero[i * tamTablero + j] != simbolo_jugador) {
                fila_completa = false;
                break;
            }
        }
        if (fila_completa) return true;
    }

    // columnas
    for (int i = 0; i < tamTablero; ++i) {
        bool col_completa = true;
        for (int j = 0; j < tamTablero; ++j) {
            if (tablero[j * tamTablero + i] != simbolo_jugador) {
                col_completa = false;
                break;
            }
        }
        if (col_completa) return true;
    }

    // diagonal principal
    bool diag1_completa = true;
    for (int i = 0; i < tamTablero; ++i) {
        if (tablero[i * tamTablero + i] != simbolo_jugador) {
            diag1_completa = false;
            break;
        }
    }
    if (diag1_completa) return true;

    // diagonal secundaria
    bool diag2_completa = true;
    for (int i = 0; i < tamTablero; ++i) {
        if (tablero[i * tamTablero + (tamTablero - 1 - i)] != simbolo_jugador) {
            diag2_completa = false;
            break;
        }
    }
    if (diag2_completa) return true;

    return false;
}

bool revisarEmpate() {
    return movimientos == tamTablero * tamTablero;
}

void manejarCliente(int client_socket) {
    while (true) {
        char buffer[1024] = {0};
        int n = read(client_socket, buffer, 1);
        cout << string(buffer, n); cout.flush();

        if (n <= 0) {
            for (auto it = clientes_conectados.begin(); it != clientes_conectados.end(); ) {
                if (it->second == client_socket) {
                    cout << "\n" << it->first << " se desconectó." << endl;
                    it = clientes_conectados.erase(it);
                } else {
                    ++it;
                }
            }
            auto it_jugador = find(clientesJugando.begin(), clientesJugando.end(), client_socket);
            if (it_jugador != clientesJugando.end()) {
                 cout << "\nUn jugador abandonó la partida. Reiniciando el juego." << endl;
                 reiniciarJuego();
            }
            close(client_socket);
            return;
        }

        string recibido(buffer, n);
        char tipo = recibido[0];

        if (tipo == 'n') {
            n = read(client_socket, buffer, 2);
            cout << string(buffer, n); cout.flush();
            int tam = stoi(string(buffer, 2));

            n = read(client_socket, buffer, tam);
            cout << string(buffer, n) << endl;
            string nickCliente(buffer, tam);
            
            for (auto it = clientes_conectados.begin(); it != clientes_conectados.end(); ) {
                if (it->second == client_socket) {
                    it = clientes_conectados.erase(it);
                } else {
                    ++it;
                }
            }
            clientes_conectados[nickCliente] = client_socket;
        } else if (tipo == 'm') {
            n = read(client_socket, buffer, 3);
            cout << string(buffer, n); cout.flush();
            int tamMsg = stoi(string(buffer, 3));

            n = read(client_socket, buffer, tamMsg);
            cout << string(buffer, n) << endl;
            string msg(buffer, tamMsg);
            
            string nickCliente;
            for (const auto& par : clientes_conectados) {
                if (par.second == client_socket) {
                    nickCliente = par.first;
                    break;
                }
            }

            char tipoResp = 'M';
            int tamNick = nickCliente.size();
            string tamañoSource = (tamNick < 10) ? "0" + to_string(tamNick) : to_string(tamNick);
            string tamañoMsg = (tamMsg < 10) ? "00" + to_string(tamMsg) : (tamMsg < 100) ? "0" + to_string(tamMsg) : to_string(tamMsg);
            
            string enviar = tipoResp + tamañoSource + nickCliente + tamañoMsg + msg;

            for (const auto& par : clientes_conectados) {
                if (par.second != client_socket) {
                    cout << enviar << endl;
                    write(par.second, enviar.c_str(), enviar.size());
                }
            }
        } else if (tipo == 't') {
            n = read(client_socket, buffer, 2);
            cout << string(buffer, n); cout.flush();
            int tamDest = stoi(string(buffer, 2));

            n = read(client_socket, buffer, tamDest);
            cout << string(buffer, n); cout.flush();
            string destination(buffer, tamDest);

            n = read(client_socket, buffer, 3);
            cout << string(buffer, n); cout.flush();
            int tamMsg = stoi(string(buffer, 3));

            n = read(client_socket, buffer, tamMsg);
            cout << string(buffer, n) << endl;
            string msg(buffer, tamMsg);

            string nickCliente;
            for (const auto& par : clientes_conectados) {
                if (par.second == client_socket) {
                    nickCliente = par.first;
                    break;
                }
            }

            char tipoResp = 'T';
            int tamNick = nickCliente.size();
            string tamañoSource = (tamNick < 10) ? "0" + to_string(tamNick) : to_string(tamNick);
            string tamañoMsg = (tamMsg < 10) ? "00" + to_string(tamMsg) : (tamMsg < 100) ? "0" + to_string(tamMsg) : to_string(tamMsg);

            string enviar = tipoResp + tamañoSource + nickCliente + tamañoMsg + msg;
            
            if (clientes_conectados.count(destination)) {
                cout << enviar << endl;
                write(clientes_conectados[destination], enviar.c_str(), enviar.size());
            }

        } else if (tipo == 'l') {
            cout << endl;
            string lista_clientes_str;
            for (const auto& par : clientes_conectados) {
                string tamNick = (par.first.size() < 10) ? "0" + to_string(par.first.size()) : to_string(par.first.size());
                lista_clientes_str += tamNick + par.first;
            }
            string cantClientes = (clientes_conectados.size() < 10) ? "0" + to_string(clientes_conectados.size()) : to_string(clientes_conectados.size());
            string enviar = "L" + cantClientes + lista_clientes_str;
            cout << enviar << endl;
            write(client_socket, enviar.c_str(), enviar.size());
        } else if (tipo == 'p') {
            cout << endl;
            if (find(clientesJugando.begin(), clientesJugando.end(), client_socket) != clientesJugando.end()) {
                continue;
            }
            if (clientesJugando.size() < 2) {
                clientesJugando.push_back(client_socket);

                if (clientesJugando.size() == 2) {
                    string enviar = "Vo";
                    cout << enviar << endl;
                    write(clientesJugando[0], enviar.c_str(), enviar.size());
                }
            }
        } else if (tipo == 'w') {
            n = read(client_socket, buffer, 1);
            cout << string(buffer, n); cout.flush();
            string simbolo_jugador(buffer, 1);

            n = read(client_socket, buffer, 1);
            cout << string(buffer, n) << endl;
            int pos = stoi(string(buffer, 1));
            
            if (pos >= 1 && pos <= tamTablero * tamTablero && tablero[pos-1] == "_") {
                tablero[pos - 1] = simbolo_jugador;
                movimientos++;

                string enviar_tablero = "v";
                for(int i = 0; i < tamTablero*tamTablero; ++i) enviar_tablero += tablero[i];
                
                cout << enviar_tablero << endl;
                write(clientesJugando[0], enviar_tablero.c_str(), enviar_tablero.size());
                cout << enviar_tablero << endl;
                write(clientesJugando[1], enviar_tablero.c_str(), enviar_tablero.size());
                
                if (revisarVictoria(simbolo_jugador)) {
                    int ganador = (simbolo_jugador == "o") ? clientesJugando[0] : clientesJugando[1];
                    int perdedor = (simbolo_jugador == "o") ? clientesJugando[1] : clientesJugando[0];
                    string enviar="Owin";
                    cout << enviar << endl;
                    write(ganador, enviar.c_str(), enviar.size()); 
                    enviar="Olos";
                    cout << enviar << endl;
                    write(perdedor, enviar.c_str(), enviar.size()); 
                    reiniciarJuego();
                } else if (revisarEmpate()) {
                    string enviar="Oemp";
                    cout << enviar << endl;
                    write(clientesJugando[0], enviar.c_str(), enviar.size()); 
                    cout << enviar << endl;
                    write(clientesJugando[1], enviar.c_str(), enviar.size());
                    reiniciarJuego();
                } else {
                    string enviar;
                    if (simbolo_jugador == "o") {
                        enviar = "Vx";
                        cout << enviar << endl;
                        write(clientesJugando[1], enviar.c_str(), enviar.size());
                    } else {
                        enviar = "Vo";
                        cout << enviar << endl;
                        write(clientesJugando[0], enviar.c_str(), enviar.size());
                    }
                }
            }
        } else if (tipo == 'x') {
            cout << endl;
            string nick_a_eliminar;
            for (const auto& par : clientes_conectados) {
                if (par.second == client_socket) {
                    nick_a_eliminar = par.first;
                    break;
                }
            }
            if (!nick_a_eliminar.empty()) {
                clientes_conectados.erase(nick_a_eliminar);
            }
             auto it_jugador = find(clientesJugando.begin(), clientesJugando.end(), client_socket);
            if (it_jugador != clientesJugando.end()) {
                 reiniciarJuego();
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
    reiniciarJuego();

    struct sockaddr_in stSockAddr;
    int servidor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(puerto);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;
    
    int enable = 1;
    if (setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
    }

    if (-1 == bind(servidor, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) {
        perror("error bind failed");
        close(servidor);
        exit(EXIT_FAILURE);
    }

    if (-1 == listen(servidor, 10)) {
        perror("error listen failed");
        close(servidor);
        exit(EXIT_FAILURE);
    }
    
    cout << "Servidor escuchando en puerto " << puerto << "...\n";

    while (true) {
        int clienteSock = accept(servidor, NULL, NULL);
        if (clienteSock < 0) {
            perror("error accept failed");
            continue;
        }
        thread t(manejarCliente, clienteSock);
        t.detach();
    }

    close(servidor);
    return 0;
}
