# Owner: Seunggyun (Person A)
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
SRC      := $(wildcard src/*.cpp)
OBJ      := $(SRC:src/%.cpp=build/%.o)
TARGET   := atm

BANK_SRC := src/Card.cpp src/Account.cpp src/Bank.cpp src/SystemLoader.cpp

.PHONY: all run test-bank clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

build:
	mkdir -p build

run: $(TARGET)
	./$(TARGET) data/initial_conditions.txt

# Bank side only (Person A), runs without the other modules.
test-bank: $(BANK_SRC) tests/bank_side_test.cpp
	$(CXX) $(CXXFLAGS) -o build_test_bank $^
	./build_test_bank

clean:
	rm -rf build $(TARGET) build_test_bank

-include $(OBJ:.o=.d)
