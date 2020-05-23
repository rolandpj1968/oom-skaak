#ifndef PERFT_HPP
#define PERFT_HPP

#include "types.hpp"
#include "board.hpp"
#include "board-utils.hpp"
#include "bounded-hash-map.hpp"
#include "fen.hpp"
#include "move-gen.hpp"
#include "make-move.hpp"
#include "bits.hpp"

#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

namespace Chess {
  
  namespace Perft {

    struct PerftStatsT {
      u64 nodes;
      u64 captures;
      u64 eps;
      u64 castles;
      u64 promos;
      u64 checks;
      // Note that 'discoverychecks' here includes checks delivered by castling, which is contraversial.
      // According to stats from: https://www.chessprogramming.org/Perft_Results
      // Would probably be better to separate this out into real discoveries and checks-from-castling
      u64 discoverychecks;
      u64 doublechecks;
      u64 checkmates;

      u64 pawndirectchecks;
      u64 kingmovefromchecks;
      u64 threekingmovesfromchecks;
    };

    inline void addAll(PerftStatsT& to, const PerftStatsT& from) {
      to.nodes += from.nodes;
      to.captures += from.captures;
      to.eps += from.eps;
      to.castles += from.castles;
      to.promos += from.promos;
      to.checks += from.checks;
      to.discoverychecks += from.discoverychecks;
      to.doublechecks += from.doublechecks;
      to.checkmates += from.checkmates;
    }

    extern void dumpStats(const Perft::PerftStatsT& stats);
    
    // Curiously adding just one more member here - depth - slows down perf substantially, particularly if they are int size!
    struct PerftStateT {
      PerftStatsT& stats;
      const bool makeMoves;
      const u8 depth;
      const u8 depthToGo;

      PerftStateT(PerftStatsT& stats, const bool makeMoves, const u8 depth, const u8 depthToGo):
	stats(stats), makeMoves(makeMoves), depth(depth), depthToGo(depthToGo) {}
    };

    template <typename BoardT, ColorT Color>
    inline void perftImpl(const PerftStateT state, const BoardT& board, const MoveInfoT moveInfo);
  
    template <typename BoardT, ColorT Color>
    struct PerftPosHandlerT {
      typedef PerftPosHandlerT<BoardT, OtherColorT<Color>::value> ReverseT;
      typedef PerftPosHandlerT<typename BoardType<BoardT>::WithPromosT, Color> WithPromosT;
      typedef PerftPosHandlerT<typename BoardType<BoardT>::WithoutPromosT, Color> WithoutPromosT;
      
      inline static void handlePos(const PerftStateT state, const BoardT& board, MoveInfoT moveInfo) {
	perftImpl<BoardT, Color>(state, board, moveInfo);
      }
    };

    template <typename BoardT, ColorT Color>
    struct PerftCountHandlerT {
      typedef PerftCountHandlerT<BoardT, OtherColorT<Color>::value> ReverseT;
      typedef PerftCountHandlerT<typename BoardType<BoardT>::WithPromosT, Color> WithPromosT;
      typedef PerftCountHandlerT<typename BoardType<BoardT>::WithoutPromosT, Color> WithoutPromosT;
      
      inline static void handleCount(PerftStatsT& stats, const u64 nodes, u64 captures, u64 eps, u64 castles, u64 promos, u64 checks, u64 discoverychecks, u64 doublechecks) {
	stats.nodes += nodes;
	stats.captures += captures;
	stats.eps += eps;
	stats.castles += castles;
	stats.promos += promos;
	stats.checks += checks;
	stats.discoverychecks += discoverychecks;
	stats.doublechecks += doublechecks;
      }
    };

    inline bool hasLegalPromoPieceMoves(const BasicColorStateImplT& colorState, const typename MoveGen::LegalMovesImplType<BasicBoardT>::LegalMovesT& legalMoves) {
      return false; // no promo pieces
    }
    
    inline bool hasLegalPromoPieceMoves(const FullColorStateImplT& colorState, const typename MoveGen::LegalMovesImplType<FullBoardT>::LegalMovesT& legalMoves) {
      BitBoardT activePromos = (BitBoardT)colorState.promos.activePromos;
      while(activePromos) {
	const int promoIndex = Bits::popLsb(activePromos);
	if(legalMoves.promoPieceMoves[promoIndex] != BbNone) {
	  return true;
	}
      }
      return false;
    }
    
    template <typename BoardT, ColorT Color>
    inline bool hasLegalMoves(const BoardT& board) {
      typedef typename MoveGen::LegalMovesImplType<BoardT>::LegalMovesT LegalMovesT;
      
      // Generate (legal) moves
      const LegalMovesT legalMoves = MoveGen::genLegalMoves<BoardT, Color>(board);

      // Are there any?
      const BitBoardT legalNonPromoMovesBb =
	  
	legalMoves.pawnMoves.pushesOneBb | legalMoves.pawnMoves.pushesTwoBb |
	legalMoves.pawnMoves.capturesLeftBb | legalMoves.pawnMoves.capturesRightBb |
	legalMoves.pawnMoves.epCaptures.epLeftCaptureBb | legalMoves.pawnMoves.epCaptures.epRightCaptureBb |

	legalMoves.pieceMoves[Knight1] |
	legalMoves.pieceMoves[Knight2] |
	  
	legalMoves.pieceMoves[Bishop1] |
	legalMoves.pieceMoves[Bishop2] |
	  
	legalMoves.pieceMoves[Rook1] |
	legalMoves.pieceMoves[Rook2] |
	  
	legalMoves.pieceMoves[TheQueen] |

	legalMoves.pieceMoves[TheKing] |

	(BitBoardT)legalMoves.canCastleFlags;

      return legalNonPromoMovesBb != BbNone || hasLegalPromoPieceMoves(board.state[(size_t)Color], legalMoves);
    }

    template <typename BoardT, ColorT Color>
    inline int nLegalKingMoves(const BoardT& board) {
      typedef typename MoveGen::LegalMovesImplType<BoardT>::LegalMovesT LegalMovesT;
      
      // Generate (legal) moves
      const LegalMovesT legalMoves = MoveGen::genLegalMoves<BoardT, Color>(board);

      return Bits::count(legalMoves.pieceMoves[TheKing]);
    }
    
    template <typename BoardT, ColorT Color>
    inline void perft0Impl(PerftStatsT& stats, const BoardT& board, const MoveInfoT moveInfo) {
      stats.nodes++;

      if(moveInfo.moveType == CaptureMove) {
	stats.captures++;
      }

      if(moveInfo.moveType == EpCaptureMove) {
	stats.captures++;
	stats.eps++;
      }

      if(moveInfo.moveType == CastlingMove) {
	stats.castles++;
      }

      if(moveInfo.isPromo) {
	stats.promos++;
      }

      if(moveInfo.isDirectCheck) {
	stats.checks++;
	if(moveInfo.pieceType == Pawn) {
	  stats.pawndirectchecks++;
	}
      }
      if(moveInfo.isDiscoveredCheck) {
	// Double checks are counted independently of discoveries
	if(moveInfo.isDirectCheck) {
	  stats.doublechecks++;
	} else {
	  stats.checks++;
	  stats.discoverychecks++;
	}
      }

      static const bool DoCheckMateStats = true;
      if(DoCheckMateStats) {
	if(moveInfo.isDirectCheck || moveInfo.isDiscoveredCheck) {
	  // Only bother if it is check
	  // It's checkmate if there are no legal moves
	  if(!hasLegalMoves<BoardT, Color>(board)) {
	    stats.checkmates++;
	  } else {
	    const int nKingMoves = nLegalKingMoves<BoardT, Color>(board);
	    if(nKingMoves != 0) {
	      stats.kingmovefromchecks++;
	      if(nKingMoves >= 3) {
		stats.threekingmovesfromchecks++;
	      }
	    }
	  }
	}
      }
    }

    template <typename BoardT, ColorT Color>
    inline void perft1Impl(PerftStatsT& stats, const BoardT& board) {
      MakeMove::countAllLegalMoves<PerftStatsT&, PerftCountHandlerT<BoardT, Color>, BoardT, Color>(stats, board);
    }
    
