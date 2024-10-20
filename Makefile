CC = gcc
CFLAGS = -Wall -O2
LDFLAGS =

SRC_DIR = src
OBJ_DIR = obj

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

all: ipc_test

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

ipc_test: $(OBJECTS)
	$(CC) $(CFLAGS) -o ipc_test $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(OBJ_DIR)/*.o ipc_test
