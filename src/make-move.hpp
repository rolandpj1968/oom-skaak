#ifndef MAKE_MOVE_HPP
#define MAKE_MOVE_HPP

#include "types.hpp"
#include "board.hpp"
#include "move-gen.hpp"

namespace Chess {

  using namespace Board;
  using namespace MoveGen;
  
  namespace MakeMove {

    //
    // Consumer of this must provide a position handler type that is templatised on BoardTraitsT and provides a static function handlePos(...).
    // The handler type also needs to echo BoardTraitsT::ReverseT and other BoardTraitsT typedefs - I can't find a neater way of doing this :(
    //
    // e.g.
    //
    // typedef blah MyStateT;
    //
    // template <typename BoardTraitsT>
    // struct MyPosHandlerT {
    //
    //   typedef PerftPosHandlerT<typename BoardTraitsT::ReverseT> ReverseT;
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

    template <typename StateT, typename PosHandlerT, typename BoardT, typename BoardTraitsT, typename To2FromFn, typename PawnMoveFn, MoveFlagsT CaptureFlag/*MoveTypeT MoveType*/>
    inline void handlePawnsMove(StateT state, const BoardT& board, const typename PawnMoveFn::PieceMapImplT& yourPieceMap, BitBoardT pawnsMoveBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
      
      while(pawnsMoveBb) {
	const SquareT to = Bits::popLsb(pawnsMoveBb);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = PawnMoveFn::fn(board, yourPieceMap, from, to);

	const MoveFlagsT directCheckFlag = ((bbForSquare(to) & directChecksBb) == BbNone) ? NoMoveFlags : DirectCheckMoveFlag;
	const MoveFlagsT discoveredCheckFlag = ((bbForSquare(from) & discoveriesBb) == BbNone) ? NoMoveFlags : DiscoveredCheckMoveFlag;
	// const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	// const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT((MoveFlagsT)(CaptureFlag | directCheckFlag | discoveredCheckFlag), from, to/*, isDirectCheck, isDiscoveredCheck*/));
      }
    }

