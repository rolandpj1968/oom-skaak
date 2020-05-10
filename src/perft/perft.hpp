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
      
      inline static void handleCount(PerftStatsT& stats, const u64 nodes, u64 captures, u64 eps, u64 castles, u64 promos) {
	stats.nodes += nodes;
	stats.captures += captures;
	stats.eps += eps;
	stats.castles += castles;
	stats.promos += promos;
      }
    };

    template <typename BoardT, ColorT Color>
    inline bool hasLegalMoves(const BoardT& board) {
      typedef typename MoveGen::LegalMovesImplType<BoardT>::LegalMovesT LegalMovesT;
      
      // Generate (legal) moves
      const LegalMovesT legalMoves = MoveGen::genLegalMoves<BoardT, Color>(board);

      // Are there any?
      const BitBoardT anyLegalMovesBb =
	  
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

      // TODO - promos TODO TODO TODO!!!
	
      return anyLegalMovesBb != BbNone;
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
    inline PerftStatsT perft(const BoardT& board, const bool makeMoves, const int depthToGo) {
      PerftStatsT stats = {};
      const int nChecks = BoardUtils::getNChecks<BoardT, Color>(board);
      MoveInfoT dummyMoveInfo(PushMove, /*from*/InvalidSquare, /*to*/InvalidSquare, /*isDirectCheck*/(nChecks > 0), /*isDiscoveredCheck*/(nChecks > 1));
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
    inline PerftStatsT splitPerft(const BoardT& board, const bool makeMoves, const int depthToGo) {
      PerftStatsT stats = {};
      const int nChecks = BoardUtils::getNChecks<BoardT, Color>(board);
      MoveInfoT dummyMoveInfo(PushMove, /*from*/InvalidSquare, /*to*/InvalidSquare, /*isDirectCheck*/(nChecks > 0), /*isDiscoveredCheck*/(nChecks > 1));
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
      std::vector<BoundedHashMap<std::string, PerftStatsT>>& tts;
      std::vector<std::pair<u64, u64>>& ttStats; // (total-nodes, ht-hits)
      const bool doSplit;
      const bool makeMoves;
      const u8 maxTtDepth;
      const u8 depth;
      const u8 depthToGo;

      TtPerftStateT(PerftStatsT& stats, std::vector<BoundedHashMap<std::string, PerftStatsT>>& tts, std::vector<std::pair<u64, u64>>& ttStats, const bool doSplit, const bool makeMoves, const u8 maxTtDepth, const u8 depth, const u8 depthToGo):
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

	// Probe the TT
	const int ttIndex = state.depth - MinTtDepth;
	if(MinTtDepth <= state.depth && state.depth <= state.maxTtDepth) {
	  state.ttStats[ttIndex].first++;
	  // Omit the EP square in cases where EP capture is impossible - this gives us more transpositions
	  fen = Fen::toFenFast<BoardT>(board, Color, /*trimEp*/true);
	  foundIt = state.tts[ttIndex].copy_if_present(fen, splitStats);
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
	  state.tts[ttIndex].put(fen, splitStats);
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
	perftImplFull<BoardT, Color>(perftState, board);
      }
    }
      
    template <typename BoardT, ColorT Color>
    inline std::pair<PerftStatsT, std::vector<std::pair<u64, u64>>> ttPerft(const BoardT& board, const bool doSplit, const bool makeMoves, const int maxTtDepth, const int depthToGo, const int ttSize) {
      PerftStatsT stats = {};
      // map: fen->stats for each depth
      std::vector<BoundedHashMap<std::string, PerftStatsT>> tts;
      for(int i = MinTtDepth; i <= maxTtDepth; i++) {
	tts.push_back(BoundedHashMap<std::string, PerftStatsT>(ttSize));
      }
      std::vector<std::pair<u64, u64>> ttStats(maxTtDepth-MinTtDepth+1);
      
      const int nChecks = BoardUtils::getNChecks<BoardT, Color>(board);
      MoveInfoT dummyMoveInfo(PushMove, /*from*/InvalidSquare, /*to*/InvalidSquare, /*isDirectCheck*/(nChecks > 0), /*isDiscoveredCheck*/(nChecks > 1));
      const TtPerftStateT state(stats, tts, ttStats, doSplit, makeMoves, maxTtDepth, 0, depthToGo);

      ttPerftImpl<BoardT, Color>(state, board, dummyMoveInfo);

      return std::make_pair(stats, ttStats);
    }
  } // namespace Perf
} // namespace Chess

#endif //ndef PERFT_HPP
