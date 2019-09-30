chess: chess.cpp board.cpp move-gen.cpp chess.hpp types.hpp move-gen.hpp board.hpp
	g++ -fshort-enums -march=native -Ofast -o chess chess.cpp board.cpp move-gen.cpp



