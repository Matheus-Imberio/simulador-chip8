CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

# SDL2 flags via pkg-config (portable)
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL_LIBS   := $(shell pkg-config --libs sdl2 2>/dev/null)

SRC = src/main.cpp src/c8vm.cpp
OBJ = src/main.o src/c8vm.o
BIN = chip8

.PHONY: all run clean deps

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(SDL_LIBS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Run with: make run ROM=roms/rom.ch8 ARGS="--clock 700 --scale 15"
run: $(BIN)
	@if [ -z "$(ROM)" ]; then \
		echo "Uso: make run ROM=roms/arquivo.ch8 [ARGS=\"--clock 700 --scale 15\"]"; \
		exit 1; \
	fi; \
	./$(BIN) $(ARGS) "$(ROM)"

# Attempt to install deps (Debian/Ubuntu). On Arch/Manjaro, install: pacman -S sdl2 pkgconf
deps:
	@echo "Instale as dependências conforme seu SO:" && \
	echo "- Debian/Ubuntu: sudo apt update && sudo apt install build-essential pkg-config libsdl2-dev" && \
	echo "- Arch/Manjaro: sudo pacman -S --needed base-devel pkgconf sdl2"

clean:
	rm -f $(OBJ) $(BIN)