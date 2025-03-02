SRC_DIR_MKPWD := src
BIN_DIR := bin

TARGET_MKPWD := $(BIN_DIR)/mkpwd

SRC_MKPWD := $(wildcard $(SRC_DIR_MKPWD)/*.c)

OBJ_MKPWD := $(patsubst %.c,%.o,$(SRC_MKPWD))

CC := gcc
CFLAGS := -Wall -g

all: $(TARGET_MKPWD)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(TARGET_MKPWD): $(OBJ_MKPWD) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJ_MKPWD)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJ_MKPWD) $(TARGET_MKPWD)

.PHONY: all clean

