CXX ?= g++

SRC_DIR = $(shell pwd)/src

CPP_FILES = $(wildcard src/*.cpp)

OBJ_FILES = $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))

CC_FLAGS = -Wall -std=c++11 -fshort-enums -fno-exceptions -flto -march=native -Ofast

LD_FLAGS = -flto

OBJ_DIR = obj

BIN_NAME = skaak

all: $(OBJ_DIR) $(BIN_NAME)

$(BIN_NAME): $(OBJ_FILES)
	$(CXX) $(LD_FLAGS) -o $@ $^

obj/%.o: src/%.cpp
	$(CXX) $(CC_FLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(BIN_NAME)



