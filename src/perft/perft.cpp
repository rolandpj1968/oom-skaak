#include <cstdio>
#include <cstdlib>

#include "board.hpp"
#include "board-utils.hpp"
#include "fen.hpp"
#include "perft.hpp"

using namespace Chess;

// perft <depth>
int main(int argc, char* argv[]) {
  if(argc <= 1 || argc > 3) {
    fprintf(stderr, "usage: %s <depth> [FEN]\n", argv[0]);
    exit(1);
  }
  int depthToGo = atoi(argv[1]);

  printf("sizeof(BasicBoardT) is %lu\n", sizeof(BasicBoardT));
  printf("sizeof(FullBoardT) is %lu\n", sizeof(FullBoardT));
  printf("sizeof(FullColorStateImplT) is %lu\n", sizeof(FullColorStateImplT));
  //printf("sizeof(FullColorStateImpl2T) is %lu\n", sizeof(FullColorStateImpl2T));
  
  BasicBoardT board;
  ColorT colorToMove;
  if(argc == 2) {
    board = BoardUtils::startingPosition();
    colorToMove = White;
  } else {
    auto boardAndColor = Fen::parseFen(argv[2]);
    board = boardAndColor.first;
    colorToMove = boardAndColor.second;
  }

  BoardUtils::printBoard<BasicBoardT>(board);
  printf("\n%s\n\n", Fen::toFen<BasicBoardT>(board, colorToMove).c_str());

  Perft::PerftStatsT stats = colorToMove == White ?
    Perft::perft<BasicBoardT, White>(board, depthToGo) :
    Perft::perft<BasicBoardT, Black>(board, depthToGo);

  printf("perft(%d) - nodes = %lu, captures = %lu, eps = %lu, castles = %lu, promos = %lu, checks = %lu, discoveries = %lu, doublechecks = %lu, checkmates = %lu\n", depthToGo, stats.nodes, stats.captures, stats.eps, stats.castles, stats.promos, stats.checks, stats.discoverychecks, stats.doublechecks, stats.checkmates);
  printf("depth-0-with-eps = %lu, epdiscoveries = %lu, ephorizdiscoveries = %lu, epdiagfromdiscoveries = %lu, epdiagcapturediscoveries = %lu\n", stats.nposwitheps, stats.epdiscoveries, stats.ephorizdiscoveries, stats.epdiagfromdiscoveries, stats.epdiagcapturediscoveries);
}