    //
    // pawn promotions
    //

#ifdef USE_PROMOS
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
	return pushPawnToPromo<BoardT, Color>(board, from, to, promoPiece);
      }
    };

    template <ColorT Color> struct PawnPromoMoveFn<Color, PawnCaptureToPromo, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
	return captureWithPawnToPromo<BoardT, Color>(board, yourPieceMap, from, to, promoPiece);
      }
    };

    template <ColorT Color> struct PawnPromoMoveFn<Color, PawnPromoCaptureToPromo, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
	return capturePromoPieceWithPawnToPromo<BoardT, Color>(board, yourPieceMap, from, to, promoPiece);
      }
    };

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn, typename PawnPromoMoveFn, MoveTypeT MoveType>
    inline void handlePawnsMoveToPromo(StateT state, const BoardT& board, const typename PawnPromoMoveFn::PieceMapImplT& yourPieceMap, BitBoardT pawnsMoveBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {
      // The order of PosHandlerT::WithPromosT::ReverseT is important - it's my color that has promos
      typedef typename PosHandlerT::WithPromosT::ReverseT ReversePosHandlerWithPromosT;

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
	ReversePosHandlerWithPromosT::handlePos(state, queenPromoBoard, MoveInfoT(MoveType, from, to, isQueenCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT knightPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoKnight);
	const bool isKnightCheck = (KnightAttacks[yourKingSq] & toBb) != BbNone;
	ReversePosHandlerWithPromosT::handlePos(state, knightPromoBoard, MoveInfoT(MoveType, from, to, isKnightCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT rookPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoRook);
	const bool isRookCheck = orthogCheckBb != BbNone;
	ReversePosHandlerWithPromosT::handlePos(state, rookPromoBoard, MoveInfoT(MoveType, from, to, isRookCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT bishopPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoBishop);
	const bool isBishopCheck = diagCheckBb != BbNone;
	ReversePosHandlerWithPromosT::handlePos(state, bishopPromoBoard, MoveInfoT(MoveType, from, to, isBishopCheck, isDiscoveredCheck, /*isPromo*/true));
      }
    }

#endif //def USE_PROMOS

    template <typename StateT, typename PosHandlerT, typename BoardT, typename BoardTraitsT, typename To2FromFn, bool IsPushTwo>
    inline void handlePawnsPush(StateT state, const BoardT& board, BitBoardT pawnsPushBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;

      // Can't reach promotion on the first move
#ifdef USE_PROMOS
      if(!IsPushTwo) {
	const BitBoardT pawnsPushToPromoBb = pawnsPushBb & (BoardTraitsT::Color == White ? Rank8 : Rank1);
	typedef PawnPromoMoveFn<BoardTraitsT::Color, PawnPushToPromo, NoPieceMapT> PawnPushToPromoFn;
	handlePawnsMoveToPromo<StateT, PosHandlerT, BoardTraitsT, To2FromFn, PawnPushToPromoFn, PushMove>(state, board, NoPieceMapT(), pawnsPushToPromoBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
	pawnsPushBb &= ~pawnsPushToPromoBb;
      }
#endif //def USE_PROMOS
      
      while(pawnsPushBb) {
	const SquareT to = Bits::popLsb(pawnsPushBb);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = pushPawn<BoardT, BoardTraitsT::Color, IsPushTwo>(board, from, to);

	const MoveFlagsT directCheckFlag = ((bbForSquare(to) & directChecksBb) == BbNone) ? NoMoveFlags : DirectCheckMoveFlag;
	const MoveFlagsT discoveredCheckFlag = ((bbForSquare(from) & discoveriesBb) == BbNone) ? NoMoveFlags : DiscoveredCheckMoveFlag;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT((MoveFlagsT)(directCheckFlag | discoveredCheckFlag), from, to/*, isDirectCheck, isDiscoveredCheck*/));
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

    template <typename StateT, typename PosHandlerT, typename BoardT, typename BoardTraitsT, typename To2FromFn>
    inline void handlePawnsCapture(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {

#ifdef USE_PROMOS
      typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      if(YourColorTraitsT::HasPromos) {
	BitBoardT promoPieceCapturesBb = pawnsCaptureBb & yourPieceMap.allPromoPiecesBb;
	pawnsCaptureBb &= ~promoPieceCapturesBb;
	
	const BitBoardT promoPieceCapturesToPromoBb = promoPieceCapturesBb & (BoardTraitsT::Color == White ? Rank8 : Rank1);
	promoPieceCapturesBb &= ~promoPieceCapturesToPromoBb;

	// Promo piece capture with pawn promo
	typedef PawnPromoMoveFn<BoardTraitsT::Color, PawnPromoCaptureToPromo, ColorPieceMapT> PawnPromoCaptureToPromoFn;
	handlePawnsMoveToPromo<StateT, PosHandlerT, BoardTraitsT, To2FromFn, PawnPromoCaptureToPromoFn, CaptureMove>(state, board, yourPieceMap, promoPieceCapturesToPromoBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
	
	// Promo piece capture (without pawn promo)
	typedef PawnMoveFn<BoardT, BoardTraitsT::Color, PawnPromoCapture, ColorPieceMapT> PawnPromoCaptureFn;
	handlePawnsMove<StateT, PosHandlerT, BoardTraitsT, To2FromFn, PawnPromoCaptureFn, CaptureMove>(state, board, yourPieceMap, promoPieceCapturesBb, directChecksBb, discoveriesBb);
      }

      const BitBoardT pawnsCaptureToPromoBb = pawnsCaptureBb & (BoardTraitsT::Color == White ? Rank8 : Rank1);
      pawnsCaptureBb &= ~pawnsCaptureToPromoBb;

      // (Non-promo-piece) capture with pawn promo
      typedef PawnPromoMoveFn<BoardTraitsT::Color, PawnCaptureToPromo, ColorPieceMapT> PawnCaptureToPromoFn;
      handlePawnsMoveToPromo<StateT, PosHandlerT, BoardTraitsT, To2FromFn, PawnCaptureToPromoFn, CaptureMove>(state, board, yourPieceMap, pawnsCaptureToPromoBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
#endif //def USE_PROMOS
	
      // (Non-promo-piece) capture (without pawn promo)
      typedef PawnMoveFn<BoardT, BoardTraitsT::Color, PawnCapture, ColorPieceMapT> PawnCaptureFn;
      handlePawnsMove<StateT, PosHandlerT, BoardT, BoardTraitsT, To2FromFn, PawnCaptureFn, CaptureMoveFlag>(state, board, yourPieceMap, pawnsCaptureBb, directChecksBb, discoveriesBb);
    }

    template <typename StateT, typename PosHandlerT, typename BoardT, typename BoardTraitsT, typename To2FromFn>
    inline void handlePawnEpCapture(StateT state, const BoardT& board, BitBoardT pawnsEpCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const bool isEpDiscovery) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
      
      // There can be only 1 en-passant capture, so no need to loop
      if(pawnsEpCaptureBb) {
	const SquareT to = Bits::lsb(pawnsEpCaptureBb);
	const SquareT from = To2FromFn::fn(to);
	const SquareT captureSq = pawnPushOneTo2From<BoardTraitsT::Color>(to);

	const BoardT newBoard = captureEp<BoardT, BoardTraitsT::Color>(board, from, to, captureSq);

	const MoveFlagsT directCheckFlag = ((bbForSquare(to) & directChecksBb) == BbNone) ? NoMoveFlags : DirectCheckMoveFlag;
	//const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const MoveFlagsT discoveredCheckFlag = (!isEpDiscovery && (bbForSquare(from) & discoveriesBb) == BbNone) ? NoMoveFlags : DiscoveredCheckMoveFlag;
	//const bool isDiscoveredCheck = isEpDiscovery || (bbForSquare(from) & discoveriesBb) != BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT((MoveFlagsT)(CaptureMoveFlag | EpCaptureMoveFlag | directCheckFlag | discoveredCheckFlag), from, to/*, isDirectCheck, isDiscoveredCheck*/));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardT, typename BoardTraitsT>
    inline void handlePawnMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const PawnPushesAndCapturesT& pawnMoves, const BitBoardT directChecksBb, const BitBoardT pushDiscoveriesBb, const BitBoardT leftDiscoveriesBb, const BitBoardT rightDiscoveriesBb, const bool isLeftEpDiscovery, const bool isRightEpDiscovery, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {

      // Pawn pushes one square forward
      handlePawnsPush<StateT, PosHandlerT, BoardT, BoardTraitsT, PawnPushOneTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/false>(state, board, pawnMoves.pushesOneBb, directChecksBb, pushDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
      // Pawn pushes two squares forward
      handlePawnsPush<StateT, PosHandlerT, BoardT, BoardTraitsT, PawnPushTwoTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/true>(state, board, pawnMoves.pushesTwoBb, directChecksBb, pushDiscoveriesBb, BbNone/*unused*/, BbNone/*unused*/);
	
      // Pawn captures left
      handlePawnsCapture<StateT, PosHandlerT, BoardT, BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(state, board, yourPieceMap, pawnMoves.capturesLeftBb, directChecksBb, leftDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
      // Pawn captures right
      handlePawnsCapture<StateT, PosHandlerT, BoardT, BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(state, board, yourPieceMap, pawnMoves.capturesRightBb, directChecksBb, rightDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);      

      // Pawn en-passant capture left
      handlePawnEpCapture<StateT, PosHandlerT, BoardT, BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(state, board, pawnMoves.epCaptures.epLeftCaptureBb, directChecksBb, leftDiscoveriesBb, isLeftEpDiscovery);
      // Pawn en-passant capture right
      handlePawnEpCapture<StateT, PosHandlerT, BoardT, BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(state, board, pawnMoves.epCaptures.epRightCaptureBb, directChecksBb, rightDiscoveriesBb, isRightEpDiscovery);
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

    template <typename StateT, typename PosHandlerT, typename BoardT, typename BoardTraitsT, typename PieceMoveFn, MoveFlagsT CaptureFlag/*MoveTypeT MoveType*/>
    inline void handlePieceMoves(StateT state, const BoardT& board, const typename PieceMoveFn::PieceMapImplT& yourPieceMap, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const MoveFlagsT discoveredCheckFlag) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
	
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);

	const BoardT newBoard = PieceMoveFn::fn(board, yourPieceMap, from, to);

	const MoveFlagsT directCheckFlag = ((bbForSquare(to) & directChecksBb) == BbNone) ? NoMoveFlags : DirectCheckMoveFlag;
	//const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT((MoveFlagsT)(CaptureFlag | discoveredCheckFlag | directCheckFlag), from, to/*, isDirectCheck, isDiscoveredCheck*/));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardT, typename BoardTraitsT, PieceT Piece>
    inline void handlePieceMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {
      typedef typename BoardT::ColorStateT ColorStateT;

      const ColorStateT& myState = board.state[(size_t)BoardTraitsT::Color];
      const SquareT from = myState.pieceSquares[Piece];
      const MoveFlagsT discoveredCheckFlag = ((bbForSquare(from) & discoveriesBb) == BbNone) ? NoMoveFlags : DiscoveredCheckMoveFlag;
      //const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;

      const BitBoardT piecePushesBb = movesBb & ~allYourPiecesBb;

      // (Non-promo-)piece pushes
      typedef PieceMoveFn<BoardT, BoardTraitsT::Color, Piece, PiecePush, NoPieceMapT> PiecePushFn;
      handlePieceMoves<StateT, PosHandlerT, BoardT, BoardTraitsT, PiecePushFn, NoMoveFlags/*PushMove*/>(state, board, NoPieceMapT(), from, piecePushesBb, directChecksBb, discoveredCheckFlag);

      BitBoardT pieceCapturesBb = movesBb & allYourPiecesBb;

#ifdef USE_PROMOS
      typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      if(YourColorTraitsT::HasPromos) {
	const BitBoardT promoPieceCapturesBb = pieceCapturesBb & yourPieceMap.allPromoPiecesBb;
	pieceCapturesBb &= ~promoPieceCapturesBb;

	// (Non-promo-)piece captures of promo pieces
	typedef PieceMoveFn<BoardT, BoardTraitsT::Color, Piece, PiecePromoCapture, ColorPieceMapT> PiecePromoCaptureFn;
	handlePieceMoves<StateT, PosHandlerT, BoardT, BoardTraitsT, PiecePromoCaptureFn, CaptureMoveFlag>(state, board, yourPieceMap, from, promoPieceCapturesBb, directChecksBb, discoveredCheckFlag);
      }
#endif //def USE_PROMOS

      // (Non-promo-)piece captures of non-promo pieces
      typedef PieceMoveFn<BoardT, BoardTraitsT::Color, Piece, PieceCapture, ColorPieceMapT> PieceCaptureFn;
      handlePieceMoves<StateT, PosHandlerT, BoardT, BoardTraitsT, PieceCaptureFn, CaptureMoveFlag>(state, board, yourPieceMap, from, pieceCapturesBb, directChecksBb, discoveredCheckFlag);
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

    template <typename StateT, typename PosHandlerT, typename BoardT, typename BoardTraitsT, typename KingMoveFn, MoveFlagsT CaptureFlag>
    inline void handleKingMoves(StateT state, const BoardT& board, const typename KingMoveFn::PieceMapImplT& yourPieceMap, const SquareT from, BitBoardT toBb/*, const BitBoardT directChecksBb*/, const BitBoardT discoveriesBb, const BitBoardT yourKingRaysBb) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
	
      const BitBoardT fromBb = bbForSquare(from);
      
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	const BitBoardT toBb = bbForSquare(to);

	const BoardT newBoard = KingMoveFn::fn(board, yourPieceMap, from, to);

	// Doh - king can't give direct check
	//const MoveFlagsT directCheckFlag = ((bbForSquare(to) & directChecksBb) == BbNone) ? NoMoveFlags : DirectCheckMoveFlag;
	//const bool isDirectCheck = (toBb & directChecksBb) != BbNone;
	const MoveFlagsT discoveredCheckFlag = (fromBb & discoveriesBb) == BbNone || (toBb & yourKingRaysBb) != BbNone ? NoMoveFlags : DiscoveredCheckMoveFlag;
	//const bool isDiscoveredCheck = (fromBb & discoveriesBb) != BbNone && (toBb & yourKingRaysBb) == BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT((MoveFlagsT)(CaptureFlag | discoveredCheckFlag), from, to/*, isDirectCheck, isDiscoveredCheck*/));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardT, typename BoardTraitsT>
    inline void handleKingMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb/*, const BitBoardT directChecksBb*/, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb, const SquareT yourKingSq) {
      typedef typename BoardT::ColorStateT ColorStateT;

      const ColorStateT& myState = board.state[(size_t)BoardTraitsT::Color];
      const SquareT from = myState.pieceSquares[TheKing];
      const BitBoardT yourKingRaysBb = BishopRays[yourKingSq] | RookRays[yourKingSq];
      
      const BitBoardT kingPushesBb = movesBb & ~allYourPiecesBb;

      // King pushes
      typedef KingMoveFn<BoardT, BoardTraitsT::Color, KingPush, NoPieceMapT> KingPushFn;
      handleKingMoves<StateT, PosHandlerT, BoardT, BoardTraitsT, KingPushFn, NoMoveFlags>(state, board, NoPieceMapT(), from, kingPushesBb/*, directChecksBb*/, discoveriesBb, yourKingRaysBb);

      BitBoardT kingCapturesBb = movesBb & allYourPiecesBb;

#ifdef USE_PROMOS
      typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      if(YourColorTraitsT::HasPromos) {
	const BitBoardT promoPieceCapturesBb = kingCapturesBb & yourPieceMap.allPromoPiecesBb;
	kingCapturesBb &= ~promoPieceCapturesBb;

	// King captures of promo pieces
	typedef KingMoveFn<BoardTraitsT::Color, KingPromoCapture, ColorPieceMapT> KingPromoCaptureFn;
	handleKingMoves<StateT, PosHandlerT, BoardTraitsT, KingPromoCaptureFn, CaptureMoveFlag>(state, board, yourPieceMap, from, promoPieceCapturesBb/*, directChecksBb*/, discoveriesBb, yourKingRaysBb);
      }
#endif //def USE_PROMOS

      // King captures of non-promo pieces
      typedef KingMoveFn<BoardT, BoardTraitsT::Color, KingCapture, ColorPieceMapT> KingCaptureFn;
      handleKingMoves<StateT, PosHandlerT, BoardT, BoardTraitsT, KingCaptureFn, CaptureMoveFlag>(state, board, yourPieceMap, from, kingCapturesBb/*, directChecksBb*/, discoveriesBb, yourKingRaysBb);
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

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename PromoPieceMoveFn, MoveTypeT MoveType>
    inline void handlePromoPieceMoves(StateT state, const BoardT& board, const typename PromoPieceMoveFn::PieceMapImplT& yourPieceMap, const int promoIndex, const PromoPieceT promoPiece, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const MoveFlagsT discoveredCheckFlag) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
      
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = PromoPieceMoveFn::fn(board, promoIndex, promoPiece, yourPieceMap, from, to);
	
	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(MoveType, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePromoPieceMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const int promoIndex, const PromoPieceT promoPiece, const SquareT from, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {
      const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;

      const BitBoardT piecePushesBb = movesBb & ~allYourPiecesBb;

      // Promo-piece pushes
      typedef PromoPieceMoveFn<BoardTraitsT::Color, PromoPiecePush, NoPieceMapT> PromoPiecePushFn;
      handlePromoPieceMoves<StateT, PosHandlerT, BoardTraitsT, PromoPiecePushFn, PushMove>(state, board, NoPieceMapT(), promoIndex, promoPiece, from, piecePushesBb, directChecksBb, isDiscoveredCheck);

      BitBoardT pieceCapturesBb = movesBb & allYourPiecesBb;
      const BitBoardT promoPieceCapturesBb = pieceCapturesBb & yourPieceMap.allPromoPiecesBb;
      pieceCapturesBb &= ~promoPieceCapturesBb;

      // Promo-piece captures of promo pieces
      typedef PromoPieceMoveFn<BoardTraitsT::Color, PromoPiecePromoCapture, ColorPieceMapT> PromoPiecePromoCaptureFn;
      handlePromoPieceMoves<StateT, PosHandlerT, BoardTraitsT, PromoPiecePromoCaptureFn, CaptureMove>(state, board, yourPieceMap, promoIndex, promoPiece, from, promoPieceCapturesBb, directChecksBb, isDiscoveredCheck);

      // Promo-piece captures of non-promo pieces
      typedef PromoPieceMoveFn<BoardTraitsT::Color, PromoPieceCapture, ColorPieceMapT> PromoPieceCaptureFn;
      handlePromoPieceMoves<StateT, PosHandlerT, BoardTraitsT, PromoPieceCaptureFn, CaptureMove>(state, board, yourPieceMap, promoIndex, promoPiece, from, pieceCapturesBb, directChecksBb, isDiscoveredCheck);
    }
#endif //def USE_PROMOS

    //
    // Castling moves
    //

    template <typename StateT, typename PosHandlerT, typename BoardT, typename BoardTraitsT, CastlingRightsT CastlingRight>
    inline void handleCastlingMove(StateT state, const BoardT& board, const /*bool isDiscoveredCheck*/MoveFlagsT discoveredCheckFlag) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
      
      const ColorT Color = BoardTraitsT::Color;
      
      const BoardT newBoard1 = pushPiece<BoardT, Color, TheKing>(board, CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo);
      const BoardT newBoard = pushPiece<BoardT, Color, CastlingTraitsT<Color, CastlingRight>::TheRook>(newBoard1, CastlingTraitsT<Color, CastlingRight>::RookFrom, CastlingTraitsT<Color, CastlingRight>::RookTo);

      // We use the king (from and) to square by convention
      ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT((MoveFlagsT)(CastlingMoveFlag | discoveredCheckFlag), CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo));
    }

    template <typename StateT, typename PosHandlerT, typename BoardT, typename BoardTraitsT>
    inline void makeAllLegalMoves(StateT state, const BoardT& board) {
      typedef typename BoardT::ColorStateT ColorStateT;

      typedef typename ColorPieceBbsImplType<BoardT>::ColorPieceBbsT ColorPieceBbsT;
      typedef typename LegalMovesImplType<BoardT>::LegalMovesT LegalMovesT;
      
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;

      // Generate (legal) moves
      const LegalMovesT legalMoves = genLegalMoves<BoardT, BoardTraitsT>(board);

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
	  printBoard(board);
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
#ifdef USE_PROMOS
	handlePawnMoves<StateT, PosHandlerT, BoardTraitsT>(state, board, yourPieceMap, legalMoves.pawnMoves, legalMoves.directChecks.pawnChecksBb, legalMoves.discoveredChecks.pawnPushDiscoveryMasksBb, legalMoves.discoveredChecks.pawnLeftDiscoveryMasksBb, legalMoves.discoveredChecks.pawnRightDiscoveryMasksBb, legalMoves.discoveredChecks.isLeftEpDiscovery, legalMoves.discoveredChecks.isRightEpDiscovery, legalMoves.yourKingRookAttacksBb, legalMoves.yourKingBishopAttacksBb);
#else
	handlePawnMoves<StateT, PosHandlerT, BoardT, BoardTraitsT>(state, board, yourPieceMap, legalMoves.pawnMoves, legalMoves.directChecks.pawnChecksBb, legalMoves.discoveredChecks.pawnPushDiscoveryMasksBb, legalMoves.discoveredChecks.pawnLeftDiscoveryMasksBb, legalMoves.discoveredChecks.pawnRightDiscoveryMasksBb, legalMoves.discoveredChecks.isLeftEpDiscovery, legalMoves.discoveredChecks.isRightEpDiscovery, BbNone, BbNone);
#endif //def USE_PROMOS
	
	// Knights
	handlePieceMoves<StateT, PosHandlerT, BoardT, BoardTraitsT, Knight1>(state, board, yourPieceMap, legalMoves.pieceMoves[Knight1], legalMoves.directChecks.knightChecksBb, (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb);
	handlePieceMoves<StateT, PosHandlerT, BoardT, BoardTraitsT, Knight2>(state, board, yourPieceMap, legalMoves.pieceMoves[Knight2], legalMoves.directChecks.knightChecksBb, (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb);
	
	// Bishops
	handlePieceMoves<StateT, PosHandlerT, BoardT, BoardTraitsT, Bishop1>(state, board, yourPieceMap, legalMoves.pieceMoves[Bishop1], legalMoves.directChecks.bishopChecksBb, legalMoves.discoveredChecks.orthogDiscoveryPiecesBb, allYourPiecesBb); 
	handlePieceMoves<StateT, PosHandlerT, BoardT, BoardTraitsT, Bishop2>(state, board, yourPieceMap, legalMoves.pieceMoves[Bishop2], legalMoves.directChecks.bishopChecksBb, legalMoves.discoveredChecks.orthogDiscoveryPiecesBb, allYourPiecesBb); 

	// Rooks
	handlePieceMoves<StateT, PosHandlerT, BoardT, BoardTraitsT, Rook1>(state, board, yourPieceMap, legalMoves.pieceMoves[Rook1], legalMoves.directChecks.rookChecksBb, legalMoves.discoveredChecks.diagDiscoveryPiecesBb, allYourPiecesBb); 
	handlePieceMoves<StateT, PosHandlerT, BoardT, BoardTraitsT, Rook2>(state, board, yourPieceMap, legalMoves.pieceMoves[Rook2], legalMoves.directChecks.rookChecksBb, legalMoves.discoveredChecks.diagDiscoveryPiecesBb, allYourPiecesBb); 

	// Queen
	handlePieceMoves<StateT, PosHandlerT, BoardT, BoardTraitsT, TheQueen>(state, board, yourPieceMap, legalMoves.pieceMoves[TheQueen], (legalMoves.directChecks.bishopChecksBb | legalMoves.directChecks.rookChecksBb), /*discoveriesBb*/BbNone, allYourPiecesBb); 

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

	    handlePromoPieceMoves<StateT, PosHandlerT, BoardTraitsT>(state, board, yourPieceMap, promoIndex, promoPiece, promoPieceSq, legalMoves.promoPieceMoves[promoIndex], directChecksBb, discoveriesBb, allYourPiecesBb); 
	  }
	}
#endif //def USE_PROMOS
	
	// Castling
	CastlingRightsT canCastleFlags = legalMoves.canCastleFlags;
	if(canCastleFlags) {
	  if((canCastleFlags & CanCastleKingside)) {
	    handleCastlingMove<StateT, PosHandlerT, BoardT, BoardTraitsT, CanCastleKingside>(state, board, legalMoves.discoveredChecks./*isKingsideCastlingDiscovery*/kingsideCastlingDiscoveryFlag);
	  }	

	  if((canCastleFlags & CanCastleQueenside)) {
	    handleCastlingMove<StateT, PosHandlerT, BoardT, BoardTraitsT, CanCastleQueenside>(state, board, legalMoves.discoveredChecks./*isQueensideCastlingDiscovery*/queensideCastlingDiscoveryFlag);
	  }
	}

      } // nChecks < 2
      
      // King - discoveries from king moves are a pain in the butt because each move direction is potentially different.
      handleKingMoves<StateT, PosHandlerT, BoardT, BoardTraitsT>(state, board, yourPieceMap, legalMoves.pieceMoves[TheKing], /*/directChecksBb = /BbNone,*/ (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb, yourState.pieceSquares[TheKing]); 
    }
     
  } // namespace MakeMove
} // namespace Chess

#endif //ndef MAKE_MOVE_HPP
