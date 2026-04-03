CC = gcc
FLAGS = -Wall -g
TARGET = monitor
SRC = src/main.c src/scanner.c src/backup.c

all:
	$(CC) $(FLAGS) $(SRC) -o $(TARGET)
	@echo "build done"

clean:
	rm -f $(TARGET)
	@echo "cleaned"
