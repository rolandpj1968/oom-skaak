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
    
    Perft::PerftStatsT stats = Perft::perft<FullBoardT, White>(board, 1, true);
    
    printf("perft(%d) - nodes = %lu, captures = %lu, eps = %lu, castles = %lu, promos = %lu, checks = %lu, discoveries = %lu, doublechecks = %lu, checkmates = %lu\n", depthToGo, stats.nodes, stats.captures, stats.eps, stats.castles, stats.promos, stats.checks, stats.discoverychecks, stats.doublechecks, stats.checkmates);
  
    exit(1);
}

static void usage_and_die(int argc, char* argv[], const char* msg = 0) {
  if(msg) {
    fprintf(stderr, "%s\n\n", msg);
  }
  
  fprintf(stderr, "usage: %s <depth> [FEN] [--split] [--max-tt-depth <depth>] [--tt-size <size>] --make-moves\n\n", argv[0]);
  fprintf(stderr, "  Default position is the starting position; also use \"-\" for starting position, e.g. %s 6 \"-\" --max-tt-depth 4\n", argv[0]);
  fprintf(stderr, "  --split provides top-level subtree statistics per top-level move - this is useful for debugging\n");
  fprintf(stderr, "  --max-tt-depth <depth> enables tableauing of results for transpositions up to <depth>\n");
  fprintf(stderr, "      Transition tables (TTs) are only used from level 3 and deeper since no shallower transpositions are possible\n");
  fprintf(stderr, "  --tt-size <size> defines the maximum TT size for each level (default 262144)\n");
  fprintf(stderr, "      TT entries at each level are discarded according to LRU\n");
  fprintf(stderr, "  --make-moves does all move do/undo up til leaf nodes which is slower that counting one level above\n");
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
  int maxTtDepth = 0;
  int ttSize = 262144;
  bool makeMoves = false;
  
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
    } else if(arg == "--max-tt-depth") {
      i++;
      if(argc <= i) {
	usage_and_die(argc, argv, "--max-tt-depth missing <max-tt-depth> argument");
      }
      maxTtDepth = atoi(argv[i]);
      if(maxTtDepth < 3 || maxTtDepth > depthToGo || depthToGo < 3) {
	usage_and_die(argc, argv, "Invalid <max-tt-depth>");
      }
    } else if(arg == "--tt-size") {
      i++;
      if(argc <= i) {
	usage_and_die(argc, argv, "--tt-size missing <size> argument");
      }
      ttSize = atol(argv[i]);
      if(ttSize < 1) {
	usage_and_die(argc, argv, "Invalid TT size");
      }
    } else if(arg == "--make-moves") {
      makeMoves = true;
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
  if(maxTtDepth != 0) {
    printf("  using TTs with %d entries at depths 3-%d\n", ttSize, maxTtDepth);
    doNewline = true;
  }
  if(doNewline) {
    printf("\n");
  }

  Perft::PerftStatsT stats;

  if(maxTtDepth != 0) {
    auto allStats = colorToMove == White ?
      Perft::ttPerft<BasicBoardT, White>(board, doSplit, makeMoves, maxTtDepth, depthToGo, ttSize) :
      Perft::ttPerft<BasicBoardT, Black>(board, doSplit, makeMoves, maxTtDepth, depthToGo, ttSize);
    stats = allStats.first;
    if(doSplit) {
      printf("\n");
    }
    const auto& ttStats = allStats.second;
    using Perft::MinTtDepth;
    for(int i = MinTtDepth; i <= maxTtDepth; i++) {
      u64 nodes = ttStats[i - MinTtDepth].first;
      u64 hits = ttStats[i - MinTtDepth].second;
      printf("Depth %d: %lu nodes, %lu TT hits - %.2f%% hit rate\n", i, nodes, hits, ((double)hits/(double)nodes)*100.0);
    }
    printf("\n");
  } else if(doSplit) {
    stats = colorToMove == White ?
      Perft::splitPerft<BasicBoardT, White>(board, makeMoves, depthToGo) :
      Perft::splitPerft<BasicBoardT, Black>(board, makeMoves, depthToGo);
   printf("\n");
  } else {
   stats = colorToMove == White ?
     Perft::perft<BasicBoardT, White>(board, makeMoves, depthToGo) :
     Perft::perft<BasicBoardT, Black>(board, makeMoves, depthToGo);
  }

  printf("perft(%d) stats:\n\n", depthToGo);
  dumpStats(stats);
}
