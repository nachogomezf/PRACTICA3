CC=gcc
FLAGS=-g -Wall -Werror
OBJ= queue costCalculator
LIBS= -pthread

all:  $(OBJ)
	@echo "***************************"
	@echo "Compilation successfully!"
	@echo ""

queue: queue.c
	$(CC) -c queue.c

costCalculator:	costCalculator.c
	$(CC) $(CFLAGS) $(LIBS) -o calculator  costCalculator.c queue.c

load:
	ld -o calculator queue.o

clean:
	rm -f calculator process *.o
	@echo "***************************"
	@echo "Deleted files!"
	@echo  ""

