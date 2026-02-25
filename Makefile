# MultiButton Library – Makefile
# Build static/shared library and example programs.

# ── Toolchain ────────────────────────────────────────────────────────────────
CC      = gcc
AR      = ar
RM      = rm -f
MKDIR   = mkdir -p

# ── Project layout ───────────────────────────────────────────────────────────
INC_DIR      = include
SRC_DIR      = src
EXAMPLES_DIR = examples
BUILD_DIR    = build
LIB_DIR      = $(BUILD_DIR)/lib
BIN_DIR      = $(BUILD_DIR)/bin
OBJ_DIR      = $(BUILD_DIR)/obj

# ── Compiler / linker flags ──────────────────────────────────────────────────
CFLAGS  = -Wall -Wextra -Wpedantic -std=c99 -O2 -g
INCLUDES = -I$(INC_DIR)

# Optional: user-supplied config override, e.g.
#   make EXTRA_CFLAGS='-DBTN_USER_CFG_FILE=\"my_cfg.h\"'
CFLAGS += $(EXTRA_CFLAGS)

# ── Library sources ──────────────────────────────────────────────────────────
LIB_SRCS = $(SRC_DIR)/multi_button.c
LIB_OBJS = $(OBJ_DIR)/multi_button.o

LIB_NAME   = libmultibutton
STATIC_LIB = $(LIB_DIR)/$(LIB_NAME).a
SHARED_LIB = $(LIB_DIR)/$(LIB_NAME).so

# ── Example programs ─────────────────────────────────────────────────────────
EXAMPLES = basic_example advanced_example poll_example

# ── Default target ───────────────────────────────────────────────────────────
.PHONY: all library shared examples clean install uninstall help info test \
        $(EXAMPLES)

all: library examples

# ── Directory creation ───────────────────────────────────────────────────────
$(BUILD_DIR) $(LIB_DIR) $(BIN_DIR) $(OBJ_DIR):
	$(MKDIR) $@

# ── Object compilation rules ────────────────────────────────────────────────
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(EXAMPLES_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# ── Static library ──────────────────────────────────────────────────────────
$(STATIC_LIB): $(LIB_OBJS) | $(LIB_DIR)
	$(AR) rcs $@ $^
	@echo "  AR    $@"

library: $(STATIC_LIB)

# ── Shared library ──────────────────────────────────────────────────────────
$(SHARED_LIB): $(LIB_SRCS) | $(LIB_DIR)
	$(CC) -shared -fPIC $(CFLAGS) $(INCLUDES) $^ -o $@
	@echo "  SO    $@"

shared: $(SHARED_LIB)

# ── Example programs (each links against the static lib) ────────────────────
define EXAMPLE_RULE
$(1): $(BIN_DIR)/$(1)
$(BIN_DIR)/$(1): $(OBJ_DIR)/$(1).o $(STATIC_LIB) | $(BIN_DIR)
	$(CC) $$< -L$(LIB_DIR) -lmultibutton -o $$@
	@echo "  LD    $$@"
endef

$(foreach ex,$(EXAMPLES),$(eval $(call EXAMPLE_RULE,$(ex))))

examples: $(addprefix $(BIN_DIR)/,$(EXAMPLES))

# ── Test / run ──────────────────────────────────────────────────────────────
test: examples
	@echo "=== Running basic_example ==="
	@$(BIN_DIR)/basic_example

# ── Install / uninstall ─────────────────────────────────────────────────────
PREFIX ?= /usr/local

install: library
	install -d $(PREFIX)/lib $(PREFIX)/include
	install -m 644 $(STATIC_LIB)               $(PREFIX)/lib/
	install -m 644 $(INC_DIR)/multi_button.h     $(PREFIX)/include/
	install -m 644 $(INC_DIR)/multi_button_cfg.h $(PREFIX)/include/
	@echo "Installed to $(PREFIX)"

uninstall:
	$(RM) $(PREFIX)/lib/$(LIB_NAME).a
	$(RM) $(PREFIX)/include/multi_button.h
	$(RM) $(PREFIX)/include/multi_button_cfg.h

# ── Clean ───────────────────────────────────────────────────────────────────
clean:
	$(RM) -r $(BUILD_DIR)
	@echo "Build directory cleaned."

# ── Help ────────────────────────────────────────────────────────────────────
help:
	@echo "MultiButton Library Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all              Build library + examples (default)"
	@echo "  library          Build static library only"
	@echo "  shared           Build shared library"
	@echo "  examples         Build all example programs"
	@echo "  basic_example    Build basic example"
	@echo "  advanced_example Build advanced example"
	@echo "  poll_example     Build polling example"
	@echo "  test             Build and run basic_example"
	@echo "  clean            Remove build directory"
	@echo "  install          Install to PREFIX (default /usr/local)"
	@echo "  uninstall        Remove installed files"
	@echo "  help             Show this message"
	@echo ""
	@echo "Variables:"
	@echo "  EXTRA_CFLAGS     Extra compiler flags"
	@echo "  PREFIX           Install prefix (default /usr/local)"
	@echo ""
	@echo "Example – custom config:"
	@echo "  make EXTRA_CFLAGS='-DBTN_USER_CFG_FILE=\"my_cfg.h\"'"

info:
	@echo "CC       = $(CC)"
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "INCLUDES = $(INCLUDES)"
	@echo "LIB_SRCS = $(LIB_SRCS)"
	@echo "EXAMPLES = $(EXAMPLES)"

# ── Explicit header dependencies ────────────────────────────────────────────
$(OBJ_DIR)/multi_button.o:    $(SRC_DIR)/multi_button.c     $(INC_DIR)/multi_button.h $(INC_DIR)/multi_button_cfg.h
$(OBJ_DIR)/basic_example.o:   $(EXAMPLES_DIR)/basic_example.c   $(INC_DIR)/multi_button.h $(INC_DIR)/multi_button_cfg.h
$(OBJ_DIR)/advanced_example.o:$(EXAMPLES_DIR)/advanced_example.c $(INC_DIR)/multi_button.h $(INC_DIR)/multi_button_cfg.h
$(OBJ_DIR)/poll_example.o:    $(EXAMPLES_DIR)/poll_example.c    $(INC_DIR)/multi_button.h $(INC_DIR)/multi_button_cfg.h
