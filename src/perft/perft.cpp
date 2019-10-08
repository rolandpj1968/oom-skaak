#include <cstdio>

#include "board.hpp"
#include "perft.hpp"

using namespace Chess;
using namespace Perft;

int main(int argc, char* argv[]) {
  BoardT startingBoard = Board::startingPosition();

  PerftStatsT stats = perft<Black>(startingBoard, 3);

  printf("Perft(1) - nodes = %lu, captures = %lu, checks = %lu\n", stats.nodes, stats.captures, stats.checks);
}
