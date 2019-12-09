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
    inline void handlePawnsPush(StateT state, const BoardT& board, BitBoardT pawnsPush, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      while(pawnsPush) {
	const SquareT to = Bits::popLsb(pawnsPush);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = movePawn<BoardTraitsT::Color, Push, IsPushTwo>(board, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(0.0, PushMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnsPushOne(StateT state, const BoardT& board, BitBoardT pawnsPushOne, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      handlePawnsPush<StateT, PosHandlerT, BoardTraitsT, PawnPushOneTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/false>(state, board, pawnsPushOne, directChecksBb, discoveriesBb);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnsPushTwo(StateT state, const BoardT& board, BitBoardT pawnsPushTwo, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      handlePawnsPush<StateT, PosHandlerT, BoardTraitsT, PawnPushTwoTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/true>(state, board, pawnsPushTwo, directChecksBb, discoveriesBb);
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
    inline void handlePawnsCapture(StateT state, const BoardT& board, BitBoardT pawnsCapture, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      while(pawnsCapture) {
	const SquareT to = Bits::popLsb(pawnsCapture);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = movePawn<BoardTraitsT::Color, Capture>(board, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(0.0, CaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnsCaptureLeft(StateT state, const BoardT& board, BitBoardT pawnsCaptureLeft, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      handlePawnsCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(state, board, pawnsCaptureLeft, directChecksBb, discoveriesBb);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnsCaptureRight(StateT state, const BoardT& board, BitBoardT pawnsCaptureRight, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      handlePawnsCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(state, board, pawnsCaptureRight, directChecksBb, discoveriesBb);
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn>
    inline void handlePawnEpCapture(StateT state, const BoardT& board, BitBoardT pawnsEpCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const bool isEpDiscovery) {
      // There can be only 1 en-passant capture, so no need to loop
      if(pawnsEpCaptureBb) {
	const SquareT to = Bits::lsb(pawnsEpCaptureBb);
	const SquareT from = To2FromFn::fn(to);
	const SquareT captureSq = pawnPushOneTo2From<BoardTraitsT::Color>(to);

	const BoardT newBoard = captureEp<BoardTraitsT::Color>(board, from, to, captureSq);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = isEpDiscovery || (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(0.0, EpCaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnEpCaptureLeft(StateT state, const BoardT& board, BitBoardT pawnsCaptureLeft, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const bool isEpDiscovery) {
      handlePawnEpCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(state, board, pawnsCaptureLeft, directChecksBb, discoveriesBb, isEpDiscovery);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnEpCaptureRight(StateT state, const BoardT& board, BitBoardT pawnsCaptureRight, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const bool isEpDiscovery) {
      handlePawnEpCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(state, board, pawnsCaptureRight, directChecksBb, discoveriesBb, isEpDiscovery);
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnMoves(StateT state, const BoardT& board, const PawnPushesAndCapturesT& pawnMoves, const BitBoardT directChecksBb, const BitBoardT pushDiscoveriesBb, const BitBoardT leftDiscoveriesBb, const BitBoardT rightDiscoveriesBb, const bool isEpDiscovery) {

      // Pawn pushes
      handlePawnsPushOne<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.pushesOneBb, directChecksBb, pushDiscoveriesBb);
      handlePawnsPushTwo<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.pushesTwoBb, directChecksBb, pushDiscoveriesBb);
	
      // Pawn captures
      handlePawnsCaptureLeft<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.capturesLeftBb, directChecksBb, leftDiscoveriesBb);
      handlePawnsCaptureRight<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.capturesRightBb, directChecksBb, rightDiscoveriesBb);
      
      // Pawn en-passant captures
      handlePawnEpCaptureLeft<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.epCaptures.epLeftCaptureBb, directChecksBb, leftDiscoveriesBb, isEpDiscovery);
      handlePawnEpCaptureRight<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.epCaptures.epRightCaptureBb, directChecksBb, rightDiscoveriesBb, isEpDiscovery);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, PieceT Piece, PushOrCaptureT PushOrCapture>
    inline void handlePieceMoves(StateT state, const BoardT& board, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const BitBoardT discoveredChecksBb, const MoveTypeT moveType) {
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = movePiece<BoardTraitsT::Color, Piece, PushOrCapture>(board, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveredChecksBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(0.0, moveType, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, PieceT Piece>
    inline void handlePiecePushes(StateT state, const BoardT& board, const SquareT from, const BitBoardT pushesBb, const BitBoardT directChecksBb, const BitBoardT discoveredChecksBb) {
      handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, Piece, Push>(state, board, from, pushesBb, directChecksBb, discoveredChecksBb, PushMove);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, PieceT Piece>
    inline void handlePieceCaptures(StateT state, const BoardT& board, const SquareT from, const BitBoardT capturesBb, const BitBoardT directChecksBb, const BitBoardT discoveredChecksBb) {
      handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, Piece, Capture>(state, board, from, capturesBb, directChecksBb, discoveredChecksBb, CaptureMove);
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, PieceT Piece>
    inline void handlePieceMoves(StateT state, const BoardT& board, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveredChecksBb, const BitBoardT allYourPiecesBb) {
      const ColorStateT& myState = board.pieces[(size_t)BoardTraitsT::Color];
      const SquareT from = myState.pieceSquares[Piece];

      handlePiecePushes<StateT, PosHandlerT, BoardTraitsT, Piece>(state, board, from, movesBb & ~allYourPiecesBb, directChecksBb, discoveredChecksBb);
      
      handlePieceCaptures<StateT, PosHandlerT, BoardTraitsT, Piece>(state, board, from, movesBb & allYourPiecesBb, directChecksBb, discoveredChecksBb);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, CastlingRightsT CastlingRight>
    inline void handleCastlingMove(StateT state, const BoardT& board) {
      const ColorT Color = BoardTraitsT::Color;
      
      const BoardT newBoard1 = movePiece<Color, TheKing, Push>(board, CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo);
      const BoardT newBoard = movePiece<Color, CastlingTraitsT<Color, CastlingRight>::TheRook, Push>(newBoard1, CastlingTraitsT<Color, CastlingRight>::RookFrom, CastlingTraitsT<Color, CastlingRight>::RookTo);

      // We use the king (from and) to square by convention
      PosHandlerT::handlePos(state, newBoard, MoveInfoT(0.0, CastlingMove, CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo, /*isDirectCheck*/false, /*isDiscoveredCheck TODO*/false));
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void makeAllLegalMoves(StateT state, const BoardT& board) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;

      const ColorT OtherColor = BoardTraitsT::OtherColor;

      // Generate (legal) moves
      const LegalMovesT legalMoves = genLegalMoves<BoardTraitsT>(board);

      const ColorPieceBbsT& yourPieceBbs = legalMoves.pieceBbs.colorPieceBbs[(size_t)OtherColor];
      
      const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];

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
	handlePawnMoves<StateT, PosHandlerT, BoardTraitsT>(state, board, legalMoves.pawnMoves, legalMoves.directChecks.pawnChecksBb, legalMoves.discoveredChecks.pawnPushDiscoveryMasksBb, legalMoves.discoveredChecks.pawnLeftDiscoveryMasksBb, legalMoves.discoveredChecks.pawnRightDiscoveryMasksBb, legalMoves.discoveredChecks.isEpDiscovery);
	
	// Knights
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, QueenKnight>(state, board, legalMoves.pieceMoves[QueenKnight], legalMoves.directChecks.knightChecksBb, (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb);
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, KingKnight>(state, board, legalMoves.pieceMoves[KingKnight], legalMoves.directChecks.knightChecksBb, (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb);
	
	// Bishops
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, BlackBishop>(state, board, legalMoves.pieceMoves[BlackBishop], legalMoves.directChecks.bishopChecksBb, legalMoves.discoveredChecks.orthogDiscoveryPiecesBb, allYourPiecesBb); 
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, WhiteBishop>(state, board, legalMoves.pieceMoves[WhiteBishop], legalMoves.directChecks.bishopChecksBb, legalMoves.discoveredChecks.orthogDiscoveryPiecesBb, allYourPiecesBb); 

	// Rooks
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, QueenRook>(state, board, legalMoves.pieceMoves[QueenRook], legalMoves.directChecks.rookChecksBb, legalMoves.discoveredChecks.diagDiscoveryPiecesBb, allYourPiecesBb); 
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, KingRook>(state, board, legalMoves.pieceMoves[KingRook], legalMoves.directChecks.rookChecksBb, legalMoves.discoveredChecks.diagDiscoveryPiecesBb, allYourPiecesBb); 

	// Queen
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, TheQueen>(state, board, legalMoves.pieceMoves[TheQueen], (legalMoves.directChecks.bishopChecksBb | legalMoves.directChecks.rookChecksBb), /*discoveredChecksBb*/BbNone, allYourPiecesBb); 

	// TODO other promo pieces
	if(MyColorTraitsT::HasPromos) {
	  if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
	    handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, PromoQueen>(state, board, legalMoves.pieceMoves[PromoQueen], (legalMoves.directChecks.bishopChecksBb | legalMoves.directChecks.rookChecksBb), /*discoveredChecksBb*/BbNone, allYourPiecesBb);  
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
      handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, TheKing>(state, board, legalMoves.pieceMoves[TheKing], /*directChecksBb = */BbNone, /*discoveredChecksBb*/BbNone, allYourPiecesBb); 
    }
     
  } // namespace MakeMove
} // namespace Chess

#endif //ndef MAKE_MOVE_HPP
