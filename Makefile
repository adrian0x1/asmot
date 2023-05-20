CC=gcc
CFLAGS=-g -Wall -Wno-unused
BUILD_DIR=build
SRC_DIR=src
TARGET=$(BUILD_DIR)/asmot
LIBRARIES=-lcpuid -lpthread
C_FILES=$(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c)
OBJ_FILES=$(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(C_FILES)))
RM=rm

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $@ $(LIBRARIES)

$(BUILD_DIR)/%.o: $(filter %/$(notdir $(basename $@))%.c, $(C_FILES))
	$(CC) $(CFLAGS) -c $(filter %/$(notdir $(basename $@)).c, $(C_FILES)) -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	@$(RM) $(BUILD_DIR)/*
