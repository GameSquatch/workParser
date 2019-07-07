TARGET = burEdit
CC = g++
CFLAGS = -I
DEPS = Segment.h DBF.h
OBJ = Segment.o DBF.o main.o

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJ)