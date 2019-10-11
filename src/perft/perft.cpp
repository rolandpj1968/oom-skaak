#include <cstdio>

#include "board.hpp"
#include "perft.hpp"

using namespace Chess;
using namespace Perft;

int main(int argc, char* argv[]) {
  printf("sizeof(BoardT) = %lu\n", sizeof(BoardT));
  
  BoardT startingBoard = Board::startingPosition();

  printf("startingBoard.colorState[White].piecesPresent = 0x%04x, StartingPiecesPresentFlags = 0x%04x\n", startingBoard.pieces[White].piecesPresent, StartingPiecesPresentFlags);

  PerftStatsT stats = perft<Black>(startingBoard, 6);

  printf("Perft(1) - nodes = %lu, captures = %lu, eps = %lu, checks = %lu, checkmates = %lu, invalids = %lu, allpieces = %lu, allmypieces = %lu\n", stats.nodes, stats.captures, stats.eps, stats.checks, stats.checkmates, stats.invalids, stats.allpieces, stats.allmypieces);
}
