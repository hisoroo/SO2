#include "Server.h"

#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

std::vector<Client> clients;
std::mutex clientsMutex;

std::mutex fileMutex;

std::string getLogFilename() {
  auto now = std::chrono::system_clock::now();
  std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm local_tm;
  localtime_r(&now_time_t, &local_tm);
  std::ostringstream oss;
  oss << "chat_log_" << std::put_time(&local_tm, "%Y-%m-%d") << ".txt";
  return oss.str();
}

void saveMessageToFile(const std::string &message) {
  std::lock_guard<std::mutex> lock(fileMutex);
  std::string filename = getLogFilename();
  std::ofstream outFile(filename, std::ios::app);
  if (outFile.is_open()) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;
    localtime_r(&now_time_t, &local_tm); 

    std::ostringstream timeStream;
    timeStream << std::put_time(&local_tm, "%H:%M:%S");

    outFile << "[" << timeStream.str() << "] " << message << "\n";
  } else {
    std::cerr << "Error: Unable to open " << filename << " for writing.\n";
  }
}

Server::Server(int port) : serverSocket(-1), port(port) {
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    std::cerr << "Error creating socket\n";
    exit(EXIT_FAILURE);
  }

  int opt = 1;
  if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
      0) {
    std::cerr << "setsockopt failed\n";
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  std::memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) <
      0) {
    std::cerr << "Error binding socket\n";
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  if (listen(serverSocket, 5) < 0) {
    std::cerr << "Error listening on socket\n";
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  printAddressAndPort();
}

Server::~Server() { close(serverSocket); }

void Server::printAddressAndPort() {
  sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  if (getsockname(serverSocket, (sockaddr *)&addr, &addrlen) == -1) {
    std::cerr << "Error getting local address" << std::endl;
    return;
  }
  char ip[INET_ADDRSTRLEN];
  if (inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip)) == nullptr) {
    std::cerr << "Error converting IP address" << std::endl;
    return;
  }
  std::cout << "Server is listening on " << ip << ":" << ntohs(addr.sin_port)
            << std::endl;
}

void Server::broadcastMessage(const std::string &username,
                              const std::string &msg, int excludeSocket) {
  std::lock_guard<std::mutex> lock(clientsMutex);
  std::string fullMessage = username + ": " + msg;

  saveMessageToFile(fullMessage);

  for (const auto &client : clients) {
    if (client.socket == excludeSocket)
      continue;
    if (send(client.socket, fullMessage.c_str(), fullMessage.size(), 0) < 0) {
      std::cerr << "Failed to send message to client (" << client.socket
                << ")\n";
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

    broadcastMessage(username, message, clientSocket);
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
  while (true) {
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0) {
      std::cerr << "Error accepting connection\n";
      continue;
    }
    std::thread(&Server::handleClient, this, clientSocket).detach();
  }
}
