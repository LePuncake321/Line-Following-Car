# Variables
CC = gcc
CFLAGS = -Wall -pthread -I./include  # Example flags, adjust as needed
LIBS = -lm -lrt -lpthread -lpigpio # Example libraries, adjust as needed

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = bin

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

# Executable
EXEC = car

# Default target
all: $(EXEC)

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

# Link executable
$(EXEC): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(BUILD_DIR)/$@ $(LIBS)

# Run the program
run: $(EXEC)
	sudo ./$(BUILD_DIR)/$(EXEC)

# Clean up
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/$(EXEC)

.PHONY: all clean
