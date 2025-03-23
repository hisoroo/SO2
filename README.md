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
