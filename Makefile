CC=gcc
CFLAGS=-g -Wall -Wno-unused
BUILD_DIR=build
SRC_DIR=src
TARGET=$(BUILD_DIR)/asmot
LIBRARIES=-lcpuid -lpthread
C_FILES=$(wildcard $(SRC_DIR)/*.c)
OBJ_FILES=$(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_FILES))
RM=rm

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $@ $(LIBRARIES)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@


run: $(TARGET)
	./$(TARGET)

clean:
	@$(RM) $(BUILD_DIR)/