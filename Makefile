chess: chess.cpp move-gen.cpp chess.hpp types.hpp move-gen.hpp board.hpp
	g++ -fshort-enums -O3 -o chess chess.cpp move-gen.cpp

