#ifndef MAKE_MOVE_HPP
#define MAKE_MOVE_HPP

#include "types.hpp"
#include "board.hpp"
#include "board-utils.hpp"
#include "move-gen.hpp"

namespace Chess {

  using namespace Board;
  using namespace MoveGen;
  
  namespace MakeMove {

    //
    // Consumer of this must provide a position handler type that provides a color-reversed pos handler type - TODO can do this more simply inside PosHandlerT
    //
    // e.g.
    //
    // typedef blah MyStateT;
    //
    // template <typename BoardT, Color>
    // struct MyPosHandlerT {
    //
    //   typedef PerftPosHandlerT<BoardT, OtherColorT<Color>::value> ReverseT;
    //
    //   static void handlePos(const MyStateT state, const BoardT& board, MoveInfoT moveInfo) { ... }
    //
    // };

    //
    // Non-promo pawn moves
    //
    
    enum PawnMoveT {
      PawnPush, // Mmm, not sure how to plumb in push one and push two so unused
      PawnCapture,
      PawnPromoCapture
    };

    template <typename BoardT, ColorT Color, PawnMoveT, typename PieceMapT>
    struct PawnMoveFn {
      static BoardT fn(const BoardT& board, const PieceMapT& yourPieceMap, const SquareT from, const SquareT to);
    };
    
    struct NoPieceMapT {};
    
    template <typename BoardT, ColorT Color> struct PawnMoveFn<BoardT, Color, PawnCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return captureWithPawn<BoardT, Color>(board, yourPieceMap, from, to);
      }
    };

#ifdef USE_PROMOS
    template <ColorT Color> struct PawnMoveFn<Color, PawnPromoCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return capturePromoPieceWithPawn<BoardT, Color>(board, yourPieceMap, from, to);
      }
    };
