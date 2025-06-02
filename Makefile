# MultiButton Library Makefile
# 支持编译库文件和示例程序

# Compiler and tools
CC = gcc
AR = ar
RM = rm -f
MKDIR = mkdir -p

# Project directories
SRC_DIR = .
EXAMPLES_DIR = examples
BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/lib
BIN_DIR = $(BUILD_DIR)/bin
OBJ_DIR = $(BUILD_DIR)/obj

# Compiler flags
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
INCLUDES = -I$(SRC_DIR)
LDFLAGS = 
LIBS = 

# Source files
LIB_SOURCES = multi_button.c
LIB_OBJECTS = $(addprefix $(OBJ_DIR)/, $(LIB_SOURCES:.c=.o))

# Library name
LIB_NAME = libmultibutton
STATIC_LIB = $(LIB_DIR)/$(LIB_NAME).a
SHARED_LIB = $(LIB_DIR)/$(LIB_NAME).so

# Example programs
EXAMPLES = basic_example advanced_example poll_example

# Default target
all: library examples

# Create directories
$(BUILD_DIR):
	$(MKDIR) $(BUILD_DIR)

$(LIB_DIR): $(BUILD_DIR)
	$(MKDIR) $(LIB_DIR)

$(BIN_DIR): $(BUILD_DIR)
	$(MKDIR) $(BIN_DIR)

$(OBJ_DIR): $(BUILD_DIR)
	$(MKDIR) $(OBJ_DIR)

# Build object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(EXAMPLES_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Build static library
$(STATIC_LIB): $(LIB_OBJECTS) | $(LIB_DIR)
	$(AR) rcs $@ $^
	@echo "Static library created: $@"

# Build shared library
$(SHARED_LIB): $(LIB_OBJECTS) | $(LIB_DIR)
	$(CC) -shared -fPIC $(CFLAGS) $(INCLUDES) $(LIB_SOURCES) -o $@
	@echo "Shared library created: $@"

# Library target
library: $(STATIC_LIB)

# Shared library target
shared: $(SHARED_LIB)

# Example programs
basic_example: $(BIN_DIR)/basic_example
$(BIN_DIR)/basic_example: $(OBJ_DIR)/basic_example.o $(STATIC_LIB) | $(BIN_DIR)
	$(CC) $< -L$(LIB_DIR) -lmultibutton -o $@
	@echo "Example program created: $@"

advanced_example: $(BIN_DIR)/advanced_example
$(BIN_DIR)/advanced_example: $(OBJ_DIR)/advanced_example.o $(STATIC_LIB) | $(BIN_DIR)
	$(CC) $< -L$(LIB_DIR) -lmultibutton -o $@
	@echo "Example program created: $@"

poll_example: $(BIN_DIR)/poll_example
$(BIN_DIR)/poll_example: $(OBJ_DIR)/poll_example.o $(STATIC_LIB) | $(BIN_DIR)
	$(CC) $< -L$(LIB_DIR) -lmultibutton -o $@
	@echo "Example program created: $@"

# Build all examples
examples: $(addprefix $(BIN_DIR)/, $(EXAMPLES))

# Test target
test: examples
	@echo "Running basic example..."
	@cd $(BIN_DIR) && ./basic_example

# Clean build files
clean:
	$(RM) -r $(BUILD_DIR)
	@echo "Build directory cleaned"

# Install library (optional)
install: library
	@echo "Installing library to /usr/local/lib..."
	sudo cp $(STATIC_LIB) /usr/local/lib/
	sudo cp multi_button.h /usr/local/include/
	sudo ldconfig

# Uninstall library
uninstall:
	sudo $(RM) /usr/local/lib/$(LIB_NAME).a
	sudo $(RM) /usr/local/include/multi_button.h

# Show help
help:
	@echo "MultiButton Library Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build library and examples (default)"
	@echo "  library      - Build static library only"
	@echo "  shared       - Build shared library"
	@echo "  examples     - Build all examples"
	@echo "  basic_example     - Build basic example"
	@echo "  advanced_example  - Build advanced example"
	@echo "  poll_example      - Build poll example"
	@echo "  test         - Build and run basic test"
	@echo "  clean        - Remove build directory"
	@echo "  install      - Install library to system"
	@echo "  uninstall    - Remove library from system"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Build configuration:"
	@echo "  CC           = $(CC)"
	@echo "  CFLAGS       = $(CFLAGS)"
	@echo "  BUILD_DIR    = $(BUILD_DIR)"

# Print build info
info:
	@echo "Project: MultiButton Library"
	@echo "Sources: $(LIB_SOURCES)"
	@echo "Examples: $(EXAMPLES)"
	@echo "Build directory: $(BUILD_DIR)"
	@echo "Compiler: $(CC)"
	@echo "Flags: $(CFLAGS)"

# Phony targets
.PHONY: all library shared examples clean install uninstall help info test basic_example advanced_example poll_example

# Dependencies
$(OBJ_DIR)/multi_button.o: multi_button.c multi_button.h
$(OBJ_DIR)/basic_example.o: $(EXAMPLES_DIR)/basic_example.c multi_button.h
$(OBJ_DIR)/advanced_example.o: $(EXAMPLES_DIR)/advanced_example.c multi_button.h
$(OBJ_DIR)/poll_example.o: $(EXAMPLES_DIR)/poll_example.c multi_button.h 