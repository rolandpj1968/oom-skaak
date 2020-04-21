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

    //
    // Non-promo pawn moves
    //
    
    enum PawnMoveT {
      PawnPush, // Mmm, not sure how to plumb in push one and push two so unused
      PawnCapture,
      PawnPromoCapture
    };

    template <ColorT Color, PawnMoveT, typename PieceMapT>
    struct PawnMoveFn {
      static BoardT fn(const BoardT& board, const PieceMapT& yourPieceMap, const SquareT from, const SquareT to);
    };
    
    struct NoPieceMapT {};
    
    template <ColorT Color> struct PawnMoveFn<Color, PawnCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return captureWithPawn<Color>(board, yourPieceMap, from, to);
      }
    };

    template <ColorT Color> struct PawnMoveFn<Color, PawnPromoCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return capturePromoPieceWithPawn<Color>(board, yourPieceMap, from, to);
      }
    };

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn, typename PawnMoveFn, MoveTypeT MoveType>
    inline void handlePawnsMove(StateT state, const BoardT& board, const typename PawnMoveFn::PieceMapImplT& yourPieceMap, BitBoardT pawnsMoveBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      while(pawnsMoveBb) {
	const SquareT to = Bits::popLsb(pawnsMoveBb);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = PawnMoveFn::fn(board, yourPieceMap, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(MoveType, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    //
    // pawn promotions
    //
    
    enum PawnPromoMoveT {
      PawnPushToPromo,
      PawnCaptureToPromo,
      PawnPromoCaptureToPromo
    };

    template <ColorT Color, PawnPromoMoveT, typename PieceMapT>
    struct PawnPromoMoveFn {
      static BoardT fn(const BoardT& board, const PieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece);
    };

    template <ColorT Color> struct PawnPromoMoveFn<Color, PawnPushToPromo, NoPieceMapT> {
      typedef NoPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const NoPieceMapT&, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
	return pushPawnToPromo<Color>(board, from, to, promoPiece);
      }
    };

    template <ColorT Color> struct PawnPromoMoveFn<Color, PawnCaptureToPromo, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
	return captureWithPawnToPromo<Color>(board, yourPieceMap, from, to, promoPiece);
      }
    };

    template <ColorT Color> struct PawnPromoMoveFn<Color, PawnPromoCaptureToPromo, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
	return capturePromoPieceWithPawnToPromo<Color>(board, yourPieceMap, from, to, promoPiece);
      }
    };

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn, typename PawnPromoMoveFn, MoveTypeT MoveType>
    inline void handlePawnsMoveToPromo(StateT state, const BoardT& board, const typename PawnPromoMoveFn::PieceMapImplT& yourPieceMap, BitBoardT pawnsMoveBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {

      const SquareT yourKingSq = board.pieces[(size_t)BoardTraitsT::OtherColor].pieceSquares[TheKing];
	
      while(pawnsMoveBb) {
	const SquareT to = Bits::popLsb(pawnsMoveBb);
	const SquareT from = To2FromFn::fn(to);

	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	const BitBoardT toBb = bbForSquare(to);
	const BitBoardT orthogCheckBb = toBb & yourKingRookAttacksBb;
	const BitBoardT diagCheckBb = toBb & yourKingBishopAttacksBb;
	
	const BoardT queenPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoQueen);
	const bool isQueenCheck = (orthogCheckBb | diagCheckBb) != BbNone;
	PosHandlerT::handlePos(state, queenPromoBoard, MoveInfoT(MoveType, from, to, isQueenCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT knightPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoKnight);
	const bool isKnightCheck = (KnightAttacks[yourKingSq] & toBb) != BbNone;
	PosHandlerT::handlePos(state, knightPromoBoard, MoveInfoT(MoveType, from, to, isKnightCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT rookPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoRook);
	const bool isRookCheck = orthogCheckBb != BbNone;
	PosHandlerT::handlePos(state, rookPromoBoard, MoveInfoT(MoveType, from, to, isRookCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT bishopPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoBishop);
	const bool isBishopCheck = diagCheckBb != BbNone;
	PosHandlerT::handlePos(state, bishopPromoBoard, MoveInfoT(MoveType, from, to, isBishopCheck, isDiscoveredCheck, /*isPromo*/true));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn, bool IsPushTwo>
    inline void handlePawnsPush(StateT state, const BoardT& board, BitBoardT pawnsPushBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {

      // Can't reach promotion on the first move
      if(!IsPushTwo) {
	const BitBoardT pawnsPushToPromoBb = pawnsPushBb & (BoardTraitsT::Color == White ? Rank8 : Rank1);
	typedef PawnPromoMoveFn<BoardTraitsT::Color, PawnPushToPromo, NoPieceMapT> PawnPushToPromoFn;
	handlePawnsMoveToPromo<StateT, PosHandlerT, BoardTraitsT, To2FromFn, PawnPushToPromoFn, PushMove>(state, board, NoPieceMapT(), pawnsPushToPromoBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
	pawnsPushBb &= ~pawnsPushToPromoBb;
      }
      
      while(pawnsPushBb) {
	const SquareT to = Bits::popLsb(pawnsPushBb);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = pushPawn<BoardTraitsT::Color, IsPushTwo>(board, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
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

    // template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn>
    // inline void handlePawnsCaptureOfPromoPiece(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {

    //   const BitBoardT pawnsCaptureToPromoBb = pawnsCaptureBb & (BoardTraitsT::Color == White ? Rank8 : Rank1);
    //   pawnsCaptureBb &= ~pawnsCaptureToPromoBb;

    //   typedef PawnPromoMoveFn<BoardTraitsT::Color, PawnPromoCaptureToPromo, ColorPieceMapT> PawnPromoCaptureToPromoFn;
    //   handlePawnsMoveToPromo<StateT, PosHandlerT, BoardTraitsT, To2FromFn, PawnPromoCaptureToPromoFn, CaptureMove>(state, board, yourPieceMap, pawnsCaptureToPromoBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
	
    //   typedef PawnMoveFn<BoardTraitsT::Color, PawnPromoCapture, ColorPieceMapT> PawnPromoCaptureFn;
    //   handlePawnsMove<StateT, PosHandlerT, BoardTraitsT, To2FromFn, PawnPromoCaptureFn, CaptureMove>(state, board, yourPieceMap, pawnsCaptureBb, directChecksBb, discoveriesBb);
    // }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn>
    inline void handlePawnsCapture(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {
      
      BitBoardT promoPieceCapturesBb = pawnsCaptureBb & yourPieceMap.allPromoPiecesBb;
      pawnsCaptureBb &= ~promoPieceCapturesBb;

      const BitBoardT promoPieceCapturesToPromoBb = promoPieceCapturesBb & (BoardTraitsT::Color == White ? Rank8 : Rank1);
      promoPieceCapturesBb &= ~promoPieceCapturesToPromoBb;

      // Promo piece capture with pawn promo
      typedef PawnPromoMoveFn<BoardTraitsT::Color, PawnPromoCaptureToPromo, ColorPieceMapT> PawnPromoCaptureToPromoFn;
      handlePawnsMoveToPromo<StateT, PosHandlerT, BoardTraitsT, To2FromFn, PawnPromoCaptureToPromoFn, CaptureMove>(state, board, yourPieceMap, promoPieceCapturesToPromoBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
	
      // Promo piece capture (without pawn promo)
      typedef PawnMoveFn<BoardTraitsT::Color, PawnPromoCapture, ColorPieceMapT> PawnPromoCaptureFn;
      handlePawnsMove<StateT, PosHandlerT, BoardTraitsT, To2FromFn, PawnPromoCaptureFn, CaptureMove>(state, board, yourPieceMap, promoPieceCapturesBb, directChecksBb, discoveriesBb);
      // handlePawnsCaptureOfPromoPiece<StateT, PosHandlerT, BoardTraitsT, To2FromFn>(state, board, yourPieceMap, promoPieceCapturesBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);

      const BitBoardT pawnsCaptureToPromoBb = pawnsCaptureBb & (BoardTraitsT::Color == White ? Rank8 : Rank1);
      pawnsCaptureBb &= ~pawnsCaptureToPromoBb;

      // (Non-promo-piece) capture with pawn promo
      typedef PawnPromoMoveFn<BoardTraitsT::Color, PawnCaptureToPromo, ColorPieceMapT> PawnCaptureToPromoFn;
      handlePawnsMoveToPromo<StateT, PosHandlerT, BoardTraitsT, To2FromFn, PawnCaptureToPromoFn, CaptureMove>(state, board, yourPieceMap, pawnsCaptureToPromoBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
	
      // (Non-promo-piece) capture (without pawn promo)
      typedef PawnMoveFn<BoardTraitsT::Color, PawnCapture, ColorPieceMapT> PawnCaptureFn;
      handlePawnsMove<StateT, PosHandlerT, BoardTraitsT, To2FromFn, PawnCaptureFn, CaptureMove>(state, board, yourPieceMap, pawnsCaptureBb, directChecksBb, discoveriesBb);
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
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(EpCaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const PawnPushesAndCapturesT& pawnMoves, const BitBoardT directChecksBb, const BitBoardT pushDiscoveriesBb, const BitBoardT leftDiscoveriesBb, const BitBoardT rightDiscoveriesBb, const bool isLeftEpDiscovery, const bool isRightEpDiscovery, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {

      // Pawn pushes - one and two squares
      handlePawnsPush<StateT, PosHandlerT, BoardTraitsT, PawnPushOneTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/false>(state, board, pawnMoves.pushesOneBb, directChecksBb, pushDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
      handlePawnsPush<StateT, PosHandlerT, BoardTraitsT, PawnPushTwoTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/true>(state, board, pawnMoves.pushesTwoBb, directChecksBb, pushDiscoveriesBb, BbNone/*unused*/, BbNone/*unused*/);
	
      // Pawn captures - left and right
      handlePawnsCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(state, board, yourPieceMap, pawnMoves.capturesLeftBb, directChecksBb, leftDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
      handlePawnsCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(state, board, yourPieceMap, pawnMoves.capturesRightBb, directChecksBb, rightDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);      

      // Pawn en-passant captures - left and right
      handlePawnEpCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(state, board, pawnMoves.epCaptures.epLeftCaptureBb, directChecksBb, leftDiscoveriesBb, isLeftEpDiscovery);
      handlePawnEpCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(state, board, pawnMoves.epCaptures.epRightCaptureBb, directChecksBb, rightDiscoveriesBb, isRightEpDiscovery);
    }
    
    //
    // (Non-promo-)piece moves
    //

    enum PieceMoveT {
      PiecePush,
      PieceCapture,
      PiecePromoCapture
    };

    template <ColorT Color, PieceT Piece, PieceMoveT, typename PieceMapT>
    struct PieceMoveFn {
      static BoardT fn(const BoardT& board, const PieceMapT& yourPieceMap, const SquareT from, const SquareT to);
    };
    
    template <ColorT Color, PieceT Piece> struct PieceMoveFn<Color, Piece, PiecePush, NoPieceMapT> {
      typedef NoPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const NoPieceMapT&, const SquareT from, const SquareT to) {
	return pushPiece<Color, Piece>(board, from, to);
      }
    };

    template <ColorT Color, PieceT Piece> struct PieceMoveFn<Color, Piece, PieceCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return captureWithPiece<Color, Piece>(board, yourPieceMap, from, to);
      }
    };

    template <ColorT Color, PieceT Piece> struct PieceMoveFn<Color, Piece, PiecePromoCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return capturePromoPieceWithPiece<Color, Piece>(board, yourPieceMap, from, to);
      }
    };

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename PieceMoveFn, MoveTypeT MoveType>
    inline void handlePieceMoves(StateT state, const BoardT& board, const typename PieceMoveFn::PieceMapImplT& yourPieceMap, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const bool isDiscoveredCheck) {
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);

	const BoardT newBoard = PieceMoveFn::fn(board, yourPieceMap, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(MoveType, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, PieceT Piece>
    inline void handlePiecePushes(StateT state, const BoardT& board, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const bool isDiscoveredCheck) {
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = pushPiece<BoardTraitsT::Color, Piece>(board, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, PieceT Piece>
    inline void handlePieceCaptures(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, BitBoardT pieceCapturesBb, const BitBoardT directChecksBb, const bool isDiscoveredCheck) {

      const BitBoardT promoPieceCapturesBb = pieceCapturesBb & yourPieceMap.allPromoPiecesBb;
      typedef PieceMoveFn<BoardTraitsT::Color, Piece, PiecePromoCapture, ColorPieceMapT> PiecePromoCaptureFn;
      handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, PiecePromoCaptureFn, CaptureMove>(state, board, yourPieceMap, from, promoPieceCapturesBb, directChecksBb, isDiscoveredCheck);
      pieceCapturesBb &= ~promoPieceCapturesBb;

      typedef PieceMoveFn<BoardTraitsT::Color, Piece, PieceCapture, ColorPieceMapT> PieceCaptureFn;
      handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, PieceCaptureFn, CaptureMove>(state, board, yourPieceMap, from, pieceCapturesBb, directChecksBb, isDiscoveredCheck);
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, PieceT Piece>
    inline void handlePieceMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {
      const ColorStateT& myState = board.pieces[(size_t)BoardTraitsT::Color];
      const SquareT from = myState.pieceSquares[Piece];
      const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;

      handlePiecePushes<StateT, PosHandlerT, BoardTraitsT, Piece>(state, board, from, movesBb & ~allYourPiecesBb, directChecksBb, isDiscoveredCheck);
      
      handlePieceCaptures<StateT, PosHandlerT, BoardTraitsT, Piece>(state, board, yourPieceMap, from, movesBb & allYourPiecesBb, directChecksBb, isDiscoveredCheck);
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePromoPiecePushes(StateT state, const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const bool isDiscoveredCheck) {
      
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = pushPromoPiece<BoardTraitsT::Color>(board, promoIndex, promoPiece, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePromoPieceCapturesOfPromoPieces(StateT state, const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const ColorPieceMapT& yourPieceMap, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const bool isDiscoveredCheck) {

      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = capturePromoPieceWithPromoPiece<BoardTraitsT::Color>(board, promoIndex, promoPiece, yourPieceMap, from, to);
	
	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(CaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePromoPieceCaptures(StateT state, const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const ColorPieceMapT& yourPieceMap, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const bool isDiscoveredCheck) {

      const BitBoardT promoPieceCaptureBb = toBb & yourPieceMap.allPromoPiecesBb;
      handlePromoPieceCapturesOfPromoPieces<StateT, PosHandlerT, BoardTraitsT>(state, board, promoIndex, promoPiece, yourPieceMap, from, promoPieceCaptureBb, directChecksBb, isDiscoveredCheck);
      toBb &= ~promoPieceCaptureBb;
      
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = captureWithPromoPiece<BoardTraitsT::Color>(board, promoIndex, promoPiece, yourPieceMap, from, to);
	
	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(CaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePromoPieceMoves(StateT state, const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const SquareT from, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {
      const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;

      handlePromoPiecePushes<StateT, PosHandlerT, BoardTraitsT>(state, board, promoIndex, promoPiece, from, movesBb & ~allYourPiecesBb, directChecksBb, isDiscoveredCheck);
      
      handlePromoPieceCaptures<StateT, PosHandlerT, BoardTraitsT>(state, board, promoIndex, promoPiece, yourPieceMap, from, movesBb & allYourPiecesBb, directChecksBb, isDiscoveredCheck);
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handleKingPushes(StateT state, const BoardT& board, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRaysBb) {
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = pushPiece<BoardTraitsT::Color, TheKing>(board, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone && (bbForSquare(to) & yourKingRaysBb) == BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handleKingCaptures(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRaysBb) {
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = captureWithPiece<BoardTraitsT::Color, TheKing>(board, yourPieceMap, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone && (bbForSquare(to) & yourKingRaysBb) == BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(CaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    // We specialise king moves because discovery handling is different - we could probably factor tho - TODO
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handleKingMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb, const SquareT yourKingSq) {
      const ColorStateT& myState = board.pieces[(size_t)BoardTraitsT::Color];
      const SquareT from = myState.pieceSquares[TheKing];
      const BitBoardT yourKingRaysBb = BishopRays[yourKingSq] | RookRays[yourKingSq];

      handleKingPushes<StateT, PosHandlerT, BoardTraitsT>(state, board, from, movesBb & ~allYourPiecesBb, directChecksBb, discoveriesBb, yourKingRaysBb);
      
      handleKingCaptures<StateT, PosHandlerT, BoardTraitsT>(state, board, yourPieceMap, from, movesBb & allYourPiecesBb, directChecksBb, discoveriesBb, yourKingRaysBb);
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, CastlingRightsT CastlingRight>
    inline void handleCastlingMove(StateT state, const BoardT& board, const bool isDiscoveredCheck) {
      const ColorT Color = BoardTraitsT::Color;
      
      const BoardT newBoard1 = pushPiece<Color, TheKing>(board, CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo);
      const BoardT newBoard = pushPiece<Color, CastlingTraitsT<Color, CastlingRight>::TheRook>(newBoard1, CastlingTraitsT<Color, CastlingRight>::RookFrom, CastlingTraitsT<Color, CastlingRight>::RookTo);

      // We use the king (from and) to square by convention
      PosHandlerT::handlePos(state, newBoard, MoveInfoT(CastlingMove, CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo, /*isDirectCheck*/false, isDiscoveredCheck));
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void makeAllLegalMoves(StateT state, const BoardT& board) {
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;

      // Generate (legal) moves
      const LegalMovesT legalMoves = genLegalMoves<BoardTraitsT>(board);

      const ColorPieceBbsT& yourPieceBbs = legalMoves.pieceBbs.colorPieceBbs[(size_t)OtherColor];

      const ColorStateT& myState = board.pieces[(size_t)Color];
      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
      const ColorPieceMapT& yourPieceMap = genColorPieceMap(yourState, yourPieceBbs.allPromoPiecesBb);
      
      const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];

      // Is this an illegal pos - note this should never happen(tm) - but we will notice quickly
      if(legalMoves.isIllegalPos) {
	static bool done = false;
	if(!done) {
	  printf("\n============================================== Invalid Pos :( ===================================\n\n");
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
	handlePawnMoves<StateT, PosHandlerT, BoardTraitsT>(state, board, yourPieceMap, legalMoves.pawnMoves, legalMoves.directChecks.pawnChecksBb, legalMoves.discoveredChecks.pawnPushDiscoveryMasksBb, legalMoves.discoveredChecks.pawnLeftDiscoveryMasksBb, legalMoves.discoveredChecks.pawnRightDiscoveryMasksBb, legalMoves.discoveredChecks.isLeftEpDiscovery, legalMoves.discoveredChecks.isRightEpDiscovery, legalMoves.yourKingRookAttacksBb, legalMoves.yourKingBishopAttacksBb);
	
	// Knights
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, Knight1>(state, board, yourPieceMap, legalMoves.pieceMoves[Knight1], legalMoves.directChecks.knightChecksBb, (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb);
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, Knight2>(state, board, yourPieceMap, legalMoves.pieceMoves[Knight2], legalMoves.directChecks.knightChecksBb, (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb);
	
	// Bishops
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, Bishop1>(state, board, yourPieceMap, legalMoves.pieceMoves[Bishop1], legalMoves.directChecks.bishopChecksBb, legalMoves.discoveredChecks.orthogDiscoveryPiecesBb, allYourPiecesBb); 
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, Bishop2>(state, board, yourPieceMap, legalMoves.pieceMoves[Bishop2], legalMoves.directChecks.bishopChecksBb, legalMoves.discoveredChecks.orthogDiscoveryPiecesBb, allYourPiecesBb); 

	// Rooks
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, Rook1>(state, board, yourPieceMap, legalMoves.pieceMoves[Rook1], legalMoves.directChecks.rookChecksBb, legalMoves.discoveredChecks.diagDiscoveryPiecesBb, allYourPiecesBb); 
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, Rook2>(state, board, yourPieceMap, legalMoves.pieceMoves[Rook2], legalMoves.directChecks.rookChecksBb, legalMoves.discoveredChecks.diagDiscoveryPiecesBb, allYourPiecesBb); 

	// Queen
	handlePieceMoves<StateT, PosHandlerT, BoardTraitsT, TheQueen>(state, board, yourPieceMap, legalMoves.pieceMoves[TheQueen], (legalMoves.directChecks.bishopChecksBb | legalMoves.directChecks.rookChecksBb), /*discoveriesBb*/BbNone, allYourPiecesBb); 

	// Promo pieces - ugh the bit stuff operates on BitBoardT type
	BitBoardT activePromos = (BitBoardT)myState.activePromos;
	while(activePromos) {
	  const int promoIndex = Bits::popLsb(activePromos);
	  const PromoPieceAndSquareT promoPieceAndSquare = myState.promos[promoIndex];
	  const PromoPieceT promoPiece = promoPieceOf(promoPieceAndSquare);
	  const SquareT promoPieceSq = squareOf(promoPieceAndSquare);
	  
	  BitBoardT directChecksBb = BbNone;
	  BitBoardT discoveriesBb = BbNone;
	  
	  // Done as a multi-if rather than switch cos in real games it's almost always going to be a Queen
	  if(promoPiece == PromoQueen) {
	    directChecksBb = (legalMoves.directChecks.bishopChecksBb | legalMoves.directChecks.rookChecksBb);
	    discoveriesBb = BbNone;
	  } else if(promoPiece == PromoKnight) {
	    directChecksBb = legalMoves.directChecks.knightChecksBb;
	    discoveriesBb = (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb);
	  } else if(promoPiece == PromoRook) {
	    directChecksBb = legalMoves.directChecks.rookChecksBb;
	    discoveriesBb = legalMoves.discoveredChecks.diagDiscoveryPiecesBb;
	  } else if(promoPiece == PromoBishop) {
	    directChecksBb = legalMoves.directChecks.bishopChecksBb;
	    discoveriesBb = legalMoves.discoveredChecks.orthogDiscoveryPiecesBb;
	  }

	  handlePromoPieceMoves<StateT, PosHandlerT, BoardTraitsT>(state, board, promoIndex, promoPiece, promoPieceSq, yourPieceMap, legalMoves.promoPieceMoves[promoIndex], directChecksBb, discoveriesBb, allYourPiecesBb); 
	}
	
	// Castling
	CastlingRightsT canCastleFlags = legalMoves.canCastleFlags;
	if(canCastleFlags) {
	  if((canCastleFlags & CanCastleKingside)) {
	    handleCastlingMove<StateT, PosHandlerT, BoardTraitsT, CanCastleKingside>(state, board, legalMoves.discoveredChecks.isKingsideCastlingDiscovery);
	  }	

	  if((canCastleFlags & CanCastleQueenside)) {
	    handleCastlingMove<StateT, PosHandlerT, BoardTraitsT, CanCastleQueenside>(state, board, legalMoves.discoveredChecks.isQueensideCastlingDiscovery);
	  }
	}

      } // nChecks < 2
      
      // King - discoveries from king moves are a pain in the butt because each move direction is potentially different.
      handleKingMoves<StateT, PosHandlerT, BoardTraitsT>(state, board, yourPieceMap, legalMoves.pieceMoves[TheKing], /*directChecksBb = */BbNone, (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb, yourState.pieceSquares[TheKing]); 
    }
     
  } // namespace MakeMove
} // namespace Chess

#endif //ndef MAKE_MOVE_HPP
