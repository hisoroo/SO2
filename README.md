# Projekt - Systemy Operacyjne 2

## Temat: Problem ucztujących filozofów

## Instrukcje uruchomienia

1. **Klonowanie repozytorium:**

   - Otwórz terminal i sklonuj repozytorium za pomocą `git`:
     ```bash
     git clone https://github.com/hisoroo/SO2.git
     cd SO2
     ```

2. **Kompilacja projektu:**

   - Upewnij się, że masz zainstalowany kompilator `g++`.
   - Skorzystaj z dołączonego pliku Makefile, wpisując w terminal:
     ```bash
     make
     ```

3. **Uruchomienie programu:**
   - Aby uruchomić symulację z domyślną liczbą filozofów (5), użyj:
     ```bash
     ./dpp
     ```
   - Aby uruchomić symulację z określoną liczbą filozofów, użyj flagi `-c` lub `--count`. Przykład dla 7 filozofów:
     ```bash
     ./dpp -c 7
     ```

## Opis problemu

**Problem ucztujących filozofów** to klasyczny problem synchronizacji w informatyce, który ilustruje wyzwania związane z przydzielaniem współdzielonych zasobów.

W opisywanym scenariuszu:

- **Filozofowie** siedzą przy okrągłym stole.
- Na stole znajduje się tyle **widelców**, ilu jest filozofów.
- Każdy filozof cyklicznie przechodzi przez trzy stany:
  - Myśli
  - Czeka na posiłek
  - Je
- Obok każdego filozofa znajdują się **dwa widelce** – jeden po lewej, drugi po prawej stronie.
- Aby rozpocząć jedzenie, filozof musi **jednocześnie podnieść oba widelce**.

Głównym wyzwaniem w tym problemie, jest zapobiegnięcie sytuacji, w której **wszyscy filozofowie jednocześnie podnoszą jeden widelec i oczekują na drugi**, co prowadzi do **zakleszczenia** (_deadlock_).

## Sekcje krytyczne i ich rozwiązanie

W projekcie wyróżniono następujące sekcje krytyczne:

- **Dostęp do konsoli (standardowe wyjście):**
  - Funkcje `think`, `hungry` oraz `eat` korzystają z mutexu `outMutex`, aby zagwarantować, że wypisywanie komunikatów na konsolę nie jest wykonywane jednocześnie przez wiele wątków. Zapobiega to powstawaniu nieczytelnych i pomieszanych komunikatów.
- **Dostęp do widelców:**
  - Każdy widelec jest reprezentowany przez oddzielny `mutex`.
  - Filozof, zanim zacznie jeść, podnosi dwa widelce. Kolejność podnoszenia jest ustalana na podstawie indeksów (mniejszy indeks jest blokowany pierwszy - hierarchia zasobów), co pomaga uniknąć sytuacji zakleszczenia.
  - Dzięki temu, gdy filozofowie próbują jednocześnie zdobyć widelce, system synchronizacji dba o to, by nie doszło do sytuacji, gdzie każdy z nich trzyma jeden widelec i czeka na drugi.

## Wątki

- **Reprezentacja filozofów:**
  - Każdy filozof jest reprezentowany przez oddzielny wątek, który wykonuje nieskończoną pętlę symulującą cykl życia filozofa (myślenie, czekanie na posiłek, jedzenie).
  - Dzięki użyciu wielowątkowości symulacja odzwierciedla równoczesne działania wszystkich filozofów oraz współdzielenie zasobów (widelców).

---

## Temat: Aplikacja czatu

## Opis aplikacji

Aplikacja CHaT to prosty system komunikacji tekstowej oparty na architekturze klient-serwer.

- **Serwer:**
  - Nasłuchuje na przychodzące połączenia od klientów.
  - Zarządza listą połączonych klientów.
  - Odbiera wiadomości od jednego klienta i rozgłasza je do wszystkich pozostałych połączonych klientów.
  - Zapisuje historię czatu do plików.
  - Wyświetla swój adres IP w sieci lokalnej (dla interfejsu `wlan0`), aby ułatwić klientom połączenie.

- **Klient:**
  - Łączy się z serwerem podając jego adres IP i port.
  - Wysyła wiadomości tekstowe do serwera.
  - Odbiera wiadomości od serwera (wysłane przez innych klientów) i wyświetla je na konsoli.
  - Komunikacja odbywa się za pomocą gniazd TCP/IP.

## Instrukcje uruchomienia

Zakładając, że repozytorium zostało już sklonowane zgodnie z instrukcjami dla pierwszego projektu i znajdujesz się w głównym katalogu `SO2`.

1.  **Nawigacja do katalogu aplikacji czatu:**
    ```bash
    cd CHaT
    ```

