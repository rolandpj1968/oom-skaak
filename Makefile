CXX ?= g++

SRC_DIR = $(shell pwd)/src

CPP_FILES = $(wildcard src/*.cpp)

HPP_FILES = $(wildcard src/*.hpp)

OBJ_FILES = $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))

CC_FLAGS = -Wall -std=c++11 -fshort-enums -fno-exceptions -flto -march=native -Ofast -I$(SRC_DIR)

LD_FLAGS = -flto

OBJ_DIR = obj

SKAAK_CPP_FILES = $(wildcard src/chess/*.cpp)
SKAAP_OBJ_FILES = $(addprefix obj/,$(notdir $(SKAAK_CPP_FILES:.cpp=.o)))

SKAAK_BIN_NAME = skaak

all: $(OBJ_DIR) $(SKAAK_BIN_NAME)

$(SKAAK_BIN_NAME): $(SKAAP_OBJ_FILES) $(OBJ_FILES)
	$(CXX) $(LD_FLAGS) -o $@ $^

obj/%.o: src/%.cpp $(HPP_FILES)
	$(CXX) $(CC_FLAGS) -c -o $@ $<

obj/%.o: src/chess/%.cpp $(HPP_FILES)
	$(CXX) $(CC_FLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(SKAAK_BIN_NAME)



