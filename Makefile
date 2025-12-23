# Effective C++ 3rd Edition - Practice Environment
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Build directory
BUILD_DIR = build

# Find all .cpp files in src directories
SOURCES = $(shell find src -name "*.cpp" 2>/dev/null)

# Default target: show help
.PHONY: help
help:
	@echo "Effective C++ 3rd Edition - Practice Environment"
	@echo ""
	@echo "Usage:"
	@echo "  make run FILE=src/ch01/item01.cpp   # Compile and run a specific file"
	@echo "  make build FILE=src/ch01/item01.cpp # Compile only"
	@echo "  make clean                          # Remove build artifacts"
	@echo "  make list                           # List all source files"
	@echo ""

# Compile and run a single file
.PHONY: run
run:
ifndef FILE
	@echo "Error: FILE is not specified"
	@echo "Usage: make run FILE=src/ch01/item01.cpp"
	@exit 1
endif
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/a.out $(FILE)
	@echo "--- Running $(FILE) ---"
	@$(BUILD_DIR)/a.out

# Compile only
.PHONY: build
build:
ifndef FILE
	@echo "Error: FILE is not specified"
	@echo "Usage: make build FILE=src/ch01/item01.cpp"
	@exit 1
endif
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/a.out $(FILE)
	@echo "Built: $(BUILD_DIR)/a.out"

# List all source files
.PHONY: list
list:
	@echo "Source files:"
	@find src -name "*.cpp" 2>/dev/null | sort || echo "  (no files yet)"

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaned build directory"
