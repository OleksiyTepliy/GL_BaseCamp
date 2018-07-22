TARGET = mylist

DEPS = lib
DEPS := $(addsuffix .o, $(DEPS))

CC = gcc
CFLAGS = -Wall

.PHONY: all clean

all:	$(TARGET)

$(TARGET): $(DEPS)
	$(CC) $(CFLAGS) $(addsuffix .c, $(TARGET)) $(DEPS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	@echo Tidying things up...
	-rm -f *.o $(TARGET)
