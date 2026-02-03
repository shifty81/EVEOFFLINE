# EVE OFFLINE - Makefile
# Provides easy commands for common development tasks

# Detect OS
ifeq ($(OS),Windows_NT)
    PYTHON := python
    RM := del /Q
    RMDIR := rmdir /S /Q
else
    PYTHON := python3
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

.PHONY: install
install: ## Install all dependencies
	$(PYTHON) -m pip install -r requirements.txt

.PHONY: install-dev
install-dev: ## Install development dependencies
	$(PYTHON) -m pip install -r requirements.txt
	$(PYTHON) -m pip install pylint flake8 pytest black

.PHONY: build
build: ## Run full build and test
	$(PYTHON) build_and_test.py

.PHONY: quick
quick: ## Run quick build (fast tests only)
	$(PYTHON) build_and_test.py --quick

.PHONY: test
test: ## Run all tests
	$(PYTHON) automated_tests.py

.PHONY: test-quick
test-quick: ## Run quick tests
	$(PYTHON) automated_tests.py --quick

.PHONY: test-ui
test-ui: ## Run UI tests
	$(PYTHON) test_eve_ui_components.py

.PHONY: lint
lint: ## Run code quality checks
	$(PYTHON) build_and_test.py --lint

.PHONY: format
format: ## Format code with black
	$(PYTHON) -m black engine client server *.py

.PHONY: clean
clean: ## Clean build artifacts and cache
	find . -type d -name "__pycache__" -exec $(RMDIR) {} + 2>/dev/null || true
	find . -type f -name "*.pyc" -delete 2>/dev/null || true
	find . -type f -name "*.pyo" -delete 2>/dev/null || true
	find . -type d -name "*.egg-info" -exec $(RMDIR) {} + 2>/dev/null || true
	$(RM) .coverage 2>/dev/null || true

.PHONY: server
server: ## Start the game server
	$(PYTHON) server/server.py

.PHONY: client
client: ## Start the text client
	$(PYTHON) client/client.py "TestPilot"

.PHONY: gui
gui: ## Start the 2D GUI client
	$(PYTHON) client/gui_client.py "TestPilot"

.PHONY: client-3d
client-3d: ## Start the 3D client
	$(PYTHON) client_3d.py "TestPilot"

.PHONY: demo
demo: ## Run interactive demo
	$(PYTHON) interactive_demo.py

.PHONY: gui-demo
gui-demo: ## Run GUI demo
	$(PYTHON) gui_demo.py

.PHONY: check-deps
check-deps: ## Check if all dependencies are installed
	@echo "Checking dependencies..."
	@$(PYTHON) -c "import sys; print('Python:', sys.version)" || echo "Python not found!"
	@$(PYTHON) -c "import pygame; print('Pygame:', pygame.version.ver)" || echo "Pygame not installed"
	@$(PYTHON) -c "import panda3d; print('Panda3D: OK')" || echo "Panda3D not installed"

.PHONY: venv
venv: ## Create virtual environment
	$(PYTHON) -m venv venv
	@echo "Virtual environment created. Activate with:"
	@echo "  Windows: venv\\Scripts\\activate"
	@echo "  Linux/Mac: source venv/bin/activate"

.PHONY: docs
docs: ## Generate documentation
	@echo "Documentation is in docs/ folder"
	@echo "Main docs:"
	@ls -1 docs/*.md 2>/dev/null || dir docs\\*.md

.PHONY: git-pull
git-pull: ## Pull latest changes and rebuild
	git pull
	$(PYTHON) build_and_test.py

.PHONY: pre-commit
pre-commit: ## Run pre-commit checks
	$(PYTHON) build_and_test.py --quick
	@echo "Pre-commit checks complete!"

.PHONY: all
all: clean install build ## Clean, install, and build everything
