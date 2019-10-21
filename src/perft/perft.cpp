#include <cstdio>
#include <cstdlib>

#include "board.hpp"
#include "perft.hpp"

using namespace Chess;
using namespace Perft;

int main(int argc, char* argv[]) {
  printf("sizeof(BoardT) = %lu\n", sizeof(BoardT));
  
  BoardT startingBoard = Board::startingPosition();

  PerftStatsT stats = perft<Black, StartingBoardTraitsT, StartingBoardTraitsT>(startingBoard, 6);

  printf("Perft(1) - nodes = %lu, captures = %lu, eps = %lu, castles = %lu, checks = %lu, discoveries = %lu, doublechecks = %lu, checkmates = %lu\n", stats.nodes, stats.captures, stats.eps, stats.castles, stats.checks, stats.discoverychecks, stats.doublechecks, stats.checkmates);
  printf("           invalids = %lu, invalidsnon0 = %lu, with-your-pins = %lu\n", stats.invalids, stats.invalidsnon0, stats.withyourpins);
}
