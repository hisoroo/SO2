#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <netinet/in.h>

class Client {
public:
    Client(const char* ip, int port, const std::string& username);
    ~Client();

    void sendMessage(const std::string& message);
    void receiveMessageLoop(std::queue<std::string>& messageQueue,
                            std::mutex& queueMutex,
                            std::condition_variable& queueCond);

    std::string getUsername() const { return username; }

private:
    int clientSocket;
    struct sockaddr_in serverAddr;
    std::string username;
};

#endif

