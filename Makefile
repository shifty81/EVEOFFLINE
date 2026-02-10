# EVE OFFLINE - Makefile
# Provides easy commands for common development tasks

# Detect OS
ifeq ($(OS),Windows_NT)
    RM := del /Q
    RMDIR := rmdir /S /Q
else
    RM := rm -f
    RMDIR := rm -rf
endif

# Default target
.DEFAULT_GOAL := help

.PHONY: help
help: ## Show this help message
	@echo "EVE OFFLINE - Development Commands"
	@echo "==================================="
	@echo ""
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""

.PHONY: build
build: ## Build both client and server (Release)
	./build.sh Release

.PHONY: build-debug
build-debug: ## Build both client and server (Debug)
	./build.sh Debug

.PHONY: build-client
build-client: ## Build C++ client only
	@mkdir -p cpp_client/build
	cd cpp_client/build && cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON && cmake --build . --config Release

.PHONY: build-server
build-server: ## Build C++ server only
	@mkdir -p cpp_server/build
	cd cpp_server/build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . --config Release

.PHONY: clean
clean: ## Clean all build artifacts
	$(RMDIR) build 2>/dev/null || true
	$(RMDIR) cpp_client/build 2>/dev/null || true
	$(RMDIR) cpp_client/build_vs 2>/dev/null || true
	$(RMDIR) cpp_server/build 2>/dev/null || true
	$(RMDIR) build_vs 2>/dev/null || true
	find . -type d -name "__pycache__" -exec $(RMDIR) {} + 2>/dev/null || true
	find . -type f -name "*.pyc" -delete 2>/dev/null || true

.PHONY: check-deps
check-deps: ## Check if build dependencies are installed
	@echo "Checking build dependencies..."
	@command -v cmake >/dev/null 2>&1 && echo "  ✓ CMake" || echo "  ✗ CMake not found"
	@command -v g++ >/dev/null 2>&1 && echo "  ✓ g++" || (command -v clang++ >/dev/null 2>&1 && echo "  ✓ clang++" || echo "  ✗ No C++ compiler found")
	@pkg-config --exists glfw3 2>/dev/null && echo "  ✓ GLFW3" || echo "  ? GLFW3 (may be available via vcpkg)"
	@pkg-config --exists glew 2>/dev/null && echo "  ✓ GLEW" || echo "  ? GLEW (may be available via vcpkg)"
	@pkg-config --exists glm 2>/dev/null && echo "  ✓ GLM" || echo "  ? GLM (may be available via vcpkg)"

.PHONY: docs
docs: ## Show documentation location
	@echo "Documentation is in docs/ folder:"
	@echo ""
	@echo "  docs/guides/       - Build & setup guides"
	@echo "  docs/cpp_client/   - C++ client documentation"
	@echo "  docs/sessions/     - Development session notes"
	@echo ""
	@ls -1 docs/*.md 2>/dev/null || true

.PHONY: test
test: test-server ## Run all tests

.PHONY: test-server
test-server: ## Build and run C++ server tests
	@mkdir -p cpp_server/build
	cd cpp_server/build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DUSE_STEAM_SDK=OFF && cmake --build . --config Release --target test_systems -j$$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
	cd cpp_server/build && ./bin/test_systems

.PHONY: all
all: clean build ## Clean and build everything
