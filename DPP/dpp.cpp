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

const string COL_THINK = "\033[1;37m";  // white
const string COL_HUNGRY = "\033[1;34m"; // blue
const string COL_EAT = "\033[1;32m";    // green
const string COL_RESET = "\033[0m";     // color reset

/*
 * Generates a random integer within the range [min, max].
 * Used to simulate the duration of a philosopher's activity (e.g., eating or
 * thinking).
 * @param min Minimum time value (default: MIN_TIME).
 * @param max Maximum time value (default: MAX_TIME).
 * @return Random time within the given range.
 */
int randTime(int min = MIN_TIME, int max = MAX_TIME) {
  static mt19937 randomGenerator(time(nullptr));
  return uniform_int_distribution<>(min, max)(randomGenerator);
}

/*
 * Parses command-line arguments to determine the number of philosophers.
 * Accepts '-c' or '--count' followed by an integer greater than 2.
 * If invalid or missing, prints an error and exits.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Number of philosophers (default: 5).
 */
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

/*
 * The following three functions display the philosopher's state
 * Each uses a mutex to ensure thread-safe console output.
 * @param pID ID of the philosopher.
 * @param outMutex Mutex for synchronized output.
 */
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

/*
 * Loop for a single philosopher in the simulation.
 * Alternates between three states (thinking, hungry, eating).
 * Each action is followed by a randomized delay to simulate activity duration.
 * @param pID ID of the philosopher.
 * @param leftFork Mutex representing the philosopher's left fork.
 * @param rightFork Mutex representing the philosopher's right fork.
 * @param outMutex Mutex used for synchronized console output.
 */
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
  // Parse command-line arguments to get the number of philosophers.
  int philosopherCount = parseArgs(argc, argv);

  // Create one fork (mutex) per philosopher.
  vector<unique_ptr<mutex>> forks;
  for (int i = 0; i < philosopherCount; ++i) {
    forks.push_back(make_unique<mutex>());
  }

  // Mutex for synchronizing console output across threads.
  mutex outMutex;

  // Create philosopher threads.
  vector<thread> philosophers;
  for (int i = 0; i < philosopherCount; ++i) {

    // Index of the next (right-hand) fork in circular order.
    int nextFork = (i + 1) % philosopherCount;
    mutex *leftFork;
    mutex *rightFork;

    // Ensure consistent fork locking order to prevent deadlock.
    if (i < nextFork) {
      leftFork = forks[i].get();
      rightFork = forks[nextFork].get();
    } else {
      leftFork = forks[nextFork].get();
      rightFork = forks[i].get();
    }

    // Start each philosopher thread.
    philosophers.push_back(thread([&, i, leftFork, rightFork]() {
      philosopherLoop(i + 1, *leftFork, *rightFork, outMutex);
    }));
  }

  // Ensure that a thread has completed its execution before the program exits.
  for (auto &t : philosophers) {
    t.join();
  }

  return 0;
}
