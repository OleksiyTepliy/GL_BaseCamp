TARGET=main
DEPS=det
DEPS:=$(addsuffix .o, $(DEPS))

CC=gcc
CFLAGS=-Wall -lm

all:$(TARGET)

$(TARGET): $(DEPS)
	$(CC) $(CFLAGS) $(addsuffix .c, $(TARGET)) $(DEPS) -o $@
	
%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	@echo Tidying things up...
	-rm -f *.o $(TARGET)
