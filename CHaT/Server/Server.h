#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct Client {
  int socket;
  std::string username;
};

class Server {
public:
  Server(int port);
  ~Server();

  void run();
  void stop();

private:
  int serverSocket;
  int port;
  struct sockaddr_in serverAddr;

  std::vector<Client> clients;
  std::mutex clientsMutex;
  std::vector<std::thread> clientThreads;
  std::atomic_bool running;

  void broadcastMessage(const std::string &msg, int excludeSocket);
  void handleClient(int clientSocket);
  void printIP();
};

#endif // SERVER_H
