#include "Server.h"
#include "utils/Utils.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <ctime>
#include <ifaddrs.h>
#include <iostream>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

Server::Server(int port) : serverSocket(-1), port(port), running(false) {
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    throw std::runtime_error("Error creating socket");
  }

  int opt = 1;
  if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
      0) {
    close(serverSocket);
    throw std::runtime_error("setsockopt failed");
  }

  std::memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) <
      0) {
    close(serverSocket);
    throw std::runtime_error("Error binding socket");
  }

  if (listen(serverSocket, 5) < 0) {
    close(serverSocket);
    throw std::runtime_error("Error listening on socket");
  }

  printIP();
}

Server::~Server() {
  stop();
  if (serverSocket != -1) {
    close(serverSocket);
  }
}

void Server::printIP() {
  struct ifaddrs *ifaddr, *i;
  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    return;
  }

  for (i = ifaddr; i != nullptr; i = i->ifa_next) {
    if (!i->ifa_addr || i->ifa_addr->sa_family != AF_INET ||
        (i->ifa_name && strcmp(i->ifa_name, "wlan0") != 0))
      continue;
    else {
      char host[NI_MAXHOST];
      int s = getnameinfo(i->ifa_addr, sizeof(struct sockaddr_in), host,
                          NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
      if (s == 0) {
        std::cout << "Connect via " << i->ifa_name << ": " << host << std::endl;
      }
    }
  }
  freeifaddrs(ifaddr);
}

void Server::broadcastMessage(const std::string &msg, int excludeSocket) {
  std::lock_guard<std::mutex> lock(clientsMutex);

  saveMessageToFile(msg);

  for (const auto &client : clients) {
    if (client.socket == excludeSocket)
      continue;
    size_t totalSent = 0;
    while (totalSent < msg.size()) {
      ssize_t sent = send(client.socket, msg.c_str() + totalSent,
                          msg.size() - totalSent, 0);
      if (sent < 0) {
        std::cerr << "Failed to send message to client (" << client.socket
                  << ")\n";
        break;
      }
      totalSent += sent;
    }
  }
}

void Server::handleClient(int clientSocket) {
  char nameBuffer[256] = {0};

  int nameBytes = recv(clientSocket, nameBuffer, sizeof(nameBuffer) - 1, 0);
  if (nameBytes <= 0) {
    close(clientSocket);
    return;
  }
  nameBuffer[nameBytes] = '\0';
  std::string username(nameBuffer);

  {
    std::lock_guard<std::mutex> lock(clientsMutex);
    clients.push_back({clientSocket, username});
  }
  std::cout << "Client (" << clientSocket << ") connected as " << username
            << std::endl;

  char buffer[1024] = {0};
  while (true) {
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
      break;
    }
    buffer[bytesReceived] = '\0';
    std::string message(buffer);
    std::cout << "Message from " << username << " (" << clientSocket
              << "): " << message << std::endl;

    broadcastMessage(message, clientSocket);
  }

  close(clientSocket);
  std::cout << "Client (" << clientSocket << ") disconnected." << std::endl;

  {
    std::lock_guard<std::mutex> lock(clientsMutex);
    clients.erase(std::remove_if(clients.begin(), clients.end(),
                                 [clientSocket](const Client &client) {
                                   return client.socket == clientSocket;
                                 }),
                  clients.end());
  }
}

void Server::run() {
  running = true;
  while (running) {
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0) {
      if (!running)
        break;
      std::cerr << "Error accepting connection\n";
      continue;
    }
    clientThreads.push_back(
        std::thread(&Server::handleClient, this, clientSocket));
  }
  for (auto &th : clientThreads) {
    if (th.joinable()) {
      th.join();
    }
  }
}

void Server::stop() {
  running = false;
  if (serverSocket != -1) {
    close(serverSocket);
    serverSocket = -1;
  }
}
