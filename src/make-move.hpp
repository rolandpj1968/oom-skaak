#ifndef MAKE_MOVE_HPP
#define MAKE_MOVE_HPP

#include "types.hpp"
#include "board.hpp"
#include "move-gen.hpp"

namespace Chess {

  using namespace Board;
  using namespace MoveGen;
  
  namespace MakeMove {

    // Note that typename PosHandlerT must take care of BoardTraitsT::ReverseT - I can't seem to make this explicit which maybe makes sense.

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn, bool IsPushTwo>
    inline void handlePawnsPush(StateT state, const BoardT& board, BitBoardT pawnsPush) {
      while(pawnsPush) {
	const SquareT to = Bits::popLsb(pawnsPush);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = moveSpecificPiece<BoardTraitsT::Color, SpecificPawn, Push, IsPushTwo>(board, from, to);

	PosHandlerT::handlePos(state, newBoard, MoveInfoT(PushMove, to));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnsPushOne(StateT state, const BoardT& board, BitBoardT pawnsPushOne) {
      handlePawnsPush<StateT, PosHandlerT, BoardTraitsT, PawnPushOneTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/false>(state, board, pawnsPushOne);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnsPushTwo(StateT state, const BoardT& board, BitBoardT pawnsPushTwo) {
      handlePawnsPush<StateT, PosHandlerT, BoardTraitsT, PawnPushTwoTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/true>(state, board, pawnsPushTwo);
    }

    template <ColorT Color>
    inline SquareT pawnAttackLeftTo2From(const SquareT square);
    template <> inline SquareT pawnAttackLeftTo2From<White>(const SquareT square) { return square - 7; }
    template <> inline SquareT pawnAttackLeftTo2From<Black>(const SquareT square) { return square + 9; }

    template <ColorT Color>
    struct PawnAttackLeftTo2FromFn {
      static inline SquareT fn(const SquareT from) { return pawnAttackLeftTo2From<Color>(from); }
    };

    template <ColorT Color>
    inline SquareT pawnAttackRightTo2From(const SquareT square);
    template <> inline SquareT pawnAttackRightTo2From<White>(const SquareT square) { return square - 9; }
    template <> inline SquareT pawnAttackRightTo2From<Black>(const SquareT square) { return square + 7; }

    template <ColorT Color>
    struct PawnAttackRightTo2FromFn {
      static inline SquareT fn(const SquareT from) { return pawnAttackRightTo2From<Color>(from); }
    };

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn>
    inline void handlePawnsCapture(StateT state, const BoardT& board, BitBoardT pawnsCapture) {
      while(pawnsCapture) {
	const SquareT to = Bits::popLsb(pawnsCapture);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = moveSpecificPiece<BoardTraitsT::Color, SpecificPawn, Capture>(board, from, to);

	PosHandlerT::handlePos(state, newBoard, MoveInfoT(CaptureMove, to));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnsCaptureLeft(StateT state, const BoardT& board, BitBoardT pawnsCaptureLeft) {
      handlePawnsCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(state, board, pawnsCaptureLeft);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnsCaptureRight(StateT state, const BoardT& board, BitBoardT pawnsCaptureRight) {
      handlePawnsCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(state, board, pawnsCaptureRight);
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn>
    inline void handlePawnEpCapture(StateT state, const BoardT& board, BitBoardT pawnsEpCaptureBb) {
      // There can be only 1 en-passant capture, so no need to loop
      if(pawnsEpCaptureBb) {
	const SquareT to = Bits::lsb(pawnsEpCaptureBb);
	const SquareT from = To2FromFn::fn(to);
	const SquareT captureSq = pawnPushOneTo2From<BoardTraitsT::Color>(to);

	const BoardT newBoard = captureEp<BoardTraitsT::Color>(board, from, to, captureSq);

	PosHandlerT::handlePos(state, newBoard, MoveInfoT(EpCaptureMove, to));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnEpCaptureLeft(StateT state, const BoardT& board, BitBoardT pawnsCaptureLeft) {
      handlePawnEpCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(state, board, pawnsCaptureLeft);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnEpCaptureRight(StateT state, const BoardT& board, BitBoardT pawnsCaptureRight) {
      handlePawnEpCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(state, board, pawnsCaptureRight);
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnMoves(StateT state, const BoardT& board, const PawnPushesAndCapturesT& pawnMoves) {

      // Pawn pushes
      handlePawnsPushOne<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.pushesOneBb);
      handlePawnsPushTwo<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.pushesTwoBb);
	
      // Pawn captures
      handlePawnsCaptureLeft<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.capturesLeftBb);
      handlePawnsCaptureRight<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.capturesRightBb);
      
      // Pawn en-passant captures
      handlePawnEpCaptureLeft<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.epCaptures.epLeftCaptureBb);
      handlePawnEpCaptureRight<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.epCaptures.epRightCaptureBb);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, SpecificPieceT SpecificPiece, PushOrCaptureT PushOrCapture>
    inline void handleSpecificPieceMoves(StateT state, const BoardT& board, const SquareT from, BitBoardT toBb, const MoveTypeT moveType) {
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);

	const BoardT newBoard = moveSpecificPiece<BoardTraitsT::Color, SpecificPiece, PushOrCapture>(board, from, to);

	PosHandlerT::handlePos(state, newBoard, MoveInfoT(moveType, to));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, SpecificPieceT SpecificPiece>
    inline void handleSpecificPiecePushes(StateT state, const BoardT& board, const SquareT from, const BitBoardT pushesBb) {
      handleSpecificPieceMoves<StateT, PosHandlerT, BoardTraitsT, SpecificPiece, Push>(state, board, from, pushesBb, PushMove);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, SpecificPieceT SpecificPiece>
    inline void handleSpecificPieceCaptures(StateT state, const BoardT& board, const SquareT from, const BitBoardT capturesBb) {
      handleSpecificPieceMoves<StateT, PosHandlerT, BoardTraitsT, SpecificPiece, Capture>(state, board, from, capturesBb, CaptureMove);
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, SpecificPieceT SpecificPiece>
    inline void handleSpecificPieceMoves(StateT state, const BoardT& board, const PushesAndCapturesT pushesAndCaptures) {
      const ColorStateT& myState = board.pieces[(size_t)BoardTraitsT::Color];
      const SquareT from = myState.pieceSquares[SpecificPiece];

      handleSpecificPiecePushes<StateT, PosHandlerT, BoardTraitsT, SpecificPiece>(state, board, from, pushesAndCaptures.pushesBb);
      
      handleSpecificPieceCaptures<StateT, PosHandlerT, BoardTraitsT, SpecificPiece>(state, board, from, pushesAndCaptures.capturesBb);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, CastlingRightsT CastlingRight>
    inline void handleCastlingMove(StateT state, const BoardT& board) {
      const ColorT Color = BoardTraitsT::Color;
      
      const BoardT newBoard1 = moveSpecificPiece<Color, SpecificKing, Push>(board, CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo);
      const BoardT newBoard = moveSpecificPiece<Color, CastlingTraitsT<Color, CastlingRight>::SpecificRook, Push>(newBoard1, CastlingTraitsT<Color, CastlingRight>::RookFrom, CastlingTraitsT<Color, CastlingRight>::RookTo);

      // We use the king (from and) to square by convention
      PosHandlerT::handlePos(state, newBoard, MoveInfoT(CastlingMove, CastlingTraitsT<Color, CastlingRight>::KingTo));
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void makeAllLegalMoves(StateT state, const BoardT& board) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;

      // Generate (legal) moves
      const LegalMovesT legalMoves = genLegalMoves<BoardTraitsT>(board);

      // Is this an illegal pos - note this should never happen(tm) - but we will notice quickly
      if(legalMoves.isIllegalPos) {
	static bool done = false;
	if(!done) {
	  printf("\n============================================== Invalid pos! ===================================\n\n");
	  printBoard(board);
	  printf("\n");
	  done = true;
	}
	return;
      }

      // Double check can only be evaded by moving the king so only bother with other pieces if nChecks < 2
      if(legalMoves.nChecks < 2) {

	// Evaluate moves

	// Pawns
	//perftImplPawnMoves<BoardTraitsT>(newState, board, legalMoves.pawnMoves);
	handlePawnMoves<StateT, PosHandlerT, BoardTraitsT>(state, board, legalMoves.pawnMoves);
	
	// Knights
	handleSpecificPieceMoves<StateT, PosHandlerT, BoardTraitsT, QueenKnight>(state, board, legalMoves.specificPieceMoves[QueenKnight]);
	handleSpecificPieceMoves<StateT, PosHandlerT, BoardTraitsT, KingKnight>(state, board, legalMoves.specificPieceMoves[KingKnight]);
	
	// Bishops
	handleSpecificPieceMoves<StateT, PosHandlerT, BoardTraitsT, BlackBishop>(state, board, legalMoves.specificPieceMoves[BlackBishop]); 
	handleSpecificPieceMoves<StateT, PosHandlerT, BoardTraitsT, WhiteBishop>(state, board, legalMoves.specificPieceMoves[WhiteBishop]); 

	// Rooks
	handleSpecificPieceMoves<StateT, PosHandlerT, BoardTraitsT, QueenRook>(state, board, legalMoves.specificPieceMoves[QueenRook]); 
	handleSpecificPieceMoves<StateT, PosHandlerT, BoardTraitsT, KingRook>(state, board, legalMoves.specificPieceMoves[KingRook]); 

	// Queen
	handleSpecificPieceMoves<StateT, PosHandlerT, BoardTraitsT, SpecificQueen>(state, board, legalMoves.specificPieceMoves[SpecificQueen]); 

	// TODO other promo pieces
	if(MyColorTraitsT::HasPromos) {
	  if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
	    handleSpecificPieceMoves<StateT, PosHandlerT, BoardTraitsT, PromoQueen>(state, board, legalMoves.specificPieceMoves[PromoQueen]);  
	  }
	}

	// Castling
	CastlingRightsT canCastleFlags = legalMoves.canCastleFlags;
	if(canCastleFlags) {

	  if((canCastleFlags & CanCastleQueenside)) {
	    handleCastlingMove<StateT, PosHandlerT, BoardTraitsT, CanCastleQueenside>(state, board);
	  }

	  if((canCastleFlags & CanCastleKingside)) {
	    handleCastlingMove<StateT, PosHandlerT, BoardTraitsT, CanCastleKingside>(state, board);
	  }	
	}

      } // nChecks < 2
      
      // King
      handleSpecificPieceMoves<StateT, PosHandlerT, BoardTraitsT, SpecificKing>(state, board, legalMoves.specificPieceMoves[SpecificKing]); 

    }
     
  } // namespace MakeMove
} // namespace Chess

#endif //ndef MAKE_MOVE_HPP
