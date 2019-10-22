#include <cstdio>
#include <cstdlib>

#include "board.hpp"
#include "perft.hpp"

using namespace Chess;
using namespace Perft;

// perft <depth>
int main(int argc, char* argv[]) {
  if(argc <= 1) {
    fprintf(stderr, "usage: %s <depth>\n", argv[0]);
    exit(1);
  }
  int depthToGo = atoi(argv[1]);
  
  BoardT startingBoard = Board::startingPosition();

  PerftStatsT stats = perft<Black, StartingBoardTraitsT, StartingBoardTraitsT>(startingBoard, depthToGo);

  printf("perft(%d) - nodes = %lu, captures = %lu, eps = %lu, castles = %lu, checks = %lu, discoveries = %lu, doublechecks = %lu, checkmates = %lu\n", depthToGo, stats.nodes, stats.captures, stats.eps, stats.castles, stats.checks, stats.discoverychecks, stats.doublechecks, stats.checkmates);
  printf("           invalids = %lu, invalidsnon0 = %lu, with-your-pins = %lu, with-your-pins-2 = %lu\n", stats.invalids, stats.invalidsnon0, stats.withyourpins, stats.withyourpins2);
}
