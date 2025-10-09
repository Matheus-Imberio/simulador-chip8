CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

SRC = main.cpp c8vm.cpp
OBJ = $(SRC:.cpp=.o)
BIN = chip8

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJ) $(BIN)