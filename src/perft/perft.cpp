#include <cstdio>
#include <cstdlib>

#include "board.hpp"
#include "board-utils.hpp"
#include "fen.hpp"
#include "perft.hpp"

using namespace Chess;

void Perft::dumpStats(const Perft::PerftStatsT& stats) {
  printf("nodes = %lu, captures = %lu, eps = %lu, castles = %lu, promos = %lu, checks = %lu, discoveries = %lu, doublechecks = %lu, checkmates = %lu\n", stats.nodes, stats.captures, stats.eps, stats.castles, stats.promos, stats.checks, stats.discoverychecks, stats.doublechecks, stats.checkmates);
  //printf("depth-0-with-eps = %lu, epdiscoveries = %lu, ephorizdiscoveries = %lu, epdiagfromdiscoveries = %lu, epdiagcapturediscoveries = %lu\n", stats.nposwitheps, stats.epdiscoveries, stats.ephorizdiscoveries, stats.epdiagfromdiscoveries, stats.epdiagcapturediscoveries);
}



static void do_special_and_die(int depthToGo) {
    printf("Hallo RPJ\n");
    //auto basicBoard = Fen::parseFen("r3k2r/Pppp1ppp/1b3nbN/nPP5/BB2P3/q4N2/Pp1P2PP/R2Q1RK1 b kq - 0 1").first;
    auto basicBoard = Fen::parseFen("r3k3/Pppp1ppp/1b3nbN/nPP5/BB2P3/q4N2/Pp1P2PP/R2Q1RK1 b q - 0 1").first;
    auto fullBoard = Board::copyBoard<FullBoardT, BasicBoardT>(basicBoard);
    // Promote black pawn on B2 to queen
    auto board = Board::pushPawnToPromo<FullBoardT, Black>(fullBoard, B2, B1, PromoRook);

    BoardUtils::printBoard<FullBoardT>(board);
    printf("\n%s\n\n", Fen::toFen<FullBoardT>(board, White).c_str());
    
    Perft::PerftStatsT stats = Perft::perft<FullBoardT, White>(board, 1);
    
    printf("perft(%d) - nodes = %lu, captures = %lu, eps = %lu, castles = %lu, promos = %lu, checks = %lu, discoveries = %lu, doublechecks = %lu, checkmates = %lu\n", depthToGo, stats.nodes, stats.captures, stats.eps, stats.castles, stats.promos, stats.checks, stats.discoverychecks, stats.doublechecks, stats.checkmates);
  
    exit(1);
}

static void usage_and_die(int argc, char* argv[], const char* msg = 0) {
  if(msg) {
    fprintf(stderr, "%s\n\n", msg);
  }
  
  fprintf(stderr, "usage: %s <depth> [FEN] [--split] [--hash-depth <max-depth>] [--hash-size <size>]\n\n", argv[0]);
  fprintf(stderr, "  Default position is the starting position; also use \"-\" for starting position, e.g. %s 6 \"-\" --hash 4\n", argv[0]);
  fprintf(stderr, "  --split provides top-level subtree statistics per top-level move - this is useful for debugging\n");
  fprintf(stderr, "  --hash <max-depth> enables tableauing of results for repeated positions up to max-depth\n");
  fprintf(stderr, "      hash tables are only used from level 3 and deeper since no repeat positions are possible nearer the root\n");
  fprintf(stderr, "  --hash-size <size> defines the maximum hash-table size for each level\n");
  fprintf(stderr, "      hash table entries are discarded according to LRU\n");
  fprintf(stderr, "\n");
  
  exit(1);
}

int main(int argc, char* argv[]) {
  printf("sizeof(NonPromosColorStateImplT) is %lu - NPieces is %d\n", sizeof(NonPromosColorStateImplT), NPieces);
  printf("sizeof(BasicBoardT) is %lu\n", sizeof(BasicBoardT));
  printf("sizeof(FullBoardT) is %lu\n", sizeof(FullBoardT));
  printf("sizeof(FullColorStateImplT) is %lu\n", sizeof(FullColorStateImplT));
  printf("\n");
  //printf("sizeof(FullColorStateImpl2T) is %lu\n", sizeof(FullColorStateImpl2T));
  
  if(argc <= 1) {
    usage_and_die(argc, argv, "Insufficient cmd-line arguments");
  }
  
  int depthToGo = atoi(argv[1]);
  if(depthToGo < 0) {
    usage_and_die(argc, argv, "<depth> must be >= 0");
  }

  if(argc == 3 && std::string(argv[2]) == "RPJ") {
    do_special_and_die(depthToGo);
  }

  BasicBoardT board;
  ColorT colorToMove;
  bool doSplit = false;
  int maxHashDepth = 0;
  int hashSize = 4096;
  
  if(argc < 3 || std::string(argv[2]) == "-") {
    board = BoardUtils::startingPosition();
    colorToMove = White;
  } else {
    auto boardAndColor = Fen::parseFen(argv[2]);
    board = boardAndColor.first;
    colorToMove = boardAndColor.second;
  }

  // Parse flags
  for(int i = 3; i < argc; i++ ) {
    std::string arg = argv[i];

    if(arg == "--split") {
      doSplit = true;
    } else if(arg == "--hash-depth") {
      i++;
      if(argc <= i) {
	usage_and_die(argc, argv, "--hash-depth missing <max-hash-depth> argument");
      }
      maxHashDepth = atoi(argv[i]);
      if(maxHashDepth < 3 || maxHashDepth > depthToGo || depthToGo < 3) {
	usage_and_die(argc, argv, "Invalid <max-hash-depth>");
      }
    } else if(arg == "--hash-size") {
      i++;
      if(argc <= i) {
	usage_and_die(argc, argv, "--hash-size missing <size> argument");
      }
      hashSize = atol(argv[i]);
      if(hashSize < 1) {
	usage_and_die(argc, argv, "Invalid hash size");
      }
    } else {
	usage_and_die(argc, argv, "Unrecognised argument");
    }
  }

  BoardUtils::printBoard<BasicBoardT>(board);
  printf("\n%s\n\n", Fen::toFen<BasicBoardT>(board, colorToMove).c_str());
  bool doNewline = false;
  if(doSplit) {
    printf("  providing split results per move at depth 1\n");
    doNewline = true;
  }
  if(maxHashDepth != 0) {
    printf("  using hash tables with %d entries at depths 3 - %d\n", hashSize, maxHashDepth);
    doNewline = true;
  }
  if(doNewline) {
    printf("\n");
  }

  Perft::PerftStatsT stats;

  if(doSplit) {
   stats = colorToMove == White ?
      Perft::splitPerft<BasicBoardT, White>(board, depthToGo) :
      Perft::splitPerft<BasicBoardT, Black>(board, depthToGo);
   printf("\n");
  } else {
   stats = colorToMove == White ?
      Perft::perft<BasicBoardT, White>(board, depthToGo) :
      Perft::perft<BasicBoardT, Black>(board, depthToGo);
  }

  printf("perft(%d) stats:\n\n", depthToGo);
  dumpStats(stats);
}
