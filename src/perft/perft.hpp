#ifndef PERFT_HPP
#define PERFT_HPP

#include "types.hpp"
#include "board.hpp"
#include "fen.hpp"
#include "move-gen.hpp"
#include "make-move.hpp"
#include "bits.hpp"

namespace Chess {
  using namespace MoveGen;
  using namespace MakeMove;
  
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
      u64 invalids;
      u64 invalidsnon0;
      u64 non0inpinpath;
      u64 non0withdiagpins;
      u64 non0withorthogpins;
      u64 l0nondiscoveries;
      u64 l0checkertakable;
      u64 l0checkkingcanmove;
      u64 directchecks1;
      u64 discoverychecks1;
      u64 doublechecks1;
    };

    struct PerftStateT {
      PerftStatsT& stats;
      int depthToGo;

      PerftStateT(PerftStatsT& stats, int depthToGo):
	stats(stats), depthToGo(depthToGo) {}
    };

    template <typename BoardTraitsT>
    inline PerftStatsT perft(const BoardT& board, const int depthToGo);
    
    template <typename BoardTraitsT>
    inline void perftImpl(const PerftStateT state, const BoardT& board, const MoveInfoT moveInfo);
  
    template <typename BoardTraitsT>
    struct PerftPosHandlerT {
      static const bool ValidatePos = false;
      
      inline static void validatePos(const BoardT& board, MoveInfoT moveInfo) {
	static bool done = false;
	if(ValidatePos && !done) {
	  if(!isValid<typename BoardTraitsT::ReverseT>(board)) {
	    printf("Invalid board - last move from %s to %s\n", SquareStr[moveInfo.from], SquareStr[moveInfo.to]);
	    printBoard(board);
	    //done = true;
	  }
	}
      }
      
      inline static void handlePos(const PerftStateT state, const BoardT& board, MoveInfoT moveInfo) {
	validatePos(board, moveInfo);
	perftImpl<typename BoardTraitsT::ReverseT>(state, board, moveInfo);
      }
    };

    template <typename BoardTraitsT>
    inline bool hasLegalMoves(const BoardT& board, const MoveInfoT moveInfo) {
      // Generate (legal) moves
      const LegalMovesT legalMoves = genLegalMoves<BoardTraitsT>(board);

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

      // TODO - promos
	
      return anyLegalMovesBb != BbNone;
    }

    template <typename BoardTraitsT>
    inline void perft0Impl(PerftStatsT& stats, const BoardT& board, const MoveInfoT moveInfo) {

      // It is strictly a bug if we encounter an invalid position - we are doing legal (only) move evaluation.
      const bool CheckForInvalid = false;
      if(CheckForInvalid) {
	if(!isValid<BoardTraitsT>(board)) {
	  // Illegal position - doesn't count
	  stats.invalids++;
	  static bool done = false;
	  if(!done) {
	    printf("\n============================================== Invalid Depth 0 - last move %s-%s ===================================\n\n", SquareStr[moveInfo.from], SquareStr[moveInfo.to]);
	    printBoard(board);
	    printf("\n%s\n\n", Fen::toFen(board, BoardTraitsT::Color).c_str());
	    done = true;
	  }
	  return;
	}
      }
	
      // Check that we detect check accurately
      const bool CheckChecks = true;
      if(CheckChecks) {
	int nChecks = getNChecks<BoardTraitsT>(board);
	bool isCheck = nChecks > 0;
	bool isCheckDetected = moveInfo.isDirectCheck || moveInfo.isDiscoveredCheck;
	
	bool ok = isCheck == isCheckDetected;
	if(nChecks > 1 && (!moveInfo.isDirectCheck || !moveInfo.isDiscoveredCheck)) {
	  ok = false;
	}

	if(!ok) {
	  // Bad check detection
	  stats.invalids++;
	  static bool done = false;
	  if(!done) {
	    printf("\n================================= Bad Check Detection Depth 0 - nChecks %d direct %d discovery %d last move %s-%s ===================================\n\n", nChecks, (int)moveInfo.isDirectCheck, (int)moveInfo.isDiscoveredCheck, SquareStr[moveInfo.from], SquareStr[moveInfo.to]);
	    printBoard(board);
	    printf("\n%s\n\n", Fen::toFen(board, BoardTraitsT::Color).c_str());
	    done = true;
	  }
	  return;
	}
      }
	
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

      const bool DoCheckStats = true;
      const bool DoCheckMateStats = true;
      if(!DoCheckStats) {
	return;
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

      if(moveInfo.isDirectCheck || moveInfo.isDiscoveredCheck) {
	if(DoCheckMateStats) {
	  // Only bother if it is check
	  // It's checkmate if there are no legal moves
	  if(!hasLegalMoves<BoardTraitsT>(board, moveInfo)) {
	    stats.checkmates++;
	  }
	}
      }
    }

    template <typename BoardTraitsT>
    inline void perftImplFull(const PerftStateT state, const BoardT& board) {
      
      const PerftStateT newState(state.stats, state.depthToGo-1);

      makeAllLegalMoves<const PerftStateT, PerftPosHandlerT<BoardTraitsT>, BoardTraitsT>(newState, board);
    }

    template <typename BoardTraitsT>
    inline void perftImpl(const PerftStateT state, const BoardT& board, const MoveInfoT moveInfo) {
      // If this is a leaf node, gather stats.
      if(state.depthToGo == 0) {
	perft0Impl<BoardTraitsT>(state.stats, board, moveInfo);
      } else {
	perftImplFull<BoardTraitsT>(state, board);
      }
    }
      
    template <typename BoardTraitsT>
    inline PerftStatsT perft(const BoardT& board, const int depthToGo) {
      PerftStatsT stats = {};
      MoveInfoT dummyMoveInfo(PushMove, /*from*/InvalidSquare, /*to*/InvalidSquare, /*isDirectCheck*/false, /*isDiscoveredCheck*/false);
      const PerftStateT state(stats, depthToGo);

      perftImpl<BoardTraitsT>(state, board, dummyMoveInfo);

      return stats;
    }

  } // namespace Perf
} // namespace Chess

#endif //ndef PERFT_HPP
