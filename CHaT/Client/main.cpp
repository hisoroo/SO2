#include "Client.h"
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <ncurses.h>
#include <queue>
#include <string>
#include <thread>

using namespace std;

queue<string> messageQueue;
mutex queueMutex;
condition_variable queueCond;

void messageReceiver(Client *client) {
  client->receiveMessageLoop(messageQueue, queueMutex, queueCond);
}

void messagePrinter(Client *client) {
  while (true) {
    unique_lock<mutex> lock(queueMutex);
    if (queueCond.wait_for(lock, chrono::seconds(1),
                           []() { return !messageQueue.empty(); })) {
      string msg = messageQueue.front();
      messageQueue.pop();
      lock.unlock();

      cout << msg << endl;
    }
  }
}

struct ConnectionDetails {
  string username;
  string ip;
  int port;
};

int main() {
  ConnectionDetails conn;

  cout << "Enter username: ";
  cin >> conn.username;

  cout << "Enter servers IP address: ";
  cin >> conn.ip;

  cout << "Enter port number: ";
  cin >> conn.port;
  cin.ignore();

  try {
    Client *client = new Client(conn.ip.c_str(), conn.port, conn.username);

    thread receiver(messageReceiver, client);
    thread printer(messagePrinter, client);

    string message;
    cout << "Type your messages (type '!exit!' to quit):\n";
    while (getline(cin, message)) {
      if (message == "!exit!") {
        break;
      }
      message = "[" + client->getUsername() + "] " + message;
      client->sendMessage(message);
    }
    receiver.detach();
    printer.detach();
  } catch (const exception &ex) {
    cerr << "Error: " << ex.what() << endl;
  }

  return 0;
}