#endif //def USE_PROMOS

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, typename To2FromFn, typename PawnMoveFn, MoveTypeT MoveType>
    inline void handlePawnsMove(StateT state, const BoardT& board, const typename PawnMoveFn::PieceMapImplT& yourPieceMap, BitBoardT pawnsMoveBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
      
      while(pawnsMoveBb) {
	const SquareT to = Bits::popLsb(pawnsMoveBb);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = PawnMoveFn::fn(board, yourPieceMap, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(MoveType, from, to, isDirectCheck, isDiscoveredCheck));
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

    template <typename BoardT, ColorT Color, PawnPromoMoveT, typename PieceMapT>
    struct PawnPromoMoveFn {
      static BoardT fn(const BoardT& board, const PieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece);
    };

    template <typename BoardT, ColorT Color> struct PawnPromoMoveFn<BoardT, Color, PawnPushToPromo, NoPieceMapT> {
      typedef NoPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const NoPieceMapT&, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
	return pushPawnToPromo<BoardT, Color>(board, from, to, promoPiece);
      }
    };

    // template <ColorT Color> struct PawnPromoMoveFn<Color, PawnCaptureToPromo, ColorPieceMapT> {
    //   typedef ColorPieceMapT PieceMapImplT;
    //   static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
    // 	return captureWithPawnToPromo<BoardT, Color>(board, yourPieceMap, from, to, promoPiece);
    //   }
    // };

    // template <ColorT Color> struct PawnPromoMoveFn<Color, PawnPromoCaptureToPromo, ColorPieceMapT> {
    //   typedef ColorPieceMapT PieceMapImplT;
    //   static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
    // 	return capturePromoPieceWithPawnToPromo<BoardT, Color>(board, yourPieceMap, from, to, promoPiece);
    //   }
    // };

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, typename To2FromFn, typename PawnPromoMoveFn, MoveTypeT MoveType>
    inline void handlePawnsMoveToPromo(StateT state, const BoardT& board, const typename PawnPromoMoveFn::PieceMapImplT& yourPieceMap, BitBoardT pawnsMoveBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;

      const SquareT yourKingSq = board.state[(size_t)OtherColorT<Color>::value].pieceSquares[TheKing];
	
      while(pawnsMoveBb) {
	const SquareT to = Bits::popLsb(pawnsMoveBb);
	const SquareT from = To2FromFn::fn(to);

	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	const BitBoardT toBb = bbForSquare(to);
	const BitBoardT orthogCheckBb = toBb & yourKingRookAttacksBb;
	const BitBoardT diagCheckBb = toBb & yourKingBishopAttacksBb;
	
	const BoardT queenPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoQueen);
	const bool isQueenCheck = (orthogCheckBb | diagCheckBb) != BbNone;
	ReversePosHandlerT::handlePos(state, queenPromoBoard, MoveInfoT(MoveType, from, to, isQueenCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT knightPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoKnight);
	const bool isKnightCheck = (KnightAttacks[yourKingSq] & toBb) != BbNone;
	ReversePosHandlerT::handlePos(state, knightPromoBoard, MoveInfoT(MoveType, from, to, isKnightCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT rookPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoRook);
	const bool isRookCheck = orthogCheckBb != BbNone;
	ReversePosHandlerT::handlePos(state, rookPromoBoard, MoveInfoT(MoveType, from, to, isRookCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT bishopPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoBishop);
	const bool isBishopCheck = diagCheckBb != BbNone;
	ReversePosHandlerT::handlePos(state, bishopPromoBoard, MoveInfoT(MoveType, from, to, isBishopCheck, isDiscoveredCheck, /*isPromo*/true));
      }
    }

    // template <typename StateT, typename PosHandlerT, ColorT Color, typename To2FromFn, typename PawnPromoMoveFn, MoveTypeT MoveType>
    // inline void handlePawnsMoveToPromo(StateT state, const SimpleBoardT& board, const typename PawnPromoMoveFn::PieceMapImplT& yourPieceMap, BitBoardT pawnsMoveBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {

    //   const SimpleBoardWithPromosT boardWithPromos = copyBoard<SimpleBoardWithPromosT, SimpleBoardT>(board);

    //   handlePawnsMoveToPromoImpl<StateT, PosHandlerT, SimpleBoardWithPromosT, Color, To2FromFn, PawnPromoMoveFn, MoveType>(state, boardWithPromos, yourPieceMap, pawnsMoveBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
    // }
    
    // template <typename StateT, typename PosHandlerT, ColorT Color, typename To2FromFn, typename PawnPromoMoveFn, MoveTypeT MoveType>
    // inline void handlePawnsMoveToPromo(StateT state, const SimpleBoardWithPromosT& board, const typename PawnPromoMoveFn::PieceMapImplT& yourPieceMap, BitBoardT pawnsMoveBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {

    //   //handlePawnsMoveToPromoImpl<StateT, PosHandlerT, SimpleBoardWithPromosT, Color, To2FromFn, PawnPromoMoveFn, MoveType>(state, board, yourPieceMap, pawnsMoveBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
    // }
    
    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, typename To2FromFn, bool IsPushTwo>
    inline void handlePawnsNonPromoPush(StateT state, const BoardT& board, BitBoardT pawnsPushBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb/*, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb*/) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
      
      while(pawnsPushBb) {
	const SquareT to = Bits::popLsb(pawnsPushBb);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = pushPawn<BoardT, Color, IsPushTwo>(board, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, ColorT Color, typename To2FromFn, bool IsPushTwo>
    inline void handlePawnsPromoPush(StateT state, const SimpleBoardWithPromosT& board, BitBoardT pawnsPushBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {
      typedef PawnPromoMoveFn<SimpleBoardWithPromosT, Color, PawnPushToPromo, NoPieceMapT> PawnPushToPromoFn;
      handlePawnsMoveToPromo<StateT, PosHandlerT, SimpleBoardWithPromosT, Color, To2FromFn, PawnPushToPromoFn, PushMove>(state, board, NoPieceMapT(), pawnsPushBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
    }

    template <typename StateT, typename PosHandlerT, ColorT Color, typename To2FromFn, bool IsPushTwo>
    inline void handlePawnsPromoPush(StateT state, const SimpleBoardT& board, BitBoardT pawnsPushBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {
      // Upgrade to SimpleBoardWithPromosT
      const SimpleBoardWithPromosT boardWithPromos = copyBoard<SimpleBoardWithPromosT, SimpleBoardT>(board);

      handlePawnsPromoPush<StateT, typename PosHandlerT::WithPromosT, Color, To2FromFn, IsPushTwo>(state, boardWithPromos, pawnsPushBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
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

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, typename To2FromFn>
    inline void handlePawnsNonPromoCapture(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {

#ifdef USE_PROMOS
      if(YourColorTraitsT::HasPromos) {
	BitBoardT promoPieceCapturesBb = pawnsCaptureBb & yourPieceMap.allPromoPiecesBb;
	pawnsCaptureBb &= ~promoPieceCapturesBb;
	
	const BitBoardT promoPieceCapturesToPromoBb = promoPieceCapturesBb & LastRankBbT<Color>::LastRankBb; //(Color == White ? Rank8 : Rank1);
	promoPieceCapturesBb &= ~promoPieceCapturesToPromoBb;

	// Promo piece capture with pawn promo
	typedef PawnPromoMoveFn<Color, PawnPromoCaptureToPromo, ColorPieceMapT> PawnPromoCaptureToPromoFn;
	handlePawnsMoveToPromo<StateT, PosHandlerT, Color, To2FromFn, PawnPromoCaptureToPromoFn, CaptureMove>(state, board, yourPieceMap, promoPieceCapturesToPromoBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
	
	// Promo piece capture (without pawn promo)
	typedef PawnMoveFn<BoardT, Color, PawnPromoCapture, ColorPieceMapT> PawnPromoCaptureFn;
	handlePawnsMove<StateT, PosHandlerT, Color, To2FromFn, PawnPromoCaptureFn, CaptureMove>(state, board, yourPieceMap, promoPieceCapturesBb, directChecksBb, discoveriesBb);
      }

      const BitBoardT pawnsCaptureToPromoBb = pawnsCaptureBb & LastRankBbT<Color>::LastRankBb; //(Color == White ? Rank8 : Rank1);
      pawnsCaptureBb &= ~pawnsCaptureToPromoBb;

      // (Non-promo-piece) capture with pawn promo
      typedef PawnPromoMoveFn<Color, PawnCaptureToPromo, ColorPieceMapT> PawnCaptureToPromoFn;
      handlePawnsMoveToPromo<StateT, PosHandlerT, Color, To2FromFn, PawnCaptureToPromoFn, CaptureMove>(state, board, yourPieceMap, pawnsCaptureToPromoBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
#endif //def USE_PROMOS
	
      // (Non-promo-piece) capture (without pawn promo)
      typedef PawnMoveFn<BoardT, Color, PawnCapture, ColorPieceMapT> PawnCaptureFn;
      handlePawnsMove<StateT, PosHandlerT, BoardT, Color, To2FromFn, PawnCaptureFn, CaptureMove>(state, board, yourPieceMap, pawnsCaptureBb, directChecksBb, discoveriesBb);
    }

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, typename To2FromFn>
    inline void handlePawnEpCapture(StateT state, const BoardT& board, BitBoardT pawnsEpCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const bool isEpDiscovery) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
      
      // There can be only 1 en-passant capture, so no need to loop
      if(pawnsEpCaptureBb) {
	const SquareT to = Bits::lsb(pawnsEpCaptureBb);
	const SquareT from = To2FromFn::fn(to);
	const SquareT captureSq = pawnPushOneTo2From<Color>(to);

	const BoardT newBoard = captureEp<BoardT, Color>(board, from, to, captureSq);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = isEpDiscovery || (bbForSquare(from) & discoveriesBb) != BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(EpCaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void handlePawnNonPromoMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const PawnPushesAndCapturesT& pawnMoves, const BitBoardT directChecksBb, const BitBoardT pushDiscoveriesBb, const BitBoardT leftDiscoveriesBb, const BitBoardT rightDiscoveriesBb, const bool isLeftEpDiscovery, const bool isRightEpDiscovery/*, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb*/) {

      // Pawn pushes one square forward
      handlePawnsNonPromoPush<StateT, PosHandlerT, BoardT, Color, PawnPushOneTo2FromFn<Color>, /*IsPushTwo =*/false>(state, board, pawnMoves.pushesOneBb & ~LastRankBbT<Color>::LastRankBb, directChecksBb, pushDiscoveriesBb/*, yourKingRookAttacksBb, yourKingBishopAttacksBb*/);
      // Pawn pushes two squares forward
      handlePawnsNonPromoPush<StateT, PosHandlerT, BoardT, Color, PawnPushTwoTo2FromFn<Color>, /*IsPushTwo =*/true>(state, board, pawnMoves.pushesTwoBb, directChecksBb, pushDiscoveriesBb/*, BbNone/unused/, BbNone/unused/*/);
	
      // Pawn captures left
      handlePawnsNonPromoCapture<StateT, PosHandlerT, BoardT, Color, PawnAttackLeftTo2FromFn<Color>>(state, board, yourPieceMap, pawnMoves.capturesLeftBb & ~LastRankBbT<Color>::LastRankBb, directChecksBb, leftDiscoveriesBb/*, yourKingRookAttacksBb, yourKingBishopAttacksBb*/);
      // Pawn captures right
      handlePawnsNonPromoCapture<StateT, PosHandlerT, BoardT, Color, PawnAttackRightTo2FromFn<Color>>(state, board, yourPieceMap, pawnMoves.capturesRightBb & ~LastRankBbT<Color>::LastRankBb, directChecksBb, rightDiscoveriesBb/*, yourKingRookAttacksBb, yourKingBishopAttacksBb*/);      

      // Pawn en-passant capture left
      handlePawnEpCapture<StateT, PosHandlerT, BoardT, Color, PawnAttackLeftTo2FromFn<Color>>(state, board, pawnMoves.epCaptures.epLeftCaptureBb, directChecksBb, leftDiscoveriesBb, isLeftEpDiscovery);
      // Pawn en-passant capture right
      handlePawnEpCapture<StateT, PosHandlerT, BoardT, Color, PawnAttackRightTo2FromFn<Color>>(state, board, pawnMoves.epCaptures.epRightCaptureBb, directChecksBb, rightDiscoveriesBb, isRightEpDiscovery);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void handlePawnPromoMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const PawnPushesAndCapturesT& pawnMoves, const BitBoardT directChecksBb, const BitBoardT pushDiscoveriesBb, const BitBoardT leftDiscoveriesBb, const BitBoardT rightDiscoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {

      // Pawn pushes one square forward
      handlePawnsPromoPush<StateT, PosHandlerT, Color, PawnPushOneTo2FromFn<Color>, /*IsPushTwo =*/false>(state, board, pawnMoves.pushesOneBb & LastRankBbT<Color>::LastRankBb, directChecksBb, pushDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
	
      // // Pawn captures left
      // handlePawnsPromoCapture<StateT, PosHandlerT, BoardT, Color, PawnAttackLeftTo2FromFn<Color>>(state, board, yourPieceMap, pawnMoves.capturesLeftBb & LastRankBbT<Color>::LastRankBb, directChecksBb, leftDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
      // // Pawn captures right
      // handlePawnsPromoCapture<StateT, PosHandlerT, BoardT, Color, PawnAttackRightTo2FromFn<Color>>(state, board, yourPieceMap, pawnMoves.capturesRightBb & LastRankBbT<Color>::LastRankBb, directChecksBb, rightDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
    }
    
    //
    // (Non-promo-)piece moves other than king moves
    //

    enum PieceMoveT {
      PiecePush,
      PieceCapture,
      PiecePromoCapture
    };

    template <typename BoardT, ColorT Color, PieceT Piece, PieceMoveT, typename PieceMapT>
    struct PieceMoveFn {
      static BoardT fn(const BoardT& board, const PieceMapT& yourPieceMap, const SquareT from, const SquareT to);
    };
    
    template <typename BoardT, ColorT Color, PieceT Piece> struct PieceMoveFn<BoardT, Color, Piece, PiecePush, NoPieceMapT> {
      typedef NoPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const NoPieceMapT&, const SquareT from, const SquareT to) {
	return pushPiece<BoardT, Color, Piece>(board, from, to);
      }
    };

    template <typename BoardT, ColorT Color, PieceT Piece> struct PieceMoveFn<BoardT, Color, Piece, PieceCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return captureWithPiece<BoardT, Color, Piece>(board, yourPieceMap, from, to);
      }
    };

#ifdef USE_PROMOS
    template <ColorT Color, PieceT Piece> struct PieceMoveFn<Color, Piece, PiecePromoCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return capturePromoPieceWithPiece<BoardT, Color, Piece>(board, yourPieceMap, from, to);
      }
    };
#endif //USE_PROMOS

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, typename PieceMoveFn, MoveTypeT MoveType>
    inline void handlePieceMoves(StateT state, const BoardT& board, const typename PieceMoveFn::PieceMapImplT& yourPieceMap, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const bool isDiscoveredCheck) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
	
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);

	const BoardT newBoard = PieceMoveFn::fn(board, yourPieceMap, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(MoveType, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, PieceT Piece>
    inline void handlePieceMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {
      typedef typename BoardT::ColorStateT ColorStateT;

      const ColorStateT& myState = board.state[(size_t)Color];
      const SquareT from = myState.pieceSquares[Piece];
      const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;

      const BitBoardT piecePushesBb = movesBb & ~allYourPiecesBb;

      // (Non-promo-)piece pushes
      typedef PieceMoveFn<BoardT, Color, Piece, PiecePush, NoPieceMapT> PiecePushFn;
      handlePieceMoves<StateT, PosHandlerT, BoardT, Color, PiecePushFn, PushMove>(state, board, NoPieceMapT(), from, piecePushesBb, directChecksBb, isDiscoveredCheck);

      BitBoardT pieceCapturesBb = movesBb & allYourPiecesBb;

#ifdef USE_PROMOS
      typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      if(YourColorTraitsT::HasPromos) {
	const BitBoardT promoPieceCapturesBb = pieceCapturesBb & yourPieceMap.allPromoPiecesBb;
	pieceCapturesBb &= ~promoPieceCapturesBb;

	// (Non-promo-)piece captures of promo pieces
	typedef PieceMoveFn<BoardT, Color, Piece, PiecePromoCapture, ColorPieceMapT> PiecePromoCaptureFn;
	handlePieceMoves<StateT, PosHandlerT, BoardT, Color, PiecePromoCaptureFn, CaptureMove>(state, board, yourPieceMap, from, promoPieceCapturesBb, directChecksBb, isDiscoveredCheck);
      }
#endif //def USE_PROMOS

      // (Non-promo-)piece captures of non-promo pieces
      typedef PieceMoveFn<BoardT, Color, Piece, PieceCapture, ColorPieceMapT> PieceCaptureFn;
      handlePieceMoves<StateT, PosHandlerT, BoardT, Color, PieceCaptureFn, CaptureMove>(state, board, yourPieceMap, from, pieceCapturesBb, directChecksBb, isDiscoveredCheck);
    }

    //
    // King moves - discovery detection is different for the king
    //

    enum KingMoveT {
      KingPush,
      KingCapture,
      KingPromoCapture
    };

    template <typename BoardT, ColorT Color, KingMoveT, typename PieceMapT>
    struct KingMoveFn {
      static BoardT fn(const BoardT& board, const PieceMapT& yourPieceMap, const SquareT from, const SquareT to);
    };
    
    template <typename BoardT, ColorT Color> struct KingMoveFn<BoardT, Color, KingPush, NoPieceMapT> {
      typedef NoPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const NoPieceMapT&, const SquareT from, const SquareT to) {
	return pushPiece<BoardT, Color, TheKing>(board, from, to);
      }
    };

    template <typename BoardT, ColorT Color> struct KingMoveFn<BoardT, Color, KingCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return captureWithPiece<BoardT, Color, TheKing>(board, yourPieceMap, from, to);
      }
    };

#ifdef USE_PROMOS
    template <ColorT Color> struct KingMoveFn<Color, KingPromoCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return capturePromoPieceWithPiece<BoardT, Color, TheKing>(board, yourPieceMap, from, to);
      }
    };
#endif //def USE_PROMOS

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, typename KingMoveFn, MoveTypeT MoveType>
    inline void handleKingMoves(StateT state, const BoardT& board, const typename KingMoveFn::PieceMapImplT& yourPieceMap, const SquareT from, BitBoardT toBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRaysBb) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
	
      const BitBoardT fromBb = bbForSquare(from);
      
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	const BitBoardT toBb = bbForSquare(to);

	const BoardT newBoard = KingMoveFn::fn(board, yourPieceMap, from, to);

	const bool isDiscoveredCheck = (fromBb & discoveriesBb) != BbNone && (toBb & yourKingRaysBb) == BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(MoveType, from, to, false/*isDirectCheck*/, isDiscoveredCheck));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void handleKingMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb, const SquareT yourKingSq) {
      typedef typename BoardT::ColorStateT ColorStateT;

      const ColorStateT& myState = board.state[(size_t)Color];
      const SquareT from = myState.pieceSquares[TheKing];
      const BitBoardT yourKingRaysBb = BishopRays[yourKingSq] | RookRays[yourKingSq];
      
      const BitBoardT kingPushesBb = movesBb & ~allYourPiecesBb;

      // King pushes
      typedef KingMoveFn<BoardT, Color, KingPush, NoPieceMapT> KingPushFn;
      handleKingMoves<StateT, PosHandlerT, BoardT, Color, KingPushFn, PushMove>(state, board, NoPieceMapT(), from, kingPushesBb, discoveriesBb, yourKingRaysBb);

      BitBoardT kingCapturesBb = movesBb & allYourPiecesBb;

#ifdef USE_PROMOS
      typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      if(YourColorTraitsT::HasPromos) {
	const BitBoardT promoPieceCapturesBb = kingCapturesBb & yourPieceMap.allPromoPiecesBb;
	kingCapturesBb &= ~promoPieceCapturesBb;

	// King captures of promo pieces
	typedef KingMoveFn<Color, KingPromoCapture, ColorPieceMapT> KingPromoCaptureFn;
	handleKingMoves<StateT, PosHandlerT, Color, KingPromoCaptureFn, CaptureMove>(state, board, yourPieceMap, from, promoPieceCapturesBb, discoveriesBb, yourKingRaysBb);
      }
#endif //def USE_PROMOS

      // King captures of non-promo pieces
      typedef KingMoveFn<BoardT, Color, KingCapture, ColorPieceMapT> KingCaptureFn;
      handleKingMoves<StateT, PosHandlerT, BoardT, Color, KingCaptureFn, CaptureMove>(state, board, yourPieceMap, from, kingCapturesBb, discoveriesBb, yourKingRaysBb);
    }
    
    //
    // Promo piece moves
    //

#ifdef USE_PROMOS
    enum PromoPieceMoveT {
      PromoPiecePush,
      PromoPieceCapture,
      PromoPiecePromoCapture
    };

    template <ColorT Color, PromoPieceMoveT, typename PieceMapT>
    struct PromoPieceMoveFn {
      static BoardT fn(const BoardT& board, const PieceMapT& yourPieceMap, const SquareT from, const SquareT to);
    };
    
    template <ColorT Color> struct PromoPieceMoveFn<Color, PromoPiecePush, NoPieceMapT> {
      typedef NoPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const NoPieceMapT&, const SquareT from, const SquareT to) {
	return pushPromoPiece<BoardT, Color>(board, promoIndex, promoPiece, to);
      }
    };

    template <ColorT Color> struct PromoPieceMoveFn<Color, PromoPieceCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return captureWithPromoPiece<BoardT, Color>(board, promoIndex, promoPiece, yourPieceMap, from, to);
      }
    };

    template <ColorT Color> struct PromoPieceMoveFn<Color, PromoPiecePromoCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return capturePromoPieceWithPromoPiece<BoardT, Color>(board, promoIndex, promoPiece, yourPieceMap, from, to);
      }
    };

    template <typename StateT, typename PosHandlerT, ColorT Color, typename PromoPieceMoveFn, MoveTypeT MoveType>
    inline void handlePromoPieceMoves(StateT state, const BoardT& board, const typename PromoPieceMoveFn::PieceMapImplT& yourPieceMap, const int promoIndex, const PromoPieceT promoPiece, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const bool isDiscoveredCheck) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
      
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = PromoPieceMoveFn::fn(board, promoIndex, promoPiece, yourPieceMap, from, to);
	
	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(MoveType, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, ColorT Color>
    inline void handlePromoPieceMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const int promoIndex, const PromoPieceT promoPiece, const SquareT from, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {
      const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;

      const BitBoardT piecePushesBb = movesBb & ~allYourPiecesBb;

      // Promo-piece pushes
      typedef PromoPieceMoveFn<Color, PromoPiecePush, NoPieceMapT> PromoPiecePushFn;
      handlePromoPieceMoves<StateT, PosHandlerT, Color, PromoPiecePushFn, PushMove>(state, board, NoPieceMapT(), promoIndex, promoPiece, from, piecePushesBb, directChecksBb, isDiscoveredCheck);

      BitBoardT pieceCapturesBb = movesBb & allYourPiecesBb;
      const BitBoardT promoPieceCapturesBb = pieceCapturesBb & yourPieceMap.allPromoPiecesBb;
      pieceCapturesBb &= ~promoPieceCapturesBb;

      // Promo-piece captures of promo pieces
      typedef PromoPieceMoveFn<Color, PromoPiecePromoCapture, ColorPieceMapT> PromoPiecePromoCaptureFn;
      handlePromoPieceMoves<StateT, PosHandlerT, Color, PromoPiecePromoCaptureFn, CaptureMove>(state, board, yourPieceMap, promoIndex, promoPiece, from, promoPieceCapturesBb, directChecksBb, isDiscoveredCheck);

      // Promo-piece captures of non-promo pieces
      typedef PromoPieceMoveFn<Color, PromoPieceCapture, ColorPieceMapT> PromoPieceCaptureFn;
      handlePromoPieceMoves<StateT, PosHandlerT, Color, PromoPieceCaptureFn, CaptureMove>(state, board, yourPieceMap, promoIndex, promoPiece, from, pieceCapturesBb, directChecksBb, isDiscoveredCheck);
    }
#endif //def USE_PROMOS

    //
    // Castling moves
    //

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, CastlingRightsT CastlingRight>
    inline void handleCastlingMove(StateT state, const BoardT& board, const bool isDiscoveredCheck) {
      const BoardT newBoard1 = pushPiece<BoardT, Color, TheKing>(board, CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo);
      const BoardT newBoard = pushPiece<BoardT, Color, CastlingTraitsT<Color, CastlingRight>::TheRook>(newBoard1, CastlingTraitsT<Color, CastlingRight>::RookFrom, CastlingTraitsT<Color, CastlingRight>::RookTo);

      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
 
      // We use the king (from and) to square by convention
      ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(CastlingMove, CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo, /*isDirectCheck*/false, isDiscoveredCheck));
    }

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void makeAllLegalMoves(StateT state, const BoardT& board) {
      typedef typename BoardT::ColorStateT ColorStateT;

      typedef typename ColorPieceBbsImplType<BoardT>::ColorPieceBbsT ColorPieceBbsT;
      typedef typename LegalMovesImplType<BoardT>::LegalMovesT LegalMovesT;
      
      const ColorT OtherColor = OtherColorT<Color>::value;

      // Generate (legal) moves
      const LegalMovesT legalMoves = genLegalMoves<BoardT, Color>(board);

      const ColorPieceBbsT& yourPieceBbs = legalMoves.pieceBbs.colorPieceBbs[(size_t)OtherColor];

      const ColorStateT& yourState = board.state[(size_t)OtherColor];
#ifdef USE_PROMOS
      const ColorPieceMapT& yourPieceMap = genColorPieceMap(yourState, yourPieceBbs.allPromoPiecesBb);
#else
      const ColorPieceMapT& yourPieceMap = genColorPieceMap(yourState, BbNone);
#endif //def USE_PROMOS
      
      const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];

      // Is this an illegal pos - note this should never happen(tm) - but we will notice quickly
      if(legalMoves.isIllegalPos) {
	static bool done = false;
	if(!done) {
	  printf("\n============================================== Invalid Pos :( ===================================\n\n");
	  BoardUtils::printBoard(board);
	  printf("\n%s\n\n", Fen::toFen(board, Color).c_str());
	  printf("\n");
	  done = true;
	}
	return;
      }

      // Double check can only be evaded by moving the king so only bother with other pieces if nChecks < 2
      if(legalMoves.nChecks < 2) {

	// Evaluate moves

	// Pawns
	handlePawnNonPromoMoves<StateT, PosHandlerT, BoardT, Color>(state, board, yourPieceMap, legalMoves.pawnMoves, legalMoves.directChecks.pawnChecksBb, legalMoves.discoveredChecks.pawnPushDiscoveryMasksBb, legalMoves.discoveredChecks.pawnLeftDiscoveryMasksBb, legalMoves.discoveredChecks.pawnRightDiscoveryMasksBb, legalMoves.discoveredChecks.isLeftEpDiscovery, legalMoves.discoveredChecks.isRightEpDiscovery/*, BbNone, BbNone*/);

	const BitBoardT allPawnPromoMovesBb = (legalMoves.pawnMoves.pushesOneBb | legalMoves.pawnMoves.capturesLeftBb | legalMoves.pawnMoves.capturesRightBb) & LastRankBbT<Color>::LastRankBb;
	if(allPawnPromoMovesBb != BbNone) {
	  typedef typename PieceBbsImplType<BoardT>::PieceBbsT PieceBbsT;
	  const PieceBbsT& pieceBbs = legalMoves.pieceBbs;
	  const ColorPieceBbsT& myPieceBbs = pieceBbs.colorPieceBbs[(size_t)Color];
	  const ColorPieceBbsT& yourPieceBbs = pieceBbs.colorPieceBbs[(size_t)OtherColor];
	  
	  const BitBoardT allMyPiecesBb = myPieceBbs.bbs[AllPieceTypes];
	  const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];
	  const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;
	  
	  const SquareT yourKingSq = yourState.pieceSquares[TheKing];
	  // These are used for promo-piece check detection
	  const BitBoardT yourKingRookAttacksBb = rookAttacks(yourKingSq, allPiecesBb);
	  const BitBoardT yourKingBishopAttacksBb = bishopAttacks(yourKingSq, allPiecesBb);

	  handlePawnPromoMoves<StateT, PosHandlerT, BoardT, Color>(state, board, yourPieceMap, legalMoves.pawnMoves, legalMoves.directChecks.pawnChecksBb, legalMoves.discoveredChecks.pawnPushDiscoveryMasksBb, legalMoves.discoveredChecks.pawnLeftDiscoveryMasksBb, legalMoves.discoveredChecks.pawnRightDiscoveryMasksBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
	}
	
	// Knights
	handlePieceMoves<StateT, PosHandlerT, BoardT, Color, Knight1>(state, board, yourPieceMap, legalMoves.pieceMoves[Knight1], legalMoves.directChecks.knightChecksBb, (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb);
	handlePieceMoves<StateT, PosHandlerT, BoardT, Color, Knight2>(state, board, yourPieceMap, legalMoves.pieceMoves[Knight2], legalMoves.directChecks.knightChecksBb, (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb);
	
	// Bishops
	handlePieceMoves<StateT, PosHandlerT, BoardT, Color, Bishop1>(state, board, yourPieceMap, legalMoves.pieceMoves[Bishop1], legalMoves.directChecks.bishopChecksBb, legalMoves.discoveredChecks.orthogDiscoveryPiecesBb, allYourPiecesBb); 
	handlePieceMoves<StateT, PosHandlerT, BoardT, Color, Bishop2>(state, board, yourPieceMap, legalMoves.pieceMoves[Bishop2], legalMoves.directChecks.bishopChecksBb, legalMoves.discoveredChecks.orthogDiscoveryPiecesBb, allYourPiecesBb); 

	// Rooks
	handlePieceMoves<StateT, PosHandlerT, BoardT, Color, Rook1>(state, board, yourPieceMap, legalMoves.pieceMoves[Rook1], legalMoves.directChecks.rookChecksBb, legalMoves.discoveredChecks.diagDiscoveryPiecesBb, allYourPiecesBb); 
	handlePieceMoves<StateT, PosHandlerT, BoardT, Color, Rook2>(state, board, yourPieceMap, legalMoves.pieceMoves[Rook2], legalMoves.directChecks.rookChecksBb, legalMoves.discoveredChecks.diagDiscoveryPiecesBb, allYourPiecesBb); 

	// Queen
	handlePieceMoves<StateT, PosHandlerT, BoardT, Color, TheQueen>(state, board, yourPieceMap, legalMoves.pieceMoves[TheQueen], (legalMoves.directChecks.bishopChecksBb | legalMoves.directChecks.rookChecksBb), /*discoveriesBb*/BbNone, allYourPiecesBb); 

	// Promo pieces
#ifdef USE_PROMOS
	typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
	const ColorStateT& myState = board.state[(size_t)Color];
	if(MyColorTraitsT::HasPromos) {
	  // Ugh the bit stuff operates on BitBoardT type
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

	    handlePromoPieceMoves<StateT, PosHandlerT, Color>(state, board, yourPieceMap, promoIndex, promoPiece, promoPieceSq, legalMoves.promoPieceMoves[promoIndex], directChecksBb, discoveriesBb, allYourPiecesBb); 
	  }
	}
#endif //def USE_PROMOS
	
	// Castling
	CastlingRightsT canCastleFlags = legalMoves.canCastleFlags;
	if(canCastleFlags) {
	  if((canCastleFlags & CanCastleKingside)) {
	    handleCastlingMove<StateT, PosHandlerT, BoardT, Color, CanCastleKingside>(state, board, legalMoves.discoveredChecks.isKingsideCastlingDiscovery);
	  }	

	  if((canCastleFlags & CanCastleQueenside)) {
	    handleCastlingMove<StateT, PosHandlerT, BoardT, Color, CanCastleQueenside>(state, board, legalMoves.discoveredChecks.isQueensideCastlingDiscovery);
	  }
	}

      } // nChecks < 2
      
      // King - discoveries from king moves are a pain in the butt because each move direction is potentially different.
      handleKingMoves<StateT, PosHandlerT, BoardT, Color>(state, board, yourPieceMap, legalMoves.pieceMoves[TheKing], (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb, yourState.pieceSquares[TheKing]); 
    }
     
  } // namespace MakeMove
} // namespace Chess

#endif //ndef MAKE_MOVE_HPP
