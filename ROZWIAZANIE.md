# Kompilowanie rozwiązania
## Wymagania
Do skompilowania potrzebne są biblioteka `ncurses` oraz `cmake`. Wersje dostępne
są w repozytoriach Debiana.
```
sudo apt-get update && sudo apt-get -y install libncurses-dev cmake
```

## Kompilacja
W katalogu projektu zawierającym plik `CMakeLists.txt` należy stworzyć katalog
`build` i uruchomić `cmake` a następnie `make`.
```
mkdir build && cd $_
cmake ..
make
```

# Opis rozwiązania
## Streszczenie
Emulator po uruchomieniu uruchamia nowy proces z emulowanym programem i podłącza
się do niego poprzez `ptrace`. Za każdym razem gdy proces emulowany uruchamia
__wywołanie sytemowe__ emulator przechwytuje je i symuluje jego wykonanie.

## Uruchominie procesu emulowanego
Proces emulowany uruchamiany jest po `fork`u standardowym `execve`. Jako, że
wcześniej wywołał polecenie `PTRACE_TRACEME` po załadowaniu do pamięci zostaje
natychmiastowo wstrzymany i oczekuje na wznowienie przez emulator. Zanim to
nastąpi emulator otwiera deskryptor do pamięci procesu emulowanego, wstawia w
odpowiednie miejsce argumenty i wznawia proces oczekując na jego __wywołania
systemowe__.

## Podział na pliki
_Poszczególne funkcje zostały opisane w plikach nagłówkowych._

### AlienOS
Pliki w katalogu `src/alienos` symulują działanie systemu AlienOS.
* `graphics` jest plikiem pomocniczym opakowującym funkcję z biblioteki
 `ncurses` w funkcje użyteczne dla interfejsu graficznego systemu AlienOS
* `alienos` zawiera implementacje wszystkich wywołań systemowych systemu
 AlienOS. Nagłówki funkcji różnią się od tych z sytemu AlienOS zwracanym
 typem (`int` zamiast `void`), aby móc zasygnalizować błąd wykonania.

### System
Pliki w katalogu `src/system` nie są bezpośrednio związane z emulacją.
* `convert` implementuje funkcję zamiany napisu na liczbę całkowitą z obsługą
 błędów
* `memory` implementuje funkcję otwierającą deskryptor do pamięci procesu
* `random` implementuje brakującą funkcję `getrandom` (wywołanie systemowe
  wprowadzone w nowszych kernelach).

### Emulator
Pliki w katalogu `src/emulator` są bezpośrednio odpowiedzialne za cały proces
emulacji.
* `emulator` odpowiada za poprawne uruchomienie procesu emulowanego,
 wpisania parametrów i reagowania jego na __wywołania systemowe__.
* `params` jest odpowiedzialny za parse'owanie pliku ELF i znalezienie segmentu
 `PT_PARAMS`, jeżeli istnieje.
* `syscall` pozwala na emulowanie zachowania __wywołań systemowych__ procesu
 emulowanego.

### Main
Plik `main.c` odpowiada za `fork`, by uruchomić proces emulowany i start
emulatora.