    template <typename BoardT, ColorT Color>
    inline void perftImplFull(const PerftStateT state, const BoardT& board) {
      
      const PerftStateT newState(state.stats, state.makeMoves, state.depth+1, state.depthToGo-1);
      
      MakeMove::makeAllLegalMoves<const PerftStateT, PerftPosHandlerT<BoardT, Color>, BoardT, Color>(newState, board);
    }

    //const bool DoDepth1Count = true;

    template <typename BoardT, ColorT Color>
    inline void perftImpl(const PerftStateT state, const BoardT& board, const MoveInfoT moveInfo) {
      // If this is a leaf node, gather stats.
      if(state.depthToGo == 0) {
	perft0Impl<BoardT, Color>(state.stats, board, moveInfo);
      } else if(state.depthToGo == 1 && !state.makeMoves) {
	perft1Impl<BoardT, Color>(state.stats, board);
      } else {
	perftImplFull<BoardT, Color>(state, board);
      }
    }
      
    template <typename BoardT, ColorT Color>
    inline PerftStatsT perft(const BoardT& board, const int depthToGo, const bool makeMoves) {
      PerftStatsT stats = {};
      const int nChecks = BoardUtils::getNChecks<BoardT, Color>(board);
      MoveInfoT dummyMoveInfo(PushMove, NoPieceType, /*from*/InvalidSquare, /*to*/InvalidSquare, /*isDirectCheck*/(nChecks > 0), /*isDiscoveredCheck*/(nChecks > 1));
      const PerftStateT state(stats, makeMoves, 0, depthToGo);

      perftImpl<BoardT, Color>(state, board, dummyMoveInfo);

      return stats;
    }

    //
    // Split perft implementation
    //

    template <typename BoardT, ColorT Color>
    inline void splitPerftImpl(const PerftStateT state, const BoardT& board, const MoveInfoT moveInfo);
  
    template <typename BoardT, ColorT Color>
    struct SplitPerftPosHandlerT {
      typedef SplitPerftPosHandlerT<BoardT, OtherColorT<Color>::value> ReverseT;
      typedef SplitPerftPosHandlerT<typename BoardType<BoardT>::WithPromosT, Color> WithPromosT;
      typedef SplitPerftPosHandlerT<typename BoardType<BoardT>::WithoutPromosT, Color> WithoutPromosT;
      
      inline static void handlePos(const PerftStateT state, const BoardT& board, MoveInfoT moveInfo) {
	PerftStatsT splitStats = {};
	const PerftStateT splitState(splitStats, state.makeMoves, state.depth, state.depthToGo);
	
	splitPerftImpl<BoardT, Color>(splitState, board, moveInfo);

	if(state.depth == 1) {
	  printf("  move %s-%s: ", SquareStr[moveInfo.from], SquareStr[moveInfo.to]);
	  dumpStats(splitStats);
	}

	// Accumulate the stats
	addAll(state.stats, splitStats);
      }
    };

    template <typename BoardT, ColorT Color>
    inline void splitPerftImplFull(const PerftStateT state, const BoardT& board) {
      
      const PerftStateT newState(state.stats, state.makeMoves, state.depth+1, state.depthToGo-1);

      MakeMove::makeAllLegalMoves<const PerftStateT, SplitPerftPosHandlerT<BoardT, Color>, BoardT, Color>(newState, board);
    }

    template <typename BoardT, ColorT Color>
    inline void splitPerftImpl(const PerftStateT state, const BoardT& board, const MoveInfoT moveInfo) {
      // If this is a leaf node, gather stats.
      if(state.depthToGo == 0) {
	perft0Impl<BoardT, Color>(state.stats, board, moveInfo);
	} else if(state.depth < 2) {
	splitPerftImplFull<BoardT, Color>(state, board);
      } else {
	perftImpl<BoardT, Color>(state, board, moveInfo);
      }
    }
      
