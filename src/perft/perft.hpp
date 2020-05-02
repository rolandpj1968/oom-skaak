#ifndef PERFT_HPP
#define PERFT_HPP

#include "types.hpp"
#include "board.hpp"
#include "board-utils.hpp"
#include "fen.hpp"
#include "move-gen.hpp"
#include "make-move.hpp"
#include "bits.hpp"

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
      u64 invalids;
      u64 nposwitheps;
      u64 epdiscoveries;
      u64 ephorizdiscoveries;
      u64 epdiagfromdiscoveries;
      u64 epdiagcapturediscoveries;
    };

    struct PerftStateT {
      PerftStatsT& stats;
      int depthToGo;

      PerftStateT(PerftStatsT& stats, int depthToGo):
	stats(stats), depthToGo(depthToGo) {}
    };

    template <typename BoardT, ColorT Color>
    inline PerftStatsT perft(const BoardT& board, const int depthToGo);
    
    template <typename BoardT, ColorT Color>
    inline void perftImpl(const PerftStateT state, const BoardT& board, const MoveInfoT moveInfo);
  
    template <typename BoardT, ColorT Color>
    struct PerftPosHandlerT {
      typedef PerftPosHandlerT<BoardT, OtherColorT<Color>::value> ReverseT;
      typedef PerftPosHandlerT<typename BoardType<BoardT>::WithPromosT, Color> WithPromosT;
      typedef PerftPosHandlerT<typename BoardType<BoardT>::WithoutPromosT, Color> WithoutPromosT;
      
      static const bool ValidatePos = false;
      
      inline static void validatePos(const BoardT& board, MoveInfoT moveInfo) {
	static bool done = false;
	if(ValidatePos && !done) {
	  if(!BoardUtils::isValid<BoardT, Color>(board)) {
	    printf("Invalid board - last move from %s to %s\n", SquareStr[moveInfo.from], SquareStr[moveInfo.to]);
	    BoardUtils::printBoard(board);
	    //done = true;
	  }
	}
      }
      
      inline static void handlePos(const PerftStateT state, const BoardT& board, MoveInfoT moveInfo) {
	validatePos(board, moveInfo);
	perftImpl<BoardT, Color>(state, board, moveInfo);
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

      // BoardUtils::printBoard(board);
      // printf("%s\n", Fen::toFen(board, Color).c_str());

      // TODO get rid...
      if(false) {
	static bool done = false;
	if(board.state[(size_t)otherColor(Color)].basic.epSquare != InvalidSquare) {
	  stats.nposwitheps++;
	  if(!done) {
	    printf("\n============================================== EP set at Depth 0 - last move %s-%s ===================================\n\n", SquareStr[moveInfo.from], SquareStr[moveInfo.to]);
	    BoardUtils::printBoard(board);
	    printf("\n%s\n\n", Fen::toFen(board, Color).c_str());
	    done = true;
	  }
	}
      }	    

      // TODO get rid...
      if(false) {
	static bool done = false;
	if(moveInfo.moveType == EpCaptureMove) {
	  stats.nposwitheps++;
	  if(!done) {
	    printf("\n============================================== EP capture Depth 0 - last move %s-%s ===================================\n\n", SquareStr[moveInfo.from], SquareStr[moveInfo.to]);
	    BoardUtils::printBoard(board);
	    printf("\n%s\n\n", Fen::toFen(board, Color).c_str());
	    //done = true;
	  }
	}
      }	    

      // It is strictly a bug if we encounter an invalid position - we are doing legal (only) move evaluation.
      const bool CheckForInvalid = false;
      if(CheckForInvalid) {
	if(!BoardUtils::isValid<BoardT, Color>(board)) {
	  // Illegal position - doesn't count
	  stats.invalids++;
	  static bool done = false;
	  if(!done) {
	    printf("\n============================================== Invalid Depth 0 - last move %s-%s ===================================\n\n", SquareStr[moveInfo.from], SquareStr[moveInfo.to]);
	    BoardUtils::printBoard(board);
	    printf("\n%s\n\n", Fen::toFen(board, Color).c_str());
	    done = true;
	  }
	  return;
	}
      }
	
      // Check that we detect check accurately
      const bool CheckChecks = false;
      if(CheckChecks) {
	int nChecks = BoardUtils::getNChecks<BoardT, Color>(board);
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
	    BoardUtils::printBoard(board);
	    printf("\n%s\n\n", Fen::toFen(board, Color).c_str());
	    done = true;
	  }
	}
      }
	
      stats.nodes++;

      if(moveInfo.moveType == CaptureMove) {
	stats.captures++;
      }

      if(moveInfo.moveType == EpCaptureMove) {
	stats.captures++;
	stats.eps++;
	if(moveInfo.isDiscoveredCheck && !moveInfo.isDirectCheck) {
	  stats.epdiscoveries++;
	  const BitBoardT kingBishopRays = MoveGen::BishopRays[board.state[(size_t)Color].basic.pieceSquares[TheKing]];
	  const BitBoardT kingRookRays = MoveGen::RookRays[board.state[(size_t)Color].basic.pieceSquares[TheKing]];
	  const BitBoardT fromBb = bbForSquare(moveInfo.from);
	  if(kingRookRays & fromBb) {
	    stats.ephorizdiscoveries++;
	  } else {
	    if(fromBb & kingBishopRays) {
	      stats.epdiagfromdiscoveries++;
	    } else {
	      stats.epdiagcapturediscoveries++;
	    }
	  }
	}
      }

      if(moveInfo.moveType == CastlingMove) {
	stats.castles++;
      }

      if(moveInfo.isPromo) {
	stats.promos++;
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
	  if(!hasLegalMoves<BoardT, Color>(board)) {
	    stats.checkmates++;
	  }
	}
      }
    }

    template <typename BoardT, ColorT Color>
    inline void perftImplFull(const PerftStateT state, const BoardT& board) {

      // TODO get rid...
      if(false && state.depthToGo == 1) {
	static bool done = false;
	if(board.state[(size_t)otherColor(Color)].basic.epSquare != InvalidSquare) {
	  if(!done) {
	    printf("\n============================================== EP sq %s set at Depth 1 ===================================\n\n", SquareStr[board.state[(size_t)otherColor(Color)].basic.epSquare]);
	    BoardUtils::printBoard(board);
	    printf("\n%s\n\n", Fen::toFen(board, Color).c_str());
	    //done = true;
	  }
	}
      }	    
      
      const PerftStateT newState(state.stats, state.depthToGo-1);

      MakeMove::makeAllLegalMoves<const PerftStateT, PerftPosHandlerT<BoardT, Color>, BoardT, Color>(newState, board);
    }

    template <typename BoardT, ColorT Color>
    inline void perftImpl(const PerftStateT state, const BoardT& board, const MoveInfoT moveInfo) {
      // If this is a leaf node, gather stats.
      if(state.depthToGo == 0) {
	perft0Impl<BoardT, Color>(state.stats, board, moveInfo);
      } else {
	perftImplFull<BoardT, Color>(state, board);
      }
    }
      
    template <typename BoardT, ColorT Color>
    inline PerftStatsT perft(const BoardT& board, const int depthToGo) {
      PerftStatsT stats = {};
      // TODO - fill in isDirectCheck and isDiscoveredCheck in order to hoist check detection out of genLegalMoves
      const int nChecks = BoardUtils::getNChecks<BoardT, Color>(board);
      MoveInfoT dummyMoveInfo(PushMove, /*from*/InvalidSquare, /*to*/InvalidSquare, /*isDirectCheck*/(nChecks > 0), /*isDiscoveredCheck*/(nChecks > 1));
      const PerftStateT state(stats, depthToGo);

      perftImpl<BoardT, Color>(state, board, dummyMoveInfo);

      return stats;
    }

  } // namespace Perf
} // namespace Chess

#endif //ndef PERFT_HPP
