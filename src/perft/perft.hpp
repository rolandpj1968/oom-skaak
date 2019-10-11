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
      u64 invalids;
      u64 allpieces;
      u64 allmypieces;
    };

    enum CaptureT {
      NoCapture = 0,
      NormalCapture,
      EpCapture,
    };

    template <ColorT Color>
    inline PerftStatsT perft(const BoardT& board, const int depthToGo);
    
    template <ColorT Color, PushOrCaptureT PushOrCapture, bool IsEpCapture = false>
    inline void perftImpl(PerftStatsT& stats, const BoardT& board, const int depthToGo, const CaptureT captureType);
  
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

    template <ColorT Color, typename To2FromFn, bool IsPushTwo>
    inline void perftImplPawnsPush(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPush) {
      // No captures here - these are just pawn pushes and already filtered from all target clashes.
      while(pawnsPush) {
	SquareT to = Bits::popLsb(pawnsPush);
	SquareT from = To2FromFn::fn(to);

	BoardT newBoard = moveSpecificPiece<Color, SpecificPawn, Push, IsPushTwo>(board, from, to);

	perftImpl<OtherColorT<Color>::value, Push>(stats, newBoard, depthToGo-1, NoCapture);
      }
    }

    template <ColorT Color> inline void perftImplPawnsPushOne(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPushOne) {
      perftImplPawnsPush<Color, PawnPushOneTo2FromFn<Color>, /*IsPushTwo =*/false>(stats, board, depthToGo, pawnsPushOne);
    }
    
    template <ColorT Color> inline void perftImplPawnsPushTwo(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPushTwo) {
      perftImplPawnsPush<Color, PawnPushTwoTo2FromFn<Color>, /*IsPushTwo =*/true>(stats, board, depthToGo, pawnsPushTwo);
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

	BoardT newBoard = moveSpecificPiece<Color, SpecificPawn, Capture>(board, from, to);

	perftImpl<OtherColorT<Color>::value, Capture>(stats, newBoard, depthToGo-1, NormalCapture);
      }
    }

    template <ColorT Color> inline void perftImplPawnsCaptureLeft(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureLeft) {
      perftImplPawnsCapture<Color, PawnAttackLeftTo2FromFn<Color>>(stats, board, depthToGo, pawnsCaptureLeft);
    }
    
    template <ColorT Color> inline void perftImplPawnsCaptureRight(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureRight) {
      perftImplPawnsCapture<Color, PawnAttackRightTo2FromFn<Color>>(stats, board, depthToGo, pawnsCaptureRight);
    }

    template <ColorT Color, typename To2FromFn>
    inline void perftImplPawnEpCapture(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsEpCapture) {
      // There can be only 1 en-passant capture, so no need to loop
      if(pawnsEpCapture) {
	SquareT to = Bits::popLsb(pawnsEpCapture);
	SquareT from = To2FromFn::fn(to);
	SquareT captureSquare = pawnPushOneTo2From<Color>(to);

	BoardT newBoard = captureEp<Color>(board, from, to, captureSquare);

	perftImpl<OtherColorT<Color>::value, Capture, /*IsEpCapture =*/true>(stats, newBoard, depthToGo-1, EpCapture);
      }
    }

    template <ColorT Color> inline void perftImplPawnEpCaptureLeft(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureLeft) {
      perftImplPawnEpCapture<Color, PawnAttackLeftTo2FromFn<Color>>(stats, board, depthToGo, pawnsCaptureLeft);
    }
    
    template <ColorT Color> inline void perftImplPawnEpCaptureRight(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureRight) {
      perftImplPawnEpCapture<Color, PawnAttackRightTo2FromFn<Color>>(stats, board, depthToGo, pawnsCaptureRight);
    }

    template <ColorT Color, PushOrCaptureT PushOrCapture>
    inline void perftImplPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT toBb, const CaptureT captureType) {
      while(toBb) {
	SquareT to = Bits::popLsb(toBb);

	BoardT newBoard = move<Color, PushOrCapture>(board, from, to);

	perftImpl<OtherColorT<Color>::value, PushOrCapture>(stats, newBoard, depthToGo-1, captureType);
      }
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece, PushOrCaptureT PushOrCapture>
    inline void perftImplSpecificPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT toBb, const CaptureT captureType) {
      while(toBb) {
	SquareT to = Bits::popLsb(toBb);

	BoardT newBoard = moveSpecificPiece<Color, SpecificPiece, PushOrCapture>(board, from, to);

	perftImpl<OtherColorT<Color>::value, PushOrCapture>(stats, newBoard, depthToGo-1, captureType);
      }
    }
    
    template <ColorT Color> inline void perftImplPiecePushes(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT pushesBb) {
      perftImplPieceMoves<Color, Push>(stats, board, depthToGo, from, pushesBb);
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece> inline void perftImplSpecificPiecePushes(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT pushesBb) {
      perftImplSpecificPieceMoves<Color, SpecificPiece, Push>(stats, board, depthToGo, from, pushesBb, NoCapture);
    }
    
    template <ColorT Color> inline void perftImplPieceCaptures(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT capturesBb) {
      perftImplPieceMoves<Color, Capture>(stats, board, depthToGo, from, capturesBb);
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece> inline void perftImplSpecificPieceCaptures(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT capturesBb) {
      perftImplSpecificPieceMoves<Color, SpecificPiece, Capture>(stats, board, depthToGo, from, capturesBb, NormalCapture);
    }
    
    template <ColorT Color> inline void perftImplPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT attacksBb, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb) {
      perftImplPiecePushes<Color>(stats, board, depthToGo, from, attacksBb & ~allPiecesBb);
      perftImplPieceCaptures<Color>(stats, board, depthToGo, from, attacksBb & allYourPiecesBb);
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece>
    inline void perftImplSpecificPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT attacksBb, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb) {
      perftImplSpecificPiecePushes<Color, SpecificPiece>(stats, board, depthToGo, from, attacksBb & ~allPiecesBb);
      perftImplSpecificPieceCaptures<Color, SpecificPiece>(stats, board, depthToGo, from, attacksBb & allYourPiecesBb);
    }
    
    template <ColorT Color, PushOrCaptureT PushOrCapture, bool IsEpCapture = false, PiecePresentFlagsT PiecesPresent = AllPiecesPresentFlags, bool UseRuntimeChecks = true>
    inline void perftImplTemplate(PerftStatsT& stats, const BoardT& board, const int depthToGo, const CaptureT captureType) {

      const ColorStateT& myState = board.pieces[Color];
      const ColorStateT& yourState = board.pieces[OtherColorT<Color>::value];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieces];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieces];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;
  
      // Generate moves
      PieceAttacksT myAttacks = genPieceAttacks<Color, PiecesPresent, UseRuntimeChecks>(myState, allPiecesBb);

      // Is your king in check? If so we got here via an illegal move of the pseudo-move-generator
      if((myAttacks.allAttacks & yourState.bbs[King]) != 0) {
	// Illegal position - doesn't count
	stats.invalids++;
	return;
      }

      // This is now a legal position.

      // If this is a leaf node, we're done.
      if(depthToGo == 0) {
	// TODO - if we're in check do we have to determine if we're in check-mate?
	// TODO - do we need to check for stalemate?
	stats.nodes++;

	if(PushOrCapture == Capture) {
	  stats.captures++;
	}

	if(IsEpCapture) {
	  stats.eps++;
	}

	if(myState.piecesPresent == StartingPiecesPresentFlags) {
	  stats.allmypieces++;

	  if(yourState.piecesPresent == StartingPiecesPresentFlags) {
	    stats.allpieces++;
	  }
	}

	// Is my king in check?
	PieceAttacksT yourAttacks = genPieceAttacks<OtherColorT<Color>::value>(yourState, allPiecesBb);
	if((yourAttacks.allAttacks & myState.bbs[King]) != 0) {
	  stats.checks++;

	  // It's checkmate if there are no valid child nodes.
	  PerftStatsT childStats = perft<Color>(board, 1);
	  if(childStats.nodes == 0) {
	    stats.checkmates++;
	  }
	}

	return;
      }

      // Evaluate moves

      PiecePresentFlagsT piecesPresent = myState.piecesPresent;

      // Pawn pushes
      if((PiecesPresent & PawnsPresentFlag) && (!UseRuntimeChecks || (piecesPresent & PawnsPresentFlag))) {
	perftImplPawnsPushOne<Color>(stats, board, depthToGo, myAttacks.pawnsPushOne);
	perftImplPawnsPushTwo<Color>(stats, board, depthToGo, myAttacks.pawnsPushTwo);
	
	// Pawn captures
	perftImplPawnsCaptureLeft<Color>(stats, board, depthToGo, myAttacks.pawnsLeftAttacks & allYourPiecesBb);
	perftImplPawnsCaptureRight<Color>(stats, board, depthToGo, myAttacks.pawnsRightAttacks & allYourPiecesBb);

	// Pawn en-passant captures
	if(yourState.epSquare) {
	  BitBoardT epSquareBb = bbForSquare(yourState.epSquare);
	  
	  perftImplPawnEpCaptureLeft<Color>(stats, board, depthToGo, myAttacks.pawnsLeftAttacks & epSquareBb);
	  perftImplPawnEpCaptureRight<Color>(stats, board, depthToGo, myAttacks.pawnsRightAttacks & epSquareBb);
	}
      }

      // Piece moves

      // Knights
      
      if((PiecesPresent & QueenKnightPresentFlag) && (!UseRuntimeChecks || (piecesPresent & QueenKnightPresentFlag))) {
	perftImplSpecificPieceMoves<Color, QueenKnight>(stats, board, depthToGo, myState.pieceSquares[QueenKnight], myAttacks.pieceAttacks[QueenKnight], allYourPiecesBb, allPiecesBb);
      }

      if((PiecesPresent & KingKnightPresentFlag) && (!UseRuntimeChecks || (piecesPresent & KingKnightPresentFlag))) {
	perftImplSpecificPieceMoves<Color, KingKnight>(stats, board, depthToGo, myState.pieceSquares[KingKnight], myAttacks.pieceAttacks[KingKnight], allYourPiecesBb, allPiecesBb);
      }

      // Bishops
      
      if((PiecesPresent & BlackBishopPresentFlag) && (!UseRuntimeChecks || (piecesPresent & BlackBishopPresentFlag))) {
	perftImplSpecificPieceMoves<Color, BlackBishop>(stats, board, depthToGo, myState.pieceSquares[BlackBishop], myAttacks.pieceAttacks[BlackBishop], allYourPiecesBb, allPiecesBb);
      }

      if((PiecesPresent & WhiteBishopPresentFlag) && (!UseRuntimeChecks || (piecesPresent & WhiteBishopPresentFlag))) {
	perftImplSpecificPieceMoves<Color, WhiteBishop>(stats, board, depthToGo, myState.pieceSquares[WhiteBishop], myAttacks.pieceAttacks[WhiteBishop], allYourPiecesBb, allPiecesBb);
      }

      // Rooks

      if((PiecesPresent & QueenRookPresentFlag) && (!UseRuntimeChecks || (piecesPresent & QueenRookPresentFlag))) {
	perftImplSpecificPieceMoves<Color, QueenRook>(stats, board, depthToGo, myState.pieceSquares[QueenRook], myAttacks.pieceAttacks[QueenRook], allYourPiecesBb, allPiecesBb);
      }

      if((PiecesPresent & KingRookPresentFlag) && (!UseRuntimeChecks || (piecesPresent & KingRookPresentFlag))) {
	perftImplSpecificPieceMoves<Color, KingRook>(stats, board, depthToGo, myState.pieceSquares[KingRook], myAttacks.pieceAttacks[KingRook], allYourPiecesBb, allPiecesBb);
      }

      // Queens

      if((PiecesPresent & QueenPresentFlag) && (!UseRuntimeChecks || (piecesPresent & QueenPresentFlag))) {
	perftImplSpecificPieceMoves<Color, SpecificQueen>(stats, board, depthToGo, myState.pieceSquares[SpecificQueen], myAttacks.pieceAttacks[SpecificQueen], allYourPiecesBb, allPiecesBb);
      }

      if((PiecesPresent & PromoQueenPresentFlag) && (!UseRuntimeChecks || (piecesPresent & PromoQueenPresentFlag))) {
	perftImplSpecificPieceMoves<Color, PromoQueen>(stats, board, depthToGo, myState.pieceSquares[PromoQueen], myAttacks.pieceAttacks[PromoQueen], allYourPiecesBb, allPiecesBb);
      }

      // King always present
      perftImplSpecificPieceMoves<Color, SpecificKing>(stats, board, depthToGo, myState.pieceSquares[SpecificKing], myAttacks.pieceAttacks[SpecificKing], allYourPiecesBb, allPiecesBb);

      // TODO other promo pieces
    }

    typedef void PerftImplFn(PerftStatsT& stats, const BoardT& board, const int depthToGo, const CaptureT captureType);

    template <ColorT Color, PushOrCaptureT PushOrCapture, bool IsEpCapture>
    struct PerftImplDispatcherT {
      static const PerftImplFn* DispatchTable[];
    };

