#ifndef TCP_STREAM_DUPLICATE_TCPSTREAMDUPLICATOR_H
#define TCP_STREAM_DUPLICATE_TCPSTREAMDUPLICATOR_H

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <thread>
#include <chrono>
#include <netdb.h>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <mutex>

using namespace std;

class TcpStreamDuplicator {

private:
    std::vector<int> clients;
    std::mutex clients_mutex;

    void accept_connections(int socket_descriptor) {
        while (true) {
            sockaddr_in address;
            socklen_t address_size = sizeof(address);

            errno = 0;
            int client_descriptor = accept(socket_descriptor, (sockaddr *) &address, &address_size);

            if (client_descriptor == -1) {
                cout << "Error caused on accept connection: " << strerror(errno) << endl;
                continue;
            }

            clients_mutex.lock();
            clients.push_back(client_descriptor);
            clients_mutex.unlock();

            cout << "Accepted new connection" << endl;
        }
    }

    int connect(const string &host, const int &port)
    {
        cout << "Connecting to the source server" << endl;

        struct hostent* resolved_host = gethostbyname(host.data());
        sockaddr_in address {};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*) *resolved_host->h_addr_list));
        address.sin_port = htons(port);

        int client_socket = socket(AF_INET, SOCK_STREAM, 0);

        if (::connect(client_socket, (sockaddr*) &address, sizeof(address)) < 0) {
            cout << "Error: " << strerror(errno) << endl;
            return -1;
        } else {
            cout << "Connected" << endl;
        }

        return client_socket;
    }

    int reconnect(const string &host, const int &port)
    {
        cout << "Reconnecting to the source server..." << endl;

        int connection_descriptor = -1;

        while (connection_descriptor == -1) {
            connection_descriptor = connect(host, port);
            if (connection_descriptor == -1) {
                this_thread::sleep_for(5000ms);
            }
        }

        return connection_descriptor;
    }

    void data_duplication(const string &host, const int &port) {
        using namespace std::chrono_literals;

        char msg[5 * 1024];
        int received_bytes;
        int client_socket;

        client_socket = connect(host, port);

        if (client_socket == -1) {
            client_socket = reconnect(host, port);
        }

        while (true) {
            errno = 0;
            received_bytes = recv(client_socket, (char*) &msg, sizeof(msg), 0);

            if (errno != 0) {
                cout << "An error caused on recv: " << strerror(errno) << endl;
                close(client_socket);
                client_socket = reconnect(host, port);
                continue;
            }

            if (received_bytes == 0) {
                cout << "Connection lost with the source server" << endl;
                close(client_socket);
                client_socket = reconnect(host, port);
                continue;
            }

            if (clients.empty()) {
                this_thread::sleep_for(100ms);
            } else {
                for (auto it = clients.begin(); it != clients.end();) {
                    errno = 0;
                    send(*it, msg, received_bytes, 0);

                    if (errno != 0) {
                        cout << "Connection closed" << endl;
                        close(*it);
                        clients_mutex.lock();
                        it = clients.erase(it);
                        clients_mutex.unlock();
                    } else {
                        it++;
                    }
                }
            }
        }
    }

public:
    void start(string source_host, unsigned int source_port, unsigned int destination_port) {
        cout << "Starting " << source_host << ":" << source_port << " -> " << ":" << destination_port << endl;

        const struct sigaction act = (struct sigaction) {SIG_IGN};
        sigaction(SIGPIPE, &act, NULL);

        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(destination_port);

        int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);

        if (socket_descriptor < 0) {
            cerr << "Caused an error on create socket" << endl;
            exit(0);
        }

        int optval = 1;
        setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

        if (bind(socket_descriptor, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
            cerr << "Error caused on binding socket" << endl;
            exit(0);
        }

        cout << "Waiting for clients..." << endl;

        if (::listen(socket_descriptor, 50) == -1) {
            cerr << "Error caused on trying to start socket" << endl;
        }

        thread accept_connection_thread(&TcpStreamDuplicator::accept_connections, this, socket_descriptor);
        thread stream_data_thread(&TcpStreamDuplicator::data_duplication, this, source_host, source_port);

        accept_connection_thread.join();
        stream_data_thread.join();
    }
};


#endif //TCP_STREAM_DUPLICATE_TCPSTREAMDUPLICATOR_H
