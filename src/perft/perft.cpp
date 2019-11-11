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

  PerftStatsT stats = perft<StartingBoardTraitsT>(startingBoard, depthToGo);

  printf("perft(%d) - nodes = %lu, captures = %lu, eps = %lu, castles = %lu, checks = %lu, discoveries = %lu, doublechecks = %lu, checkmates = %lu\n", depthToGo, stats.nodes, stats.captures, stats.eps, stats.castles, stats.checks, stats.discoverychecks, stats.doublechecks, stats.checkmates);
  printf("           invalids = %lu, invalidsnon0 = %lu, non0-in-pin-path = %lu, diag-pins = %lu, orthog-pins = %lu\n", stats.invalids, stats.invalidsnon0, stats.non0inpinpath, stats.non0withdiagpins, stats.non0withorthogpins);
  printf("           direct-checks = %lu, l0-non-discoveries = %lu, l0-checker-takable = %lu, l0-check-king-can-move = %lu\n", stats.directchecks, stats.l0nondiscoveries, stats.l0checkertakable, stats.l0checkkingcanmove);
}
