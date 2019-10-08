#ifndef PERFT_HPP
#define PERFT_HPP

#include "types.hpp"
#include "board.hpp"
#include "move-gen.hpp"
#include "bits.hpp"

namespace Chess {
  using namespace MoveGen;
  
  namespace Perft {

    struct PerftStatsT {
      u64 nodes;
      u64 captures;
      u64 eps;
      u64 castles;
      u64 promos;
      u64 checks;
      u64 checkmates;
      u64 stalemates;
      u64 invalids;
    };

    template <ColorT Color> inline void perftImpl(PerftStatsT& stats, const BoardT& board, const int depthToGo);
  
    inline void addStats(PerftStatsT& stats, const PerftStatsT& moreStats) {
      stats.nodes += moreStats.nodes;
      stats.captures += moreStats.captures;
      stats.eps += moreStats.eps;
      stats.castles += moreStats.castles;
      stats.promos += moreStats.promos;
      stats.checks += moreStats.checks;
      stats.checkmates += moreStats.checkmates;
      stats.stalemates += moreStats.stalemates;
    }

    template <ColorT Color> SquareT pawnPushOneTo2From(SquareT square);
    template <> SquareT pawnPushOneTo2From<White>(SquareT square) { return square - 8; }
    template <> SquareT pawnPushOneTo2From<Black>(SquareT square) { return square + 8; }

    template <ColorT Color> struct PawnPushOneTo2FromFn {
      static SquareT fn(SquareT from) { return pawnPushOneTo2From<Color>(from); }
    };

    template <ColorT Color> SquareT pawnPushTwoTo2From(SquareT square);
    template <> SquareT pawnPushTwoTo2From<White>(SquareT square) { return square - 16; }
    template <> SquareT pawnPushTwoTo2From<Black>(SquareT square) { return square + 16; }

    template <ColorT Color> struct PawnPushTwoTo2FromFn {
      static SquareT fn(SquareT from) { return pawnPushTwoTo2From<Color>(from); }
    };

    template <ColorT Color, typename To2FromFn> inline void perftImplPawnsPush(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPush) {
      // No captures here - these are just pawn pushes and already filtered from all target clashes.
      while(pawnsPush) {
	SquareT to = Bits::popLsb(pawnsPush);
	SquareT from = To2FromFn::fn(to);

	BoardT newBoard = move<Color, /*isCapture =*/false>(board, from, to);

	perftImpl<otherColor<Color>::value>(stats, newBoard, depthToGo-1);
      }
    }

    template <ColorT Color> inline void perftImplPawnsPushOne(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPushOne) {
      perftImplPawnsPush<Color, PawnPushOneTo2FromFn<Color>>(stats, board, depthToGo, pawnsPushOne);
    }
    
    template <ColorT Color> inline void perftImplPawnsPushTwo(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPushTwo) {
      perftImplPawnsPush<Color, PawnPushTwoTo2FromFn<Color>>(stats, board, depthToGo, pawnsPushTwo);
    }

    template <ColorT Color> SquareT pawnAttackLeftTo2From(SquareT square);
    template <> SquareT pawnAttackLeftTo2From<White>(SquareT square) { return square - 7; }
    template <> SquareT pawnAttackLeftTo2From<Black>(SquareT square) { return square + 9; }

    template <ColorT Color> struct PawnAttackLeftTo2FromFn {
      static SquareT fn(SquareT from) { return pawnAttackLeftTo2From<Color>(from); }
    };

    template <ColorT Color> SquareT pawnAttackRightTo2From(SquareT square);
    template <> SquareT pawnAttackRightTo2From<White>(SquareT square) { return square - 9; }
    template <> SquareT pawnAttackRightTo2From<Black>(SquareT square) { return square + 7; }

    template <ColorT Color> struct PawnAttackRightTo2FromFn {
      static SquareT fn(SquareT from) { return pawnAttackRightTo2From<Color>(from); }
    };

    template <ColorT Color, typename To2FromFn> inline void perftImplPawnsCapture(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCapture) {
      while(pawnsCapture) {
	SquareT to = Bits::popLsb(pawnsCapture);
	SquareT from = To2FromFn::fn(to);

	BoardT newBoard = move<Color, /*isCapture =*/true>(board, from, to);

	stats.captures++;
	printf("               pawn capture color %d: %d->%d\n", Color, from, to);

	perftImpl<otherColor<Color>::value>(stats, newBoard, depthToGo-1);
      }
    }

    template <ColorT Color> inline void perftImplPawnsCaptureLeft(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureLeft) {
      perftImplPawnsCapture<Color, PawnAttackLeftTo2FromFn<Color>>(stats, board, depthToGo, pawnsCaptureLeft);
    }
    
