#include <cstdio>
#include <cstdlib>

#include "board.hpp"
#include "perft-basic.hpp"

using namespace Chess;
using namespace PerftBasic;

// perft <depth>
int main(int argc, char* argv[]) {
  if(argc <= 1) {
    fprintf(stderr, "usage: %s <depth>\n", argv[0]);
    exit(1);
  }
  int depthToGo = atoi(argv[1]);
  
  BoardT startingBoard = Board::startingPosition();

  PerftStatsT stats = perft<StartingBoardTraitsT>(startingBoard, depthToGo);

  printf("perft(%d) - nodes = %lu, captures = %lu, eps = %lu, castles = %lu\n", depthToGo, stats.nodes, stats.captures, stats.eps, stats.castles);
}
