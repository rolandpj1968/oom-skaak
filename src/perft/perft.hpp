#ifndef PERFT_HPP
#define PERFT_HPP

#include "types.hpp"
#include "board.hpp"
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
      inline static void handlePos(const PerftStateT state, const BoardT& board, MoveInfoT moveInfo) {
	perftImpl<typename BoardTraitsT::ReverseT>(state, board, moveInfo);
      }
    };

    template <typename BoardTraitsT>
    inline void perft0Impl(PerftStatsT& stats, const BoardT& board, const MoveInfoT moveInfo) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;

      const ColorStateT& myState = board.pieces[(size_t)Color];
      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieceTypes];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      // It is strictly a bug if we encounter an invalid position - we are doing legal (only) move evaluation.
      const bool CheckForInvalid = false;
      if(CheckForInvalid) {
	// Is your king in check? If so we got here via an illegal move of the pseudo-move-generator
	const SquareAttackersT yourKingAttackers = genSquareAttackers<MyColorTraitsT>(yourState.pieceSquares[TheKing], myState, allPiecesBb);
	if(yourKingAttackers.pieceAttackers[AllPieceTypes] != 0) {
	  // Illegal position - doesn't count
	  stats.invalids++;
	  static bool done = false;
	  if(!done) {
	    printf("\n============================================== Invalid Depth 0 - last move to %d! ===================================\n\n", moveInfo.to);
	    printBoard(board);
	    printf("\n");
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

      // Is my king in check?
      const SquareAttackersT myKingAttackers = genSquareAttackers<YourColorTraitsT>(myState.pieceSquares[TheKing], yourState, allPiecesBb);
      const BitBoardT allMyKingAttackers = myKingAttackers.pieceAttackers[AllPieceTypes];
      if(allMyKingAttackers != 0) {
	stats.checks++;

	// If the moved piece is not attacking the king then this is a discovered check
	bool isDiscovery = false;
	if((bbForSquare(moveInfo.to) & allMyKingAttackers) == 0) {
	  isDiscovery = true;
	  stats.discoverychecks++;
	}
	  
	// If there are multiple king attackers then we have a double check
	bool isDoubleCheck = false;
	if(Bits::count(allMyKingAttackers) != 1) {
	  isDoubleCheck = true;
	  stats.doublechecks++;
	}

	if(DoCheckMateStats) {
	  bool isPossibleCheckmate = true;
	  // If it's a non-discovery and we can take the checker then it's not checkmate - BROKKEN; not sure why? Maybe cos of discovered check through the taker!
	  if(false && isPossibleCheckmate && !isDiscovery && !isDoubleCheck) {
	    // See if the checking piece can be taken
	    stats.l0nondiscoveries++;
	    const SquareAttackersT checkerAttackers = genSquareAttackers<MyColorTraitsT>(moveInfo.to, myState, allPiecesBb);
	    // It's not safe for the king to capture cos he might still be in check
	    if(checkerAttackers.pieceAttackers[AllPieceTypes] &~ checkerAttackers.pieceAttackers[King]) {
	      stats.l0checkertakable++;
	      isPossibleCheckmate = false;
	    }
	  }
	  // It's not checkmate if the king can move to safety
	  if(isPossibleCheckmate) {
	    // Remove my king to expose moving away from a slider checker
	    const PieceAttacksT yourAttacks = genPieceAttacks<YourColorTraitsT>(yourState, allPiecesBb & ~myState.bbs[King]);
	    const SquareT myKingSq = myState.pieceSquares[TheKing];
	    const BitBoardT myKingAttacksBb = KingAttacks[myKingSq];
	    const BitBoardT myKingMovesBb = myKingAttacksBb & ~allMyPiecesBb;
	    if(myKingMovesBb & ~yourAttacks.allAttacks) {
	      stats.l0checkkingcanmove++;
	      isPossibleCheckmate = false;
	    }
	  }
	  // TODO - or king can take checking piece - actually not - king could still be in check!
	  // It's checkmate if there are no valid child nodes.
	  if(isPossibleCheckmate) {
	    PerftStatsT childStats = perft<BoardTraitsT>(board, 1);
	    if(childStats.nodes == 0) {
	      stats.checkmates++;
	    }
	  }
	}
      }
    }

    template <typename BoardTraitsT>
    inline void perftImplFull(const PerftStateT state, const BoardT& board, const MoveInfoT moveInfo) {
      
      const PerftStateT newState(state.stats, state.depthToGo-1);

      makeAllLegalMoves<const PerftStateT, PerftPosHandlerT<BoardTraitsT>, BoardTraitsT>(newState, board);
    }

    template <typename BoardTraitsT>
    inline void perftImpl(const PerftStateT state, const BoardT& board, const MoveInfoT moveInfo) {
      // If this is a leaf node, gather stats.
      if(state.depthToGo == 0) {
	perft0Impl<BoardTraitsT>(state.stats, board, moveInfo);
      } else {
	perftImplFull<BoardTraitsT>(state, board, moveInfo);
      }
    }
      
    template <typename BoardTraitsT>
    inline PerftStatsT perft(const BoardT& board, const int depthToGo) {
      PerftStatsT stats = {0};
      const PerftStateT state(stats, depthToGo);

      perftImpl<BoardTraitsT>(state, board, MoveInfoT(PushMove, InvalidSquare));

      return stats;
    }

  } // namespace Perf
} // namespace Chess

#endif //ndef PERFT_HPP
