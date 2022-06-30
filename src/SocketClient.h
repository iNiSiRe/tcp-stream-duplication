//
// Created by inisire on 30.06.22.
//

#ifndef TCP_STREAM_DUPLICATE_SOCKETCLIENT_H
#define TCP_STREAM_DUPLICATE_SOCKETCLIENT_H

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

class SocketClient {
private:
    const int socket;
    bool connected;
    thread transmission;
    queue<string> messages;
    mutex queue_mutex;
    condition_variable queue_cv;

    void transmit()
    {
        while (this->connected) {
            std::unique_lock<std::mutex> lock(queue_mutex);

            while (this->messages.empty()) {
                queue_cv.wait(lock);
            }

            auto message = this->messages.front();
            this->messages.pop();

            lock.unlock();

            errno = 0;
            send(socket, message.c_str(), message.length(), 0);

            if (errno != 0) {
                cout << "Connection closed" << endl;
                close(socket);
                this->connected = false;
            }
        }
    }

public:
    SocketClient(const int socket) :
        socket(socket)
    {
        this->connected = true;
        this->transmission = thread(&SocketClient::transmit, this);
        this->transmission.detach();
    }

    void write(const string &message)
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        this->messages.push(message);
        queue_cv.notify_one();
    }

    bool is_connected()
    {
        return this->connected;
    }
};

#endif //TCP_STREAM_DUPLICATE_SOCKETCLIENT_H