#include <boost/preprocessor/iteration/local.hpp>

#define BOOST_PP_LOCAL_MACRO(n) \
    perftImplTemplate<Color, PushOrCapture, IsEpCapture, (n), false>/*perftImplTemplate<Color, PushOrCapture, IsEpCapture, AllPiecesPresentFlags, true>*/,

#define BOOST_PP_LOCAL_LIMITS (0, MAX_PIECE_PRESENT_FLAGS)
    
    template <ColorT Color, PushOrCaptureT PushOrCapture, bool IsEpCapture>
    const PerftImplFn* PerftImplDispatcherT<Color, PushOrCapture, IsEpCapture>::DispatchTable[] = {
#include BOOST_PP_LOCAL_ITERATE()
    };

    // Disappointingly the clever template specialisation produces code that runs slower than without???
#define PERFT_DIRECT_DISPATCH
    template <ColorT Color, PushOrCaptureT PushOrCapture, bool IsEpCapture = false>
    inline void perftImpl(PerftStatsT& stats, const BoardT& board, const int depthToGo, const CaptureT captureType) {
#ifdef PERFT_DIRECT_DISPATCH
      perftImplTemplate<Color, PushOrCapture, IsEpCapture>(stats, board, depthToGo, captureType);
#else
      const ColorStateT& myState = board.pieces[Color];
      // if(myState.piecesPresent == StartingPiecesPresentFlags) {
      // 	perftImplTemplate<Color, PushOrCapture, IsEpCapture, StartingPiecesPresentFlags, false>(stats, board, depthToGo, captureType);
      // } else {
      // 	perftImplTemplate<Color, PushOrCapture, IsEpCapture>(stats, board, depthToGo, captureType);
      // }
      PerftImplDispatcherT<Color, PushOrCapture, IsEpCapture>::DispatchTable[myState.piecesPresent](stats, board, depthToGo, captureType);
#endif
    }
    
    template <ColorT Color> inline PerftStatsT perft(const BoardT& board, const int depthToGo) {
      PerftStatsT stats = {0};

      perftImpl<Color, Push>(stats, board, depthToGo, NoCapture);

      return stats;
    }

  } // namespace Perf
} // namespace Chess

#endif //ndef PERFT_HPP
