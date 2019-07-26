SRC=./src

ae: $(SRC)/ae.c
	$(CC) $(SRC)/ae.c -o ae -lcurses -lreadline -Wall -Wextra -pedantic -std=c99
