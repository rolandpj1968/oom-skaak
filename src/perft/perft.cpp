#include <cstdio>
#include <cstdlib>

#include "board.hpp"
#include "board-utils.hpp"
#include "fen.hpp"
#include "perft.hpp"

using namespace Chess;

void Perft::dumpStats(const Perft::PerftStatsT& stats) {
  printf("nodes = %lu, captures = %lu, eps = %lu, castles = %lu, promos = %lu, checks = %lu, discoveries = %lu, doublechecks = %lu, checkmates = %lu\n", stats.nodes, stats.captures, stats.eps, stats.castles, stats.promos, stats.checks, stats.discoverychecks, stats.doublechecks, stats.checkmates);
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
  
  fprintf(stderr, "usage: %s <depth> [FEN] [--split] [--max-tt-depth <depth>] [--tt-size <size>] [--tt-partitions <parts>] [--make-moves] [--threads <N>]\n\n", argv[0]);
  fprintf(stderr, "  Default position is the starting position; also use \"-\" for starting position, e.g. %s 6 \"-\" --max-tt-depth 4\n", argv[0]);
  fprintf(stderr, "  --split provides top-level subtree statistics per top-level move - this is useful for debugging\n");
  fprintf(stderr, "  --max-tt-depth <depth> enables tableauing of results for transpositions up to <depth>\n");
  fprintf(stderr, "      Transition tables (TTs) are only used from level 3 and deeper since no shallower transpositions are possible\n");
  fprintf(stderr, "  --tt-size <size> defines the maximum TT size for each level for each partition (default 16384)\n");
  fprintf(stderr, "      TT entries at each level are discarded according to LRU\n");
  fprintf(stderr, "  --tt-partitions <parts> defines the number of partitions that the TT's are split into (default 16)\n");
  fprintf(stderr, "      Partioned TT's are required to scale multi-threading beyond 16 threads. MUST be a power of 2\n");
  fprintf(stderr, "  --make-moves does all move do/undo up til leaf nodes which is slower that counting one level above\n");
  fprintf(stderr, "  --threads <N> runs N threads which distribute perft calculations from depth 2 and deeper\n");
  fprintf(stderr, "\n");
  
  exit(1);
}


template <typename BoardT, ColorT Color>
static std::pair<Perft::PerftStatsT, std::vector<std::pair<u64, u64>>> runPerft(const BoardT& board, const int depthToGo, const bool doSplit, const int maxTtDepth, const int ttSize, const int nTtParts, const bool makeMoves, const int nThreads) {
  Perft::PerftStatsT stats;
  std::vector<std::pair<u64, u64>> ttStats;

  // When --threads argument is not specified then we run inline
  if(nThreads == 0) {
    // Single-threaded
    if(maxTtDepth != 0) {
      auto allStats = Perft::ttPerft<BoardT, Color>(board, depthToGo, doSplit, makeMoves, maxTtDepth, ttSize, nTtParts);
      stats = allStats.first;
      ttStats = allStats.second;
    } else if(doSplit) {
      stats = Perft::splitPerft<BoardT, Color>(board, depthToGo, makeMoves);
    } else {
      stats = Perft::perft<BoardT, Color>(board, depthToGo, makeMoves);
    }
  } else {
    // Multi-threaded  
    auto allStats = Perft::paraPerft<BoardT, Color>(board, doSplit, makeMoves, maxTtDepth, depthToGo, ttSize, nTtParts, nThreads);
    stats = allStats.first;
    ttStats = allStats.second;
  }

  return std::make_pair(stats, ttStats);
}

int main(int argc, char* argv[]) {
  // printf("sizeof(NonPromosColorStateImplT) is %lu - NPieces is %d\n", sizeof(NonPromosColorStateImplT), NPieces);
  // printf("sizeof(BasicBoardT) is %lu\n", sizeof(BasicBoardT));
  // printf("sizeof(FullBoardT) is %lu\n", sizeof(FullBoardT));
  // printf("sizeof(FullColorStateImplT) is %lu\n", sizeof(FullColorStateImplT));
  // printf("\n");
  //printf("sizeof(FullColorStateImpl2T) is %lu\n", sizeof(FullColorStateImpl2T));
  
  if(argc <= 1) {
    usage_and_die(argc, argv, "Insufficient cmd-line arguments");
  }
  
  int depthToGo = atoi(argv[1]);
  BasicBoardT board;
  ColorT colorToMove;
  bool doSplit = false;
  int maxTtDepth = 0;
  int ttSize = 16384;
  int nTtParts = 16;
  bool makeMoves = false;
  int nThreads = 0;

  if(depthToGo < 0) {
    usage_and_die(argc, argv, "<depth> must be >= 0");
  }

  if(argc == 3 && std::string(argv[2]) == "RPJ") {
    do_special_and_die(depthToGo);
  }

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
    } else if(arg == "--tt-partitions") {
      i++;
      if(argc <= i) {
	usage_and_die(argc, argv, "--tt-partitions missing <parts> argument");
      }
      nTtParts = atol(argv[i]);
      if(nTtParts < 1 || (nTtParts & (nTtParts-1)) != 0) {
	usage_and_die(argc, argv, "Invalid TT partitions - must be a positive power of two.");
      }
    } else if(arg == "--make-moves") {
      makeMoves = true;
    } else if(arg == "--threads") {
      i++;
      if(argc <= i) {
	usage_and_die(argc, argv, "--threads missing <N> argument");
      }
      nThreads = atol(argv[i]);
      if(nThreads < 1 || nThreads > 8192) {
	usage_and_die(argc, argv, "Invalid #threads <N> - --threads 1 through --threads 8192 are valid");
      }
      if(nThreads > 1 && depthToGo <= 2) {
	usage_and_die(argc, argv, "Multi-threading is only valid for depth > 2");
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
  if(maxTtDepth != 0) {
    printf("  using TTs of %d partitions with %d entries at depths 3-%d\n", nTtParts, ttSize, maxTtDepth);
    doNewline = true;
  }
  if(nThreads > 0) {
    printf("  using %d worker threads\n", nThreads);
    doNewline = true;
  }
  if(doNewline) {
    printf("\n");
  }

  auto allStats = colorToMove == White ?
    runPerft<BasicBoardT, White>(board, depthToGo, doSplit, maxTtDepth, ttSize, nTtParts, makeMoves, nThreads) :
    runPerft<BasicBoardT, Black>(board, depthToGo, doSplit, maxTtDepth, ttSize, nTtParts, makeMoves, nThreads);

  if(doSplit) {
    printf("\n");
  }

  const auto& ttStats = allStats.second;
  if(maxTtDepth != 0) {
    using Perft::MinTtDepth;
    for(int i = MinTtDepth; i <= maxTtDepth; i++) {
      u64 nodes = ttStats[i - MinTtDepth].first;
      u64 hits = ttStats[i - MinTtDepth].second;
      printf("Depth %d: %lu nodes, %lu TT hits - %.2f%% hit rate\n", i, nodes, hits, ((double)hits/(double)nodes)*100.0);
    }
    printf("\n");
  }

  const auto& stats = allStats.first;
  printf("perft(%d) stats:\n\n", depthToGo);
  dumpStats(stats);
}
