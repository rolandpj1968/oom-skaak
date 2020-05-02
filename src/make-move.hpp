#ifndef MAKE_MOVE_HPP
#define MAKE_MOVE_HPP

#include "types.hpp"
#include "board.hpp"
#include "board-utils.hpp"
#include "move-gen.hpp"

namespace Chess {

  using namespace Board;
  
  namespace MakeMove {

    //
    // Consumer of this must provide a position handler type that provides a color-reversed pos handler type,
    //   and a pos handler type that does promotion/demotion between board representatives.
    //
    // e.g.
    //
    // typedef blah MyStateT;
    //
    // template <typename BoardT, Color>
    // struct MyPosHandlerT {
    //
    //   typedef MyPosHandlerT<BoardT, OtherColorT<Color>::value> ReverseT;
    //   typedef MyPosHandlerT<typename BoardType<BoardT>::WithPromosT, Color> WithPromosT;
    //   typedef MyPosHandlerT<typename BoardType<BoardT>::WithoutPromosT, Color> WithoutPromosT;
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

    template <typename BoardT, ColorT Color> struct PawnMoveFn<BoardT, Color, PawnPromoCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return capturePromoPieceWithPawn<BoardT, Color>(board, yourPieceMap, from, to);
      }
    };

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

    template <typename BoardT, ColorT Color> struct PawnPromoMoveFn<BoardT, Color, PawnCaptureToPromo, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
    	return captureWithPawnToPromo<BoardT, Color>(board, yourPieceMap, from, to, promoPiece);
      }
    };

    template <typename BoardT, ColorT Color> struct PawnPromoMoveFn<BoardT, Color, PawnPromoCaptureToPromo, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
    	return capturePromoPieceWithPawnToPromo<BoardT, Color>(board, yourPieceMap, from, to, promoPiece);
      }
    };

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, typename To2FromFn, typename PawnPromoMoveFn, MoveTypeT MoveType>
    inline void handlePawnsMoveToPromo(StateT state, const BoardT& board, const typename PawnPromoMoveFn::PieceMapImplT& yourPieceMap, BitBoardT pawnsMoveBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;

      const SquareT yourKingSq = board.state[(size_t)OtherColorT<Color>::value].basic.pieceSquares[TheKing];
	
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
	const bool isKnightCheck = (MoveGen::KnightAttacks[yourKingSq] & toBb) != BbNone;
	ReversePosHandlerT::handlePos(state, knightPromoBoard, MoveInfoT(MoveType, from, to, isKnightCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT rookPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoRook);
	const bool isRookCheck = orthogCheckBb != BbNone;
	ReversePosHandlerT::handlePos(state, rookPromoBoard, MoveInfoT(MoveType, from, to, isRookCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT bishopPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoBishop);
	const bool isBishopCheck = diagCheckBb != BbNone;
	ReversePosHandlerT::handlePos(state, bishopPromoBoard, MoveInfoT(MoveType, from, to, isBishopCheck, isDiscoveredCheck, /*isPromo*/true));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, typename To2FromFn, bool IsPushTwo>
    inline void handlePawnsNonPromoPush(StateT state, const BoardT& board, BitBoardT pawnsPushBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
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
    inline void handlePawnsPromoPush(StateT state, const FullBoardT& board, BitBoardT pawnsPushBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {
      typedef PawnPromoMoveFn<FullBoardT, Color, PawnPushToPromo, NoPieceMapT> PawnPushToPromoFn;
      handlePawnsMoveToPromo<StateT, PosHandlerT, FullBoardT, Color, To2FromFn, PawnPushToPromoFn, PushMove>(state, board, NoPieceMapT(), pawnsPushBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
    }

    template <typename StateT, typename PosHandlerT, ColorT Color, typename To2FromFn, bool IsPushTwo>
    inline void handlePawnsPromoPush(StateT state, const BasicBoardT& board, BitBoardT pawnsPushBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {
      // Upgrade to FullBoardT
      const FullBoardT boardWithPromos = copyBoard<FullBoardT, BasicBoardT>(board);

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

    template <typename StateT, typename PosHandlerT, ColorT Color, typename To2FromFn>
    inline BitBoardT handlePawnsNonPromoCaptureOfPromos(StateT state, const BasicBoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      // No promos
      return pawnsCaptureBb;
    }

    template <typename StateT, typename PosHandlerT, ColorT Color, typename To2FromFn>
    inline BitBoardT handlePawnsNonPromoCaptureOfPromos(StateT state, const FullBoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      BitBoardT promoPieceCapturesBb = pawnsCaptureBb & yourPieceMap.allPromoPiecesBb;
	
      typedef PawnMoveFn<FullBoardT, Color, PawnPromoCapture, ColorPieceMapT> PawnPromoCaptureFn;
      handlePawnsMove<StateT, PosHandlerT, FullBoardT, Color, To2FromFn, PawnPromoCaptureFn, CaptureMove>(state, board, yourPieceMap, promoPieceCapturesBb, directChecksBb, discoveriesBb);
	
      return pawnsCaptureBb & ~promoPieceCapturesBb;
    }

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, typename To2FromFn>
    inline void handlePawnsNonPromoCapture(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {

      const BitBoardT pawnsCaptureOfNonPromosBb = handlePawnsNonPromoCaptureOfPromos<StateT, PosHandlerT, Color, To2FromFn>(state, board, yourPieceMap, pawnsCaptureBb, directChecksBb, discoveriesBb);

      // (Non-promo-piece) capture (without pawn promo)
      typedef PawnMoveFn<BoardT, Color, PawnCapture, ColorPieceMapT> PawnCaptureFn;
      handlePawnsMove<StateT, PosHandlerT, BoardT, Color, To2FromFn, PawnCaptureFn, CaptureMove>(state, board, yourPieceMap, pawnsCaptureOfNonPromosBb, directChecksBb, discoveriesBb);
    }

    template <typename StateT, typename PosHandlerT, ColorT Color, typename To2FromFn>
    inline void handlePawnsPromoCapture(StateT state, const FullBoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {

      const BitBoardT promoPieceCapturesToPromoBb = pawnsCaptureBb & yourPieceMap.allPromoPiecesBb;
      pawnsCaptureBb &= ~promoPieceCapturesToPromoBb;
      
      // Promo piece captures with pawn promo
      typedef PawnPromoMoveFn<FullBoardT, Color, PawnPromoCaptureToPromo, ColorPieceMapT> PawnPromoCaptureToPromoFn;
      handlePawnsMoveToPromo<StateT, PosHandlerT, FullBoardT, Color, To2FromFn, PawnPromoCaptureToPromoFn, CaptureMove>(state, board, yourPieceMap, promoPieceCapturesToPromoBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
	
      // Non-promo-piece captures with pawn promo
      typedef PawnPromoMoveFn<FullBoardT, Color, PawnCaptureToPromo, ColorPieceMapT> PawnCaptureToPromoFn;
      handlePawnsMoveToPromo<StateT, PosHandlerT, FullBoardT, Color, To2FromFn, PawnCaptureToPromoFn, CaptureMove>(state, board, yourPieceMap, pawnsCaptureBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
    }

    template <typename StateT, typename PosHandlerT, ColorT Color, typename To2FromFn>
    inline void handlePawnsPromoCapture(StateT state, const BasicBoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {
      // Upgrade to FullBoardT
      const FullBoardT boardWithPromos = copyBoard<FullBoardT, BasicBoardT>(board);

      // No capture of promo pieces to consider, so just handle capture of non-promo pieces
      typedef PawnPromoMoveFn<FullBoardT, Color, PawnCaptureToPromo, ColorPieceMapT> PawnCaptureToPromoFn;
      handlePawnsMoveToPromo<StateT, typename PosHandlerT::WithPromosT, FullBoardT, Color, To2FromFn, PawnCaptureToPromoFn, CaptureMove>(state, boardWithPromos, yourPieceMap, pawnsCaptureBb, directChecksBb, discoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
    }

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, typename To2FromFn>
    inline void handlePawnEpCapture(StateT state, const BoardT& board, BitBoardT pawnsEpCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const bool isEpDiscovery) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
      
      // There can be only 1 en-passant capture, so no need to loop
      if(pawnsEpCaptureBb) {
	const SquareT to = Bits::lsb(pawnsEpCaptureBb);
	const SquareT from = To2FromFn::fn(to);
	const SquareT captureSq = MoveGen::pawnPushOneTo2From<Color>(to);

	const BoardT newBoard = captureEp<BoardT, Color>(board, from, to, captureSq);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = isEpDiscovery || (bbForSquare(from) & discoveriesBb) != BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(EpCaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void handlePawnNonPromoMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const MoveGen::PawnPushesAndCapturesT& pawnMoves, const BitBoardT directChecksBb, const BitBoardT pushDiscoveriesBb, const BitBoardT leftDiscoveriesBb, const BitBoardT rightDiscoveriesBb, const bool isLeftEpDiscovery, const bool isRightEpDiscovery) {

      // Pawn pushes one square forward
      handlePawnsNonPromoPush<StateT, PosHandlerT, BoardT, Color, MoveGen::PawnPushOneTo2FromFn<Color>, /*IsPushTwo =*/false>(state, board, pawnMoves.pushesOneBb & ~LastRankBbT<Color>::LastRankBb, directChecksBb, pushDiscoveriesBb);
      // Pawn pushes two squares forward
      handlePawnsNonPromoPush<StateT, PosHandlerT, BoardT, Color, MoveGen::PawnPushTwoTo2FromFn<Color>, /*IsPushTwo =*/true>(state, board, pawnMoves.pushesTwoBb, directChecksBb, pushDiscoveriesBb);
	
      // Pawn captures left
      handlePawnsNonPromoCapture<StateT, PosHandlerT, BoardT, Color, PawnAttackLeftTo2FromFn<Color>>(state, board, yourPieceMap, pawnMoves.capturesLeftBb & ~LastRankBbT<Color>::LastRankBb, directChecksBb, leftDiscoveriesBb);
      // Pawn captures right
      handlePawnsNonPromoCapture<StateT, PosHandlerT, BoardT, Color, PawnAttackRightTo2FromFn<Color>>(state, board, yourPieceMap, pawnMoves.capturesRightBb & ~LastRankBbT<Color>::LastRankBb, directChecksBb, rightDiscoveriesBb);      

      // Pawn en-passant capture left
      handlePawnEpCapture<StateT, PosHandlerT, BoardT, Color, PawnAttackLeftTo2FromFn<Color>>(state, board, pawnMoves.epCaptures.epLeftCaptureBb, directChecksBb, leftDiscoveriesBb, isLeftEpDiscovery);
      // Pawn en-passant capture right
      handlePawnEpCapture<StateT, PosHandlerT, BoardT, Color, PawnAttackRightTo2FromFn<Color>>(state, board, pawnMoves.epCaptures.epRightCaptureBb, directChecksBb, rightDiscoveriesBb, isRightEpDiscovery);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void handlePawnPromoMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const MoveGen::PawnPushesAndCapturesT& pawnMoves, const BitBoardT directChecksBb, const BitBoardT pushDiscoveriesBb, const BitBoardT leftDiscoveriesBb, const BitBoardT rightDiscoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {

      // Pawn pushes one square forward
      handlePawnsPromoPush<StateT, PosHandlerT, Color, MoveGen::PawnPushOneTo2FromFn<Color>, /*IsPushTwo =*/false>(state, board, pawnMoves.pushesOneBb & LastRankBbT<Color>::LastRankBb, directChecksBb, pushDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
	
      // Pawn captures left
      handlePawnsPromoCapture<StateT, PosHandlerT, Color, PawnAttackLeftTo2FromFn<Color>>(state, board, yourPieceMap, pawnMoves.capturesLeftBb & LastRankBbT<Color>::LastRankBb, directChecksBb, leftDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
      // Pawn captures right
      handlePawnsPromoCapture<StateT, PosHandlerT, Color, PawnAttackRightTo2FromFn<Color>>(state, board, yourPieceMap, pawnMoves.capturesRightBb & LastRankBbT<Color>::LastRankBb, directChecksBb, rightDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
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

    template <typename BoardT, ColorT Color, PieceT Piece> struct PieceMoveFn<BoardT, Color, Piece, PiecePromoCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return capturePromoPieceWithPiece<BoardT, Color, Piece>(board, yourPieceMap, from, to);
      }
    };

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
    
    template <typename StateT, typename PosHandlerT, ColorT Color, PieceT Piece>
    inline BitBoardT handlePiecePromoCaptures(StateT state, const BasicBoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const BitBoardT pieceCapturesBb, const BitBoardT directChecksBb, const bool isDiscoveredCheck) {
      // No promo pieces
      return pieceCapturesBb;
    }
    
    template <typename StateT, typename PosHandlerT, ColorT Color, PieceT Piece>
    inline BitBoardT handlePiecePromoCaptures(StateT state, const FullBoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const BitBoardT pieceCapturesBb, const BitBoardT directChecksBb, const bool isDiscoveredCheck) {
      const BitBoardT promoPieceCapturesBb = pieceCapturesBb & yourPieceMap.allPromoPiecesBb;

      if(false && Color == Black && promoPieceCapturesBb != BbNone) {
	printf("\n---------------------------------------------------------------------------------\n");
	printf("We got us some promo-piece captures - from %s\n\n", SquareStr[from]);
	BoardUtils::printBoard(board);
	printf("\nCapturing at:\n");
	BoardUtils::printBb(promoPieceCapturesBb);
	printf("\n---------------------------------------------------------------------------------\n");
      }

      // (Non-promo-)piece captures of promo pieces
      typedef PieceMoveFn<FullBoardT, Color, Piece, PiecePromoCapture, ColorPieceMapT> PiecePromoCaptureFn;
      handlePieceMoves<StateT, PosHandlerT, FullBoardT, Color, PiecePromoCaptureFn, CaptureMove>(state, board, yourPieceMap, from, promoPieceCapturesBb, directChecksBb, isDiscoveredCheck);

      return pieceCapturesBb & ~promoPieceCapturesBb;
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, PieceT Piece>
    inline void handlePieceMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {
      typedef typename BoardT::ColorStateT ColorStateT;

      const ColorStateT& myState = board.state[(size_t)Color];
      const SquareT from = myState.basic.pieceSquares[Piece];
      const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;

      const BitBoardT piecePushesBb = movesBb & ~allYourPiecesBb;

      // (Non-promo-)piece pushes
      typedef PieceMoveFn<BoardT, Color, Piece, PiecePush, NoPieceMapT> PiecePushFn;
      handlePieceMoves<StateT, PosHandlerT, BoardT, Color, PiecePushFn, PushMove>(state, board, NoPieceMapT(), from, piecePushesBb, directChecksBb, isDiscoveredCheck);

      const BitBoardT pieceCapturesBb = movesBb & allYourPiecesBb;

      // (Non-promo-)piece captures of promo pieces
      const BitBoardT pieceNonPromoCaptures = handlePiecePromoCaptures<StateT, PosHandlerT, Color, Piece>(state, board, yourPieceMap, from, pieceCapturesBb, directChecksBb, isDiscoveredCheck);

      // (Non-promo-)piece captures of non-promo pieces
      typedef PieceMoveFn<BoardT, Color, Piece, PieceCapture, ColorPieceMapT> PieceCaptureFn;
      handlePieceMoves<StateT, PosHandlerT, BoardT, Color, PieceCaptureFn, CaptureMove>(state, board, yourPieceMap, from, pieceNonPromoCaptures, directChecksBb, isDiscoveredCheck);
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

    template <typename BoardT, ColorT Color> struct KingMoveFn<BoardT, Color, KingPromoCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return capturePromoPieceWithPiece<BoardT, Color, TheKing>(board, yourPieceMap, from, to);
      }
    };

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
    
    template <typename StateT, typename PosHandlerT, ColorT Color>
    inline BitBoardT handleKingPromoCaptures(StateT state, const BasicBoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const BitBoardT kingCapturesBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRaysBb) {
      // No promo pieces
      return kingCapturesBb;
    }
    
    template <typename StateT, typename PosHandlerT, ColorT Color>
    inline BitBoardT handleKingPromoCaptures(StateT state, const FullBoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const BitBoardT kingCapturesBb, const BitBoardT discoveriesBb, const BitBoardT yourKingRaysBb) {
      const BitBoardT promoPieceCapturesBb = kingCapturesBb & yourPieceMap.allPromoPiecesBb;
      
      // King captures of promo pieces
      typedef KingMoveFn<FullBoardT, Color, KingPromoCapture, ColorPieceMapT> KingPromoCaptureFn;
      handleKingMoves<StateT, PosHandlerT, FullBoardT, Color, KingPromoCaptureFn, CaptureMove>(state, board, yourPieceMap, from, promoPieceCapturesBb, discoveriesBb, yourKingRaysBb);

      return kingCapturesBb & ~promoPieceCapturesBb;
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void handleKingMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb, const SquareT yourKingSq) {
      typedef typename BoardT::ColorStateT ColorStateT;

      const ColorStateT& myState = board.state[(size_t)Color];
      const SquareT from = myState.basic.pieceSquares[TheKing];
      const BitBoardT yourKingRaysBb = MoveGen::BishopRays[yourKingSq] | MoveGen::RookRays[yourKingSq];
      
      const BitBoardT kingPushesBb = movesBb & ~allYourPiecesBb;

      // King pushes
      typedef KingMoveFn<BoardT, Color, KingPush, NoPieceMapT> KingPushFn;
      handleKingMoves<StateT, PosHandlerT, BoardT, Color, KingPushFn, PushMove>(state, board, NoPieceMapT(), from, kingPushesBb, discoveriesBb, yourKingRaysBb);

      const BitBoardT kingCapturesBb = movesBb & allYourPiecesBb;
      const BitBoardT kingNonPromoCapturesBb = handleKingPromoCaptures<StateT, PosHandlerT, Color>(state, board, yourPieceMap, from, kingCapturesBb, discoveriesBb, yourKingRaysBb);

      // King captures of non-promo pieces
      typedef KingMoveFn<BoardT, Color, KingCapture, ColorPieceMapT> KingCaptureFn;
      handleKingMoves<StateT, PosHandlerT, BoardT, Color, KingCaptureFn, CaptureMove>(state, board, yourPieceMap, from, kingNonPromoCapturesBb, discoveriesBb, yourKingRaysBb);
    }
    
    //
    // Promo piece moves
    //

    enum PromoPieceMoveT {
      PromoPiecePush,
      PromoPieceCapture,
      PromoPiecePromoCapture
    };

    template <typename BoardT, ColorT Color, PromoPieceMoveT, typename PieceMapT>
    struct PromoPieceMoveFn {
      static BoardT fn(const BoardT& board, const PieceMapT& yourPieceMap, const SquareT from, const SquareT to);
    };
    
    template <typename BoardT, ColorT Color> struct PromoPieceMoveFn<BoardT, Color, PromoPiecePush, NoPieceMapT> {
      typedef NoPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const NoPieceMapT&, const SquareT from, const SquareT to) {
	return pushPromoPiece<BoardT, Color>(board, promoIndex, promoPiece, to);
      }
    };

    template <typename BoardT, ColorT Color> struct PromoPieceMoveFn<BoardT, Color, PromoPieceCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return captureWithPromoPiece<BoardT, Color>(board, promoIndex, promoPiece, yourPieceMap, from, to);
      }
    };

    template <typename BoardT, ColorT Color> struct PromoPieceMoveFn<BoardT, Color, PromoPiecePromoCapture, ColorPieceMapT> {
      typedef ColorPieceMapT PieceMapImplT;
      static BoardT fn(const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
	return capturePromoPieceWithPromoPiece<BoardT, Color>(board, promoIndex, promoPiece, yourPieceMap, from, to);
      }
    };

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, typename PromoPieceMoveFn, MoveTypeT MoveType>
    inline void handlePromoPieceMoves(StateT state, const BoardT& board, const typename PromoPieceMoveFn::PieceMapImplT& yourPieceMap, const int promoIndex, const PromoPieceT promoPiece, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const bool isDiscoveredCheck) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
      
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = PromoPieceMoveFn::fn(board, promoIndex, promoPiece, yourPieceMap, from, to);
	
	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(MoveType, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void handlePromoPieceMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const int promoIndex, const PromoPieceT promoPiece, const SquareT from, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {
      const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;

      const BitBoardT piecePushesBb = movesBb & ~allYourPiecesBb;

      // Promo-piece pushes
      typedef PromoPieceMoveFn<BoardT, Color, PromoPiecePush, NoPieceMapT> PromoPiecePushFn;
      handlePromoPieceMoves<StateT, PosHandlerT, BoardT, Color, PromoPiecePushFn, PushMove>(state, board, NoPieceMapT(), promoIndex, promoPiece, from, piecePushesBb, directChecksBb, isDiscoveredCheck);

      const BitBoardT pieceCapturesBb = movesBb & allYourPiecesBb;
      
      // Promo-piece captures of promo pieces
      const BitBoardT promoPieceCapturesBb = pieceCapturesBb & yourPieceMap.allPromoPiecesBb;
      typedef PromoPieceMoveFn<BoardT, Color, PromoPiecePromoCapture, ColorPieceMapT> PromoPiecePromoCaptureFn;
      handlePromoPieceMoves<StateT, PosHandlerT, BoardT, Color, PromoPiecePromoCaptureFn, CaptureMove>(state, board, yourPieceMap, promoIndex, promoPiece, from, promoPieceCapturesBb, directChecksBb, isDiscoveredCheck);

      // Promo-piece captures of non-promo pieces
      const BitBoardT nonPromoPieceCaptures = pieceCapturesBb & ~promoPieceCapturesBb;
      typedef PromoPieceMoveFn<BoardT, Color, PromoPieceCapture, ColorPieceMapT> PromoPieceCaptureFn;
      handlePromoPieceMoves<StateT, PosHandlerT, BoardT, Color, PromoPieceCaptureFn, CaptureMove>(state, board, yourPieceMap, promoIndex, promoPiece, from, nonPromoPieceCaptures, directChecksBb, isDiscoveredCheck);
    }

    //
    // Castling moves
    //

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, CastlingRightsT CastlingRight>
    inline void handleCastlingMove(StateT state, const BoardT& board, const bool isDiscoveredCheck) {
      const BoardT newBoard1 = pushPiece<BoardT, Color, TheKing>(board, MoveGen::CastlingTraitsT<Color, CastlingRight>::KingFrom, MoveGen::CastlingTraitsT<Color, CastlingRight>::KingTo);
      const BoardT newBoard = pushPiece<BoardT, Color, MoveGen::CastlingTraitsT<Color, CastlingRight>::TheRook>(newBoard1, MoveGen::CastlingTraitsT<Color, CastlingRight>::RookFrom, MoveGen::CastlingTraitsT<Color, CastlingRight>::RookTo);

      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
 
      // We use the king (from and) to square by convention
      ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(CastlingMove, MoveGen::CastlingTraitsT<Color, CastlingRight>::KingFrom, MoveGen::CastlingTraitsT<Color, CastlingRight>::KingTo, /*isDirectCheck*/false, isDiscoveredCheck));
    }

    template <typename BoardT>
    inline BitBoardT getAllPromoPiecesBb(const typename MoveGen::ColorPieceBbsImplType<BoardT>::ColorPieceBbsT& pieceBbs);

    template <>
    inline BitBoardT getAllPromoPiecesBb<BasicBoardT>(const typename MoveGen::ColorPieceBbsImplType<BasicBoardT>::ColorPieceBbsT& pieceBbs) {
      return BbNone;
    }

    template <>
    inline BitBoardT getAllPromoPiecesBb<FullBoardT>(const typename MoveGen::ColorPieceBbsImplType<FullBoardT>::ColorPieceBbsT& pieceBbs) {
      return pieceBbs.allPromoPiecesBb;
    }

    template <typename StateT, typename PosHandlerT, ColorT Color>
    inline void makeLegalPromoPieceMoves(StateT state, const BasicBoardT& board, const typename MoveGen::LegalMovesImplType<BasicBoardT>::LegalMovesT& legalMoves, const ColorPieceMapT& yourPieceMap, const BitBoardT allYourPiecesBb) {
      // No promo pieces
    }
    
    template <typename StateT, typename PosHandlerT, ColorT Color>
    inline void makeLegalPromoPieceMoves(StateT state, const FullBoardT& board, const typename MoveGen::LegalMovesImplType<FullBoardT>::LegalMovesT& legalMoves, const ColorPieceMapT& yourPieceMap, const BitBoardT allYourPiecesBb) {
      const FullColorStateImplT& myState = board.state[(size_t)Color];

      // Ugh the bit stuff operates on BitBoardT type
      BitBoardT activePromos = (BitBoardT)myState.promos.activePromos;
      while(activePromos) {
	const int promoIndex = Bits::popLsb(activePromos);
	const PromoPieceAndSquareT promoPieceAndSquare = myState.promos.promos[promoIndex];
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
	
	handlePromoPieceMoves<StateT, PosHandlerT, FullBoardT, Color>(state, board, yourPieceMap, promoIndex, promoPiece, promoPieceSq, legalMoves.promoPieceMoves[promoIndex], directChecksBb, discoveriesBb, allYourPiecesBb); 
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void makeAllLegalMoves(StateT state, const BoardT& board) {
      typedef typename BoardT::ColorStateT ColorStateT;

      typedef typename MoveGen::ColorPieceBbsImplType<BoardT>::ColorPieceBbsT ColorPieceBbsT;
      typedef typename MoveGen::LegalMovesImplType<BoardT>::LegalMovesT LegalMovesT;
      
      const ColorT OtherColor = OtherColorT<Color>::value;

      // Generate (legal) moves
      const LegalMovesT legalMoves = MoveGen::genLegalMoves<BoardT, Color>(board);

      const ColorPieceBbsT& yourPieceBbs = legalMoves.pieceBbs.colorPieceBbs[(size_t)OtherColor];

      const ColorStateT& yourState = board.state[(size_t)OtherColor];
      const BitBoardT allYourPromoPiecesBb = getAllPromoPiecesBb<BoardT>(yourPieceBbs);
      const ColorPieceMapT& yourPieceMap = genColorPieceMap(yourState, allYourPromoPiecesBb);
      
      const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];

      // Is this an illegal pos - note this should never happen(tm) - but we will notice quickly
      if(legalMoves.isIllegalPos) {
	static bool done = false;
	if(!done) {
	  printf("\n============================================== Invalid Pos :( Color %s ===================================\n\n", (Color == White ? "W" : "B"));
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
	  typedef typename MoveGen::PieceBbsImplType<BoardT>::PieceBbsT PieceBbsT;
	  const PieceBbsT& pieceBbs = legalMoves.pieceBbs;
	  const ColorPieceBbsT& myPieceBbs = pieceBbs.colorPieceBbs[(size_t)Color];
	  const ColorPieceBbsT& yourPieceBbs = pieceBbs.colorPieceBbs[(size_t)OtherColor];
	  
	  const BitBoardT allMyPiecesBb = myPieceBbs.bbs[AllPieceTypes];
	  const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];
	  const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;
	  
	  const SquareT yourKingSq = yourState.basic.pieceSquares[TheKing];
	  // These are used for promo-piece check detection
	  const BitBoardT yourKingRookAttacksBb = MoveGen::rookAttacks(yourKingSq, allPiecesBb);
	  const BitBoardT yourKingBishopAttacksBb = MoveGen::bishopAttacks(yourKingSq, allPiecesBb);

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
	makeLegalPromoPieceMoves<StateT, PosHandlerT, Color>(state, board, legalMoves, yourPieceMap, allYourPiecesBb);
	
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
      handleKingMoves<StateT, PosHandlerT, BoardT, Color>(state, board, yourPieceMap, legalMoves.pieceMoves[TheKing], (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb, yourState.basic.pieceSquares[TheKing]); 
    }
     
  } // namespace MakeMove
} // namespace Chess

#endif //ndef MAKE_MOVE_HPP
