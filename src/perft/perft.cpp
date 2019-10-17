#include <cstdio>
#include <cstdlib>

#include "board.hpp"
#include "perft.hpp"

using namespace Chess;
using namespace Perft;

int main(int argc, char* argv[]) {
  printf("sizeof(BoardT) = %lu\n", sizeof(BoardT));
  
  BoardT startingBoard = Board::startingPosition();
  Board::printBoard(startingBoard);
  printf("\n");

  // Board::printBb(EdgeSquaresBb);
  // printf("\n");

  PerftStatsT stats = perft<Black, StartingBoardTraitsT, StartingBoardTraitsT>(startingBoard, 7);

  printf("Perft(1) - nodes = %lu, captures = %lu, eps = %lu, castles = %lu, checks = %lu, discoveries = %lu, doublechecks = %lu, checkmates = %lu, invalids = %lu\n", stats.nodes, stats.captures, stats.eps, stats.castles, stats.checks, stats.discoverychecks, stats.doublechecks, stats.checkmates, stats.invalids);
  printf("           d1 slow (in check) = %lu, d1 nodes = %lu, d1 no checks = %lu, d1 no discoveries = %lu, d1 no illegals = %lu, d1 no nasty moves (wrong) = %lu\n", stats.d1checks, stats.d1nodes, stats.d1nochecks, stats.d1nodiscoveredchecks, stats.d1noillegalmoves, stats.d1nonastymoves);
}