    template <typename BoardT, ColorT Color>
    inline PerftStatsT splitPerft(const BoardT& board, const int depthToGo, const bool makeMoves) {
      PerftStatsT stats = {};
      const int nChecks = BoardUtils::getNChecks<BoardT, Color>(board);
      MoveInfoT dummyMoveInfo(PushMove, NoPieceType, /*from*/InvalidSquare, /*to*/InvalidSquare, /*isDirectCheck*/(nChecks > 0), /*isDiscoveredCheck*/(nChecks > 1));
      const PerftStateT state(stats, makeMoves, 0, depthToGo);

      splitPerftImpl<BoardT, Color>(state, board, dummyMoveInfo);

      return stats;
    }

    //
    // Perft with transition tables
    //
    // We maintain a separate TT per depth
    //

    using BoundedHashMap::BoundedHashMap;

    struct TtPerftStateT {
      PerftStatsT& stats;
      std::vector<std::vector<BoundedHashMap<std::string, PerftStatsT>>>& tts; // indexed on tts[partition][depth-MinTtDepth]
      std::vector<std::pair<u64, u64>>& ttStats; // (total-nodes, ht-hits)
      const bool doSplit;
      const bool makeMoves;
      const u8 maxTtDepth;
      const u8 depth;
      const u8 depthToGo;

      TtPerftStateT(PerftStatsT& stats, std::vector<std::vector<BoundedHashMap<std::string, PerftStatsT>>>& tts, std::vector<std::pair<u64, u64>>& ttStats, const bool doSplit, const bool makeMoves, const u8 maxTtDepth, const u8 depth, const u8 depthToGo):
	stats(stats), tts(tts), ttStats(ttStats), doSplit(doSplit), makeMoves(makeMoves), maxTtDepth(maxTtDepth), depth(depth), depthToGo(depthToGo) {}
    };

    const int MinTtDepth = 3;
    
    template <typename BoardT, ColorT Color>
    inline void ttPerftImpl(const TtPerftStateT state, const BoardT& board, const MoveInfoT moveInfo);
  
    template <typename BoardT, ColorT Color>
    struct TtPerftPosHandlerT {
      typedef TtPerftPosHandlerT<BoardT, OtherColorT<Color>::value> ReverseT;
      typedef TtPerftPosHandlerT<typename BoardType<BoardT>::WithPromosT, Color> WithPromosT;
      typedef TtPerftPosHandlerT<typename BoardType<BoardT>::WithoutPromosT, Color> WithoutPromosT;
      
      inline static void handlePos(const TtPerftStateT state, const BoardT& board, MoveInfoT moveInfo) {
	PerftStatsT splitStats = {};

	bool foundIt = false;
	std::string fen;
	const size_t nParts = state.tts.size();
	const size_t partMask = nParts - 1;
	size_t part = 0;

	// Probe the TT
	const int ttIndex = state.depth - MinTtDepth;
	if(MinTtDepth <= state.depth && state.depth <= state.maxTtDepth) {
	  state.ttStats[ttIndex].first++;
	  // Omit the EP square in cases where EP capture is impossible - this gives us more transpositions
	  fen = Fen::toFenFast<BoardT>(board, Color, /*trimEp*/true);
	  part = std::hash<std::string>{}(fen) & partMask;
	  foundIt = state.tts[part][ttIndex].copy_if_present(fen, splitStats);
	  if(foundIt) {
	    state.ttStats[ttIndex].second++;
	  }
	}

	// If it's not in the TT then compute it
	if(!foundIt) {
	  const TtPerftStateT splitState(splitStats, state.tts, state.ttStats, state.doSplit, state. makeMoves, state.maxTtDepth, state.depth, state.depthToGo);
	
	  ttPerftImpl<BoardT, Color>(splitState, board, moveInfo);
	}
	
	if(state.doSplit && state.depth == 1) {
	  printf("  move %s-%s: ", SquareStr[moveInfo.from], SquareStr[moveInfo.to]);
	  dumpStats(splitStats);
	}

	// Accumulate the stats
	addAll(state.stats, splitStats);

	// If it's not in the TT then insert it
	if(MinTtDepth <= state.depth && state.depth <= state.maxTtDepth && !foundIt) {
	  state.tts[part][ttIndex].put(fen, splitStats);
	}
      }
    };

    template <typename BoardT, ColorT Color>
    inline void ttPerftImplFull(const TtPerftStateT state, const BoardT& board) {
      
      const TtPerftStateT newState(state.stats, state.tts, state.ttStats, state.doSplit, state.makeMoves, state.maxTtDepth, state.depth+1, state.depthToGo-1);

      MakeMove::makeAllLegalMoves<const TtPerftStateT, TtPerftPosHandlerT<BoardT, Color>, BoardT, Color>(newState, board);
    }

    template <typename BoardT, ColorT Color>
    inline void ttPerftImpl(const TtPerftStateT state, const BoardT& board, const MoveInfoT moveInfo) {
      // If this is a leaf node, gather stats.
      if(state.depthToGo == 0) {
	perft0Impl<BoardT, Color>(state.stats, board, moveInfo);
      } else if(state.depth <= state.maxTtDepth) {
	ttPerftImplFull<BoardT, Color>(state, board);
      } else {
	PerftStateT perftState(state.stats, state.makeMoves, state.depth, state.depthToGo);
	perftImpl<BoardT, Color>(perftState, board, moveInfo);
      }
    }
      
    template <typename BoardT, ColorT Color>
    inline PerftStatsT ttPerft(const BoardT& board, const MoveInfoT moveInfo, std::vector<std::vector<BoundedHashMap<std::string, PerftStatsT>>>& tts, std::vector<std::pair<u64, u64>>& ttStats, const bool doSplit, const bool makeMoves, const int maxTtDepth, const int depth, const int depthToGo) {
      PerftStatsT stats = {};
      const TtPerftStateT state(stats, tts, ttStats, doSplit, makeMoves, maxTtDepth, depth, depthToGo);

      ttPerftImpl<BoardT, Color>(state, board, moveInfo);
	
      return stats;
    }

    template <typename BoardT, ColorT Color>
    inline std::pair<PerftStatsT, std::vector<std::pair<u64, u64>>> ttPerft(const BoardT& board, const int depthToGo, const bool doSplit, const bool makeMoves, const int maxTtDepth, const int ttSize, const int nTtParts) {

      std::vector<std::vector<BoundedHashMap<std::string, PerftStatsT>>> tts(nTtParts);

      for(int partNo = 0; partNo < nTtParts; partNo++) {
	// map: fen->stats for each depth for each partition
	for(int i = MinTtDepth; i <= maxTtDepth; i++) {
	  tts[partNo].push_back(BoundedHashMap<std::string, PerftStatsT>(ttSize));
	}
      }
      std::vector<std::pair<u64, u64>> ttStats(maxTtDepth-MinTtDepth+1);
      
      const int nChecks = BoardUtils::getNChecks<BoardT, Color>(board);
      MoveInfoT dummyMoveInfo(PushMove, NoPieceType, /*from*/InvalidSquare, /*to*/InvalidSquare, /*isDirectCheck*/(nChecks > 0), /*isDiscoveredCheck*/(nChecks > 1));

      PerftStatsT stats = ttPerft<BoardT, Color>(board, dummyMoveInfo, tts, ttStats, doSplit, makeMoves, maxTtDepth, /*depth*/0, depthToGo);

      return std::make_pair(stats, ttStats);
    }
    
    //
    // Parallel perft
    //
    struct Depth2CollectorStateT {
      std::list<std::pair<std::string, MoveInfoT>>& depth2FensAndMoves;
      const int depth;

      Depth2CollectorStateT(std::list<std::pair<std::string, MoveInfoT>>& depth2FensAndMoves, const int depth) :
	depth2FensAndMoves(depth2FensAndMoves), depth(depth) {}
    };
    
    template <typename BoardT, ColorT Color>
    struct Depth2CollectorPosHandlerT {
      typedef Depth2CollectorPosHandlerT<BoardT, OtherColorT<Color>::value> ReverseT;
      typedef Depth2CollectorPosHandlerT<typename BoardType<BoardT>::WithPromosT, Color> WithPromosT;
      typedef Depth2CollectorPosHandlerT<typename BoardType<BoardT>::WithoutPromosT, Color> WithoutPromosT;
      
      inline static void handlePos(const Depth2CollectorStateT& state, const BoardT& board, MoveInfoT moveInfo) {
	// This is a child node of the given depth and we're collecting depth-2 FEN's, hence when state.depth == 1
	if(state.depth == 1) {
	  state.depth2FensAndMoves.push_back(std::make_pair(Fen::toFenFast(board, Color, /*trimEp*/true), moveInfo));
	} else {
	  Depth2CollectorStateT newState(state.depth2FensAndMoves, state.depth+1);
	  MakeMove::makeAllLegalMoves<const Depth2CollectorStateT&, Depth2CollectorPosHandlerT<BoardT, Color>, BoardT, Color>(newState, board);
	}
      }
    };

    struct Depth2AccumulatorStateT {
      PerftStatsT& stats;
      const std::map<std::string, PerftStatsT>& depth2PosStats;
      const bool doSplit;
      const int depth;

      Depth2AccumulatorStateT(PerftStatsT& stats, const std::map<std::string, PerftStatsT>& depth2PosStats, const bool doSplit, const int depth) :
	stats(stats), depth2PosStats(depth2PosStats), doSplit(doSplit), depth(depth) {}
    };
    
    template <typename BoardT, ColorT Color>
    struct Depth2AccumulatorPosHandlerT {
      typedef Depth2AccumulatorPosHandlerT<BoardT, OtherColorT<Color>::value> ReverseT;
      typedef Depth2AccumulatorPosHandlerT<typename BoardType<BoardT>::WithPromosT, Color> WithPromosT;
      typedef Depth2AccumulatorPosHandlerT<typename BoardType<BoardT>::WithoutPromosT, Color> WithoutPromosT;
      
      inline static void handlePos(const Depth2AccumulatorStateT& state, const BoardT& board, MoveInfoT moveInfo) {
	// This is a child node of the given depth and we're accumulating depth-2 stats, hence when state.depth == 1
	if(state.depth == 1) {
	  addAll(state.stats, state.depth2PosStats.at(Fen::toFenFast(board, Color, /*trimEp*/true)));
	} else {
	  PerftStatsT splitStats = {};
	  Depth2AccumulatorStateT newState(splitStats, state.depth2PosStats, state.doSplit, state.depth+1);
	  MakeMove::makeAllLegalMoves<const Depth2AccumulatorStateT&, Depth2AccumulatorPosHandlerT<BoardT, Color>, BoardT, Color>(newState, board);
	  if(state.doSplit) {
	    printf("  move %s-%s: ", SquareStr[moveInfo.from], SquareStr[moveInfo.to]);
	    dumpStats(splitStats);
	  }
	  addAll(state.stats, splitStats);
	}
      }
    };

    inline void paraPerftWorkerFn(int n, std::mutex& m, std::list<std::pair<std::string, MoveInfoT>>& depth2FensAndMoves, std::map<std::string, PerftStatsT>& depth2PosStats, std::vector<std::vector<BoundedHashMap<std::string, PerftStatsT>>>& tts, std::vector<std::pair<u64, u64>>& ttStats, const bool makeMoves, const int maxTtDepth, const int depthToGo) {
      int nFens = 0;
      // Finish when the list is empty
      while(true) {
	// Get a depth-2 FEN to calculate
	std::string fen;
	MoveInfoT moveInfo(PushMove, NoPieceType, /*from*/InvalidSquare, /*to*/InvalidSquare, /*isDirectCheck*/false, /*isDiscoveredCheck*/false); // not used
	{
	  std::unique_lock<std::mutex> lock(m);
	  if(depth2FensAndMoves.empty()) {
	    return; // no more work
	  }
	  auto& fenAndMove = depth2FensAndMoves.front();
	  fen = fenAndMove.first;
	  moveInfo = fenAndMove.second;
	  depth2FensAndMoves.pop_front();
	  nFens++;
	}

	// Compute perft stats
	auto boardAndColor = Fen::parseFen(fen);
	const BasicBoardT& board = boardAndColor.first;
	const ColorT colorToMove = boardAndColor.second;
	
	PerftStatsT stats = colorToMove == White ?
	  ttPerft<BasicBoardT, White>(board, moveInfo, tts, ttStats, /*doSplit*/false, makeMoves, maxTtDepth, /*depth*/2, depthToGo-2) :
	  ttPerft<BasicBoardT, Black>(board, moveInfo, tts, ttStats, /*doSplit*/false, makeMoves, maxTtDepth, /*depth*/2, depthToGo-2);

	// Record the perft results
	{
	  std::unique_lock<std::mutex> lock(m);
	  depth2PosStats[fen] = stats;
	}
      }
    }

