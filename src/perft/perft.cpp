#include <cstdio>
#include <cstdlib>

#include "board.hpp"
#include "fen.hpp"
#include "perft.hpp"

using namespace Chess;
using namespace Perft;

// perft <depth>
int main(int argc, char* argv[]) {
  if(argc <= 1 || argc > 3) {
    fprintf(stderr, "usage: %s <depth> [FEN]\n", argv[0]);
    exit(1);
  }
  int depthToGo = atoi(argv[1]);

  printf("sizeof(BoardT) is %lu\n", sizeof(BoardT));
  
  BoardT board;
  ColorT colorToMove;
  if(argc == 2) {
    board = Board::startingPosition();
    colorToMove = White;
  } else {
    // if(std::string(argv[2]) == "RPJ") {
    //   printf("\nRPJ special debug setup...\n\n");
    //   board = emptyBoard();
    //   colorToMove = White;
    //   Board::addPromoPiece(board, White, 3, PromoRook, A8);
    //   Board::addPromoPiece(board, Black, 7, PromoRook, A1);
    // } else {
    auto boardAndColor = Fen::parseFen(argv[2]);
    board = boardAndColor.first;
    colorToMove = boardAndColor.second;
    // }
  }

  printBoard(board);
  printf("\n%s\n\n", Fen::toFen(board, colorToMove).c_str());

  PerftStatsT stats = colorToMove == White ?
    perft<StartingBoardTraitsT>(board, depthToGo) :
    perft<StartingBoardTraitsT::ReverseT>(board, depthToGo);

  printf("perft(%d) - nodes = %lu, captures = %lu, eps = %lu, castles = %lu, promos = %lu, checks = %lu, discoveries = %lu, doublechecks = %lu, checkmates = %lu\n", depthToGo, stats.nodes, stats.captures, stats.eps, stats.castles, stats.promos, stats.checks, stats.discoverychecks, stats.doublechecks, stats.checkmates);
  printf("depth-0-with-eps = %lu, epdiscoveries = %lu, ephorizdiscoveries = %lu, epdiagfromdiscoveries = %lu, epdiagcapturediscoveries = %lu\n", stats.nposwitheps, stats.epdiscoveries, stats.ephorizdiscoveries, stats.epdiagfromdiscoveries, stats.epdiagcapturediscoveries);
}