    template <ColorT Color> inline void perftImplPawnsCaptureRight(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureRight) {
      perftImplPawnsCapture<Color, PawnAttackRightTo2FromFn<Color>>(stats, board, depthToGo, pawnsCaptureRight);
    }

    template <ColorT Color, bool IsCapture> inline void perftImplPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT toBb) {
      while(toBb) {
	SquareT to = Bits::popLsb(toBb);

	BoardT newBoard = move<Color, IsCapture>(board, from, to);

	if(IsCapture) {
	  stats.captures++;
	}

	perftImpl<otherColor<Color>::value>(stats, newBoard, depthToGo-1);
      }
    }
    
    template <ColorT Color> inline void perftImplPiecePushes(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT pushesBb) {
      perftImplPieceMoves<Color, /*IsCapture =*/false>(stats, board, depthToGo, from, pushesBb);
    }
    
    template <ColorT Color> inline void perftImplPieceCaptures(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT capturesBb) {
      perftImplPieceMoves<Color, /*IsCapture =*/true>(stats, board, depthToGo, from, capturesBb);
    }
    
    template <ColorT Color> inline void perftImplPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT attacksBb, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb) {
      perftImplPiecePushes<Color>(stats, board, depthToGo, from, attacksBb & ~allPiecesBb);
      perftImplPieceCaptures<Color>(stats, board, depthToGo, from, attacksBb & allYourPiecesBb);
    }
    
    template <ColorT Color> inline void perftImpl(PerftStatsT& stats, const BoardT& board, const int depthToGo) {

      const PiecesForColorT& myPieces = board.pieces[Color];
      const PiecesForColorT& yourPieces = board.pieces[otherColor<Color>::value];
      const BitBoardT allMyPiecesBb = myPieces.bbs[AllPieces];
      const BitBoardT allYourPiecesBb = yourPieces.bbs[AllPieces];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;
  
      // Generate moves
      PieceAttacksT myAttacks = genPieceAttacks<Color>(myPieces, allPiecesBb);

      // Is your king in check? If so we got here via an illegal move of the pseudo-move-generator
      if((myAttacks.allAttacks & yourPieces.bbs[King]) != 0) {
	// Illegal position - doesn't count
	stats.invalids++;
	return;
      }

      // This is now a legal position.

      PieceAttacksT yourAttacks = genPieceAttacks<otherColor<Color>::value>(yourPieces, allPiecesBb);
      // Is my king in check?
      bool isCheck = false;
      if((yourAttacks.allAttacks & myPieces.bbs[King]) != 0) {
	isCheck = true;
	stats.checks++;
      }

      // If this is a leaf node, we're done.
      if(depthToGo == 0) {
	// TODO - if we're in check do we have to determine if we're in check-mate?
	// TODO - do we need to check for stalemate?
	stats.nodes++;
	return;
      }

      // Remember the number of nodes prior to evaluating children - if the children add no new (valid) nodes then this is checkmate or stalemate
      u64 origNodes = stats.nodes;

      // Evaluate moves

      // Pawn pushes
      perftImplPawnsPushOne<Color>(stats, board, depthToGo, myAttacks.pawnsPushOne);
      perftImplPawnsPushTwo<Color>(stats, board, depthToGo, myAttacks.pawnsPushTwo);

      // Pawn captures
      perftImplPawnsCaptureLeft<Color>(stats, board, depthToGo, myAttacks.pawnsLeftAttacks & allYourPiecesBb);
      perftImplPawnsCaptureRight<Color>(stats, board, depthToGo, myAttacks.pawnsRightAttacks & allYourPiecesBb);

      // Piece moves
      for(SpecificPieceT specificPiece = QueenKnight; specificPiece <= SpecificKing; specificPiece = SpecificPieceT(specificPiece + 1)) {
	perftImplPieceMoves<Color>(stats, board, depthToGo, myPieces.pieceSquares[specificPiece], myAttacks.pieceAttacks[specificPiece], allYourPiecesBb, allPiecesBb);
      }
      
      // If we found no valid child moves then this is checkmate or stalemate
      if(stats.nodes == origNodes) {
	if(isCheck) {
	  stats.checkmates++;
	} else {
	  stats.stalemates++;
	}
      }
    }

    template <ColorT Color> inline PerftStatsT perft(const BoardT& board, const int depthToGo) {
      PerftStatsT stats = {0};

      perftImpl<Color>(stats, board, depthToGo);

      return stats;
    }

  } // namespace Perf
} // namespace Chess

#endif //ndef PERFT_HPP
