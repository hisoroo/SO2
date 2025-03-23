#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>

using namespace std;

const int MIN_TIME = 2000;
const int MAX_TIME = 5000;

const string COL_THINK = "\033[1;37m";  // biały
const string COL_HUNGRY = "\033[1;34m"; // niebieski
const string COL_EAT = "\033[1;32m";    // zielony
const string COL_RESET = "\033[0m";     // reset koloru

int randTime(int min = MIN_TIME, int max = MAX_TIME) {
  static mt19937 randomGenerator(time(nullptr));
  return uniform_int_distribution<>(min, max)(randomGenerator);
}

// TODO: dodac -t --time do ustalania dlugosci trwania symulacji
int parseArgs(int argc, char *argv[]) {
  int philosopherCount = 5;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--count") == 0) {
      if (i + 1 < argc) {
        istringstream iss(argv[i + 1]);
        if (!(iss >> philosopherCount) || philosopherCount < 2) {
          cerr << "Podaj poprawną liczbę filozofów (wiecej niż dwóch).\n";
          exit(EXIT_FAILURE);
        }
        ++i;
      } else {
        cerr << "Brakuje wartości dla flagi " << argv[i] << "\n";
        exit(EXIT_FAILURE);
      }
    }
  }
  return philosopherCount;
}

void think(int pID, mutex &outMutex) {
  lock_guard<mutex> og(outMutex);
  cout << COL_THINK;
  cout << "------------------------------\n";
  cout << "Filozof " << pID << " myśli\n";
  cout << "------------------------------" << COL_RESET << "\n";
}

void hungry(int pID, mutex &outMutex) {
  lock_guard<mutex> og(outMutex);
  cout << COL_HUNGRY;
  cout << "------------------------------\n";
  cout << "Filozof " << pID << " jest głodny\n";
  cout << "------------------------------" << COL_RESET << "\n";
}

void eat(int pID, mutex &outMutex) {
  lock_guard<mutex> og(outMutex);
  cout << COL_EAT;
  cout << "------------------------------\n";
  cout << "Filozof " << pID << " je\n";
  cout << "------------------------------" << COL_RESET << "\n";
}

void philosopherLoop(int pID, mutex &leftFork, mutex &rightFork,
                     mutex &outMutex) {
  while (true) {
    think(pID, outMutex);
    this_thread::sleep_for(chrono::milliseconds(randTime()));
    hungry(pID, outMutex);
    lock_guard<mutex> lfg(leftFork);
    lock_guard<mutex> rfg(rightFork);
    eat(pID, outMutex);
    this_thread::sleep_for(chrono::milliseconds(randTime()));
  }
}
int main(int argc, char *argv[]) {

  int philosopherCount = parseArgs(argc, argv);

  vector<unique_ptr<mutex>> forks;
  for (int i = 0; i < philosopherCount; ++i) {
    forks.push_back(make_unique<mutex>());
  }

  mutex outMutex;

  vector<thread> philosophers;
  for (int i = 0; i < philosopherCount; ++i) {

    int nextFork = (i + 1) % philosopherCount;
    mutex *leftFork;
    mutex *rightFork;

    if (i < nextFork) {
      leftFork = forks[i].get();
      rightFork = forks[nextFork].get();
    } else {
      leftFork = forks[nextFork].get();
      rightFork = forks[i].get();
    }

    philosophers.push_back(thread([&, i, leftFork, rightFork]() {
      philosopherLoop(i + 1, *leftFork, *rightFork, outMutex);
    }));
  }

  for (auto &t : philosophers) {
    t.join();
  }

  return 0;
}
