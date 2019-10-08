#include <cstdio>

#include "board.hpp"
#include "perft.hpp"

using namespace Chess;
using namespace Perft;

int main(int argc, char* argv[]) {
  BoardT startingBoard = Board::startingPosition();

  PerftStatsT stats = perft<Black>(startingBoard, 6);

  printf("Perft(1) - nodes = %lu, captures = %lu, eps = %lu, checks = %lu, checkmates = %lu, invalids = %lu\n", stats.nodes, stats.captures, stats.eps, stats.checks, stats.checkmates, stats.invalids);
}
