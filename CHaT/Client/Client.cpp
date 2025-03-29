#include "Client.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

Client::Client(const char *ip, int port, const std::string &username)
    : clientSocket(-1), username(username) {
  clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket < 0) {
    throw std::runtime_error("Error creating socket");
  }

  std::memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);

  if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
    close(clientSocket);
    throw std::runtime_error("Invalid address");
  }

  if (connect(clientSocket, reinterpret_cast<struct sockaddr *>(&serverAddr),
              sizeof(serverAddr)) < 0) {
    close(clientSocket);
    throw std::runtime_error("Connection failed");
  }

  size_t totalSent = 0;
  while (totalSent < username.size()) {
    ssize_t bytesSent = send(clientSocket, username.c_str() + totalSent,
                             username.size() - totalSent, 0);
    if (bytesSent < 0) {
      close(clientSocket);
      throw std::runtime_error("Failed to send username");
    }
    totalSent += bytesSent;
  }
}

Client::~Client() {
  if (clientSocket >= 0) {
    close(clientSocket);
  }
}

void Client::sendMessage(const std::string &message) {
  size_t totalSent = 0;
  while (totalSent < message.size()) {
    ssize_t bytesSent = send(clientSocket, message.c_str() + totalSent,
                             message.size() - totalSent, 0);
    if (bytesSent < 0) {
      std::cerr << "Failed to send message\n";
      return;
    }
    totalSent += bytesSent;
  }
}

void Client::receiveMessageLoop(std::queue<std::string> &messageQueue,
                                std::mutex &queueMutex,
                                std::condition_variable &queueCond) {
  char buffer[1024];
  while (true) {
    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
      break;
    }
    buffer[bytesReceived] = '\0';
    std::string msg(buffer);
    {
      std::lock_guard<std::mutex> lock(queueMutex);
      messageQueue.push(msg);
    }
    queueCond.notify_one();
  }
}
