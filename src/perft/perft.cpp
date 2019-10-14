#include <cstdio>

#include "board.hpp"
#include "perft.hpp"

using namespace Chess;
using namespace Perft;

int main(int argc, char* argv[]) {
  printf("sizeof(BoardT) = %lu\n", sizeof(BoardT));
  
  BoardT startingBoard = Board::startingPosition();

  PerftStatsT stats = perft<White, StartingBoardTraitsT, StartingBoardTraitsT>(startingBoard, 7);

  printf("Perft(1) - nodes = %lu, captures = %lu, eps = %lu, checks = %lu, checkmates = %lu, invalids = %lu\n", stats.nodes, stats.captures, stats.eps, stats.checks, stats.checkmates, stats.invalids);
}
