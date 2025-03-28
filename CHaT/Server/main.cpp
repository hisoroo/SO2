#include "Server.h"
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

Server *g_server = nullptr;

void signalHandler(int signal) {
  if (g_server) {
    std::cout << "\nShutting down server..." << std::endl;
    g_server->stop();
  }
}

int main(int argc, char *argv[]) {
  int port = 8080;
  try {
    for (int i = 1; i < argc; ++i) {
      if ((std::strcmp(argv[i], "-p") == 0 ||
           std::strcmp(argv[i], "--port") == 0) &&
          i + 1 < argc) {
        port = std::stoi(argv[i + 1]);
        if (port <= 0 || port > 65535) {
          throw std::invalid_argument("Port number out of range");
        }
        ++i;
      } else {
        std::cerr << "Unknown argument: " << argv[i] << std::endl;
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "Error parsing arguments: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  std::signal(SIGINT, signalHandler);

  try {
    Server server(port);
    g_server = &server;
    server.run();
  } catch (const std::exception &e) {
    std::cerr << "Server error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Server stopped." << std::endl;
  return EXIT_SUCCESS;
}

