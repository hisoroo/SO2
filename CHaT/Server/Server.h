#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <mutex>
#include <string>
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

private:
  int serverSocket;
  int port;
  struct sockaddr_in serverAddr;

  void broadcastMessage(const std::string &username, const std::string &msg,
                        int excludeSocket);

  void handleClient(int clientSocket);

  void printAddressAndPort();
};

#endif // SERVER_H
