TARGET = vm_riskxvii

CC = gcc

CFLAGS     = -c -Wall -Wvla -Werror -O0 -O3 -Os -g -std=c11
SRC        = vm_riskxvii.c decode.c virtual_routines.c
OBJ        = $(SRC:.c=.o)

all:$(TARGET)
	strip -s vm_riskxvii

$(TARGET):$(OBJ)
	$(CC) -o $@ $(OBJ)

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $<

run:
	./$(TARGET)

test:
	bash test_script.sh

run_tests:
	bash test_script.sh

clean:
	rm -f *.o *.obj $(TARGET)
