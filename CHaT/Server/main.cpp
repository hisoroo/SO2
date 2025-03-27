#include "Server.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

int main(int argc, char* argv[]) {
  int port = 8080; 

  for (int i = 1; i < argc; ++i) {
    if ((std::strcmp(argv[i], "-p") == 0 || std::strcmp(argv[i], "--port") == 0) && i + 1 < argc) {
      port = std::atoi(argv[i + 1]);
      ++i;
    } else {
      std::cerr << "Unknown argument: " << argv[i] << std::endl;
    }
  }

  Server server(port);
  server.run();
  return 0;
}