    template <typename BoardT, ColorT Color>
    inline std::pair<PerftStatsT, std::vector<std::pair<u64, u64>>> paraPerft(const BoardT& board, const bool doSplit, const bool makeMoves, const int maxTtDepth, const int depthToGo, const int ttSize, const int nTtParts, const int nThreads) {
      // Collect all depth-2 positions - set of FEN's
      std::list<std::pair<std::string, MoveInfoT>> depth2FensAndMoves;
      const Depth2CollectorStateT depth2CollectorState(depth2FensAndMoves, /*depth*/0);
      MakeMove::makeAllLegalMoves<const Depth2CollectorStateT&, Depth2CollectorPosHandlerT<BoardT, Color>, BoardT, Color>(depth2CollectorState, board);

      // Perft stats for each depth-2 position
      std::map<std::string, PerftStatsT> depth2PosStats;

      // TT map: fen->stats for each depth
      std::vector<std::vector<BoundedHashMap<std::string, PerftStatsT>>> tts(nTtParts);
      for(int partNo = 0; partNo < nTtParts; partNo++) {
	for(int i = MinTtDepth; i <= maxTtDepth; i++) {
	  tts[partNo].push_back(BoundedHashMap<std::string, PerftStatsT>(ttSize));
	}
      }
      // TT usage stats - for each thread
      std::vector<std::vector<std::pair<u64, u64>>> threadTtStats(nThreads);
      for(int i = 0; i < nThreads; i++) {
	threadTtStats[i] = std::vector<std::pair<u64, u64>>(maxTtDepth == 0 ? 0 : (maxTtDepth-MinTtDepth+1));
      }
      
      // Mutex to lock all accesses to input list of depth2FensAndMoves and output map 
      std::mutex workerMutex;
      // Run worker threads to process the depth-2 positions in parallel
      std::vector<std::thread> workers;
      for(int i = 0; i < nThreads; i++) {
	workers.push_back(std::thread(paraPerftWorkerFn, i, std::ref(workerMutex), std::ref(depth2FensAndMoves), std::ref(depth2PosStats), std::ref(tts), std::ref(threadTtStats[i]), makeMoves, maxTtDepth, depthToGo)); 
      }
      for(int i = 0; i < nThreads; i++) {
	workers[i].join();
      }

      PerftStatsT stats = {};
      Depth2AccumulatorStateT depth2AccumulatorState(stats, depth2PosStats, doSplit, /*depth*/0);
      MakeMove::makeAllLegalMoves<const Depth2AccumulatorStateT&, Depth2AccumulatorPosHandlerT<BoardT, Color>, BoardT, Color>(depth2AccumulatorState, board);

      // Accumulate the stats from all threads
      std::vector<std::pair<u64, u64>> ttStats(maxTtDepth == 0 ? 0 : (maxTtDepth-MinTtDepth+1));
      for(int threadNo = 0; threadNo < nThreads; threadNo++) {
	for(size_t i = 0; i < threadTtStats[threadNo].size(); i++) {
	  ttStats[i].first += threadTtStats[threadNo][i].first;
	  ttStats[i].second += threadTtStats[threadNo][i].second;
	}
      }
      
      return std::make_pair(stats, ttStats);
    }

    
  } // namespace Perf
} // namespace Chess

#endif //ndef PERFT_HPP
