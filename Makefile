CXX ?= g++

SRC_DIR = $(shell pwd)/src

CPP_FILES = $(wildcard src/*.cpp)

HPP_FILES = $(wildcard src/*.hpp)

OBJ_FILES = $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))

# don't seem to get much better than plain old -O3 - except for -finline-limit=2000

# debug
#CC_FLAGS = -Wall -std=c++11 -fshort-enums -fno-rtti -finline-limit=2000 -flto -march=native -g -I$(SRC_DIR) -pthread

CC_FLAGS = -Wall -Wuninitialized -std=c++11 -fshort-enums -finline-limit=1000 -flto -march=native -O3 -I$(SRC_DIR) -pthread
#CC_FLAGS = -Wall -Wuninitialized -std=c++11 -fshort-enums -fno-rtti -finline-limit=2000 -flto -march=native -O3 -I$(SRC_DIR) -pthread
#CC_FLAGS = -Wall -Wuninitialized -std=c++11 -fshort-enums -fno-rtti -flto -march=native -O3 -I$(SRC_DIR)
#CC_FLAGS = -Wall -std=c++11 -fshort-enums -fno-exceptions -fno-rtti -finline-limit=2000 -flto -march=native -Ofast -I$(SRC_DIR)
#CC_FLAGS = -fprofile-generate -Wall -std=c++11 -fshort-enums -fno-exceptions -fno-rtti -finline-limit=2000 -flto -march=native -Ofast -I$(SRC_DIR)
#CC_FLAGS = -fprofile-use -Wall -std=c++11 -fshort-enums -fno-exceptions -fno-rtti -finline-limit=2000 -flto -march=native -Ofast -I$(SRC_DIR)

# debug
#LD_FLAGS = -fshort-enums -fno-rtti -finline-limit=2000 -flto -march=native -g -pthread

LD_FLAGS = -fshort-enums -finline-limit=1000 -flto -march=native -O3 -pthread
#LD_FLAGS = -fshort-enums -fno-rtti -finline-limit=2000 -flto -march=native -O3 -pthread
#LD_FLAGS = -fshort-enums -fno-exceptions -fno-rtti -finline-limit=2000 -flto -march=native -Ofast
#LD_FLAGS = -fprofile-generate -fshort-enums -fno-exceptions -fno-rtti -finline-limit=2000 -flto -march=native -Ofast
#LD_FLAGS = -fprofile-use -fshort-enums -fno-exceptions -fno-rtti -finline-limit=2000 -flto -march=native -Ofast

OBJ_DIR = obj

SKAAK_CPP_FILES = $(wildcard src/chess/*.cpp)
SKAAK_OBJ_FILES = $(addprefix obj/,$(notdir $(SKAAK_CPP_FILES:.cpp=.o)))

SKAAK_BIN_NAME = skaak

PERFT_CPP_FILES = $(wildcard src/perft/*.cpp)
PERFT_HPP_FILES = $(wildcard src/perft/*.hpp)
PERFT_OBJ_FILES = $(addprefix obj/,$(notdir $(PERFT_CPP_FILES:.cpp=.o)))

PERFT_BIN_NAME = perft

all: $(OBJ_DIR) $(SKAAK_BIN_NAME) $(PERFT_BIN_NAME)

$(SKAAK_BIN_NAME): $(SKAAK_OBJ_FILES) $(OBJ_FILES)
	$(CXX) $(LD_FLAGS) -o $@ $^

$(PERFT_BIN_NAME): $(PERFT_OBJ_FILES) $(OBJ_FILES)
	$(CXX) $(LD_FLAGS) -o $@ $^

obj/%.o: src/%.cpp $(HPP_FILES) Makefile
	$(CXX) $(CC_FLAGS) -c -o $@ $<

obj/%.o: src/chess/%.cpp $(HPP_FILES)
	$(CXX) $(CC_FLAGS) -c -o $@ $<

obj/%.o: src/perft/%.cpp $(HPP_FILES) $(PERFT_HPP_FILES)
	$(CXX) $(CC_FLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(SKAAK_BIN_NAME) $(PERFT_BIN_NAME)