2.  **Kompilacja serwera:**
    - Upewnij się, że masz zainstalowany kompilator `g++`.
    - Przejdź do katalogu serwera:
      ```bash
      cd Server
      ```
    - Skorzystaj z dołączonego pliku Makefile, wpisując w terminal:
      ```bash
      make
      ```
    - Wróć do głównego katalogu aplikacji czatu:
      ```bash
      cd ..
      ```

3.  **Kompilacja klienta:**
    - Upewnij się, że masz zainstalowany kompilator `g++`.
    - Przejdź do katalogu klienta:
      ```bash
      cd Client
      ```
    - Skorzystaj z dołączonego pliku Makefile, wpisując w terminal:
      ```bash
      make
      ```
    - Wróć do głównego katalogu aplikacji czatu:
      ```bash
      cd ..
      ```

4.  **Uruchomienie serwera:**
    - Otwórz nowy terminal.
    - Przejdź do katalogu `CHaT/server`:
      ```bash
      cd CHaT/Server
      ```
    - Uruchom serwer:
      ```bash
      ./server
      ```
    - Serwer powinien wyświetlić swój adres IP i port, na którym nasłuchuje.

5.  **Uruchomienie klienta:**
    - Otwórz nowy terminal.
    - Przejdź do katalogu `CHaT/Client`:
      ```bash
      cd CHaT/Client
      ```
    - Uruchom klienta, podając adres IP i port serwera wyświetlony w konsoli serwera:
      ```bash
      ./client

## Sekcje krytyczne i ich rozwiązanie

W projekcie CHaT zidentyfikowano następujące sekcje krytyczne:

- **Dostęp do listy klientów na serwerze (`clients`):**
  - Serwer przechowuje listę połączonych klientów w strukturze `std::vector<Client> clients`.
  - Dostęp do tej listy (dodawanie nowego klienta, usuwanie klienta, iterowanie w celu rozgłaszania wiadomości) jest chroniony przez `std::mutex clientsMutex`. Zapobiega to race condition, gdy wiele wątków próbuje modyfikować lub odczytywać listę jednocześnie.

- **Zapis logów z czatu do pliku (`saveMessageToFile`):**
  - Funkcja `saveMessageToFile` w `Utils.cpp` jest odpowiedzialna za zapisywanie wiadomości do pliku.
  - Aby zapewnić, że zapisy z różnych wątków nie będą się przeplatać i uszkadzać pliku logu, używany jest `std::mutex fileMutex`. Gwarantuje to, że tylko jeden wątek na raz może zapisywać do pliku.

- **Kolejka wiadomości u klienta (`messageQueue`):**
  - Klient używa `std::queue<std::string> messageQueue` do przechowywania wiadomości odebranych z serwera, zanim zostaną one wyświetlone użytkownikowi.
  - Dostęp do tej kolejki jest synchronizowany za pomocą `std::mutex queueMutex` oraz `std::condition_variable queueCond`.
  - Wątek odbierający wiadomości (`messageReceiver`) dodaje wiadomości do kolejki, a wątek drukujący (`messagePrinter`) je z niej pobiera. Mutex chroni kolejkę przed jednoczesnym dostępem, a zmienna warunkowa pozwala wątkowi drukującemu efektywnie czekać na nowe wiadomości bez aktywnego odpytywania.

## Wątki

- **Serwer:**
  - **Główny wątek serwera:** Odpowiedzialny za inicjalizację gniazda serwera, nasłuchiwanie na przychodzące połączenia i akceptowanie ich.
  - **Wątki obsługi klienta (`handleClient`):** Dla każdego zaakceptowanego połączenia klienta, serwer tworzy nowy wątek. Ten wątek jest dedykowany do obsługi komunikacji z tym konkretnym klientem – odbierania od niego wiadomości i inicjowania procesu rozgłaszania. Każdy taki wątek działa niezależnie.

- **Klient:**
  - **Główny wątek klienta:** Odpowiedzialny za pobieranie danych wejściowych od użytkownika (nazwa użytkownika, IP serwera, port), inicjalizację połączenia z serwerem oraz pobieranie wiadomości od użytkownika i wysyłanie ich do serwera.
  - **Wątek odbierający wiadomości (`messageReceiver`):** Działa w tle, ciągle nasłuchując na wiadomości przychodzące od serwera. Po odebraniu wiadomości, dodaje ją do współdzielonej kolejki `messageQueue`.
  - **Wątek drukujący wiadomości (`messagePrinter`):** Działa w tle, monitoruje kolejkę `messageQueue`. Gdy w kolejce pojawią się nowe wiadomości, pobiera je i wyświetla na konsoli użytkownika.

---
