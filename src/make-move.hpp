#ifndef MAKE_MOVE_HPP
#define MAKE_MOVE_HPP

#include "types.hpp"
#include "board.hpp"
#include "board-utils.hpp"
#include "move-gen.hpp"
#include "pawn-move.hpp"

namespace Chess {

  using namespace Board;
  
  namespace MakeMove {

    // Are we making moves and handling positions, or are we counting moves?
    struct PosTag {};
    struct CountTag {};

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
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(MoveType, Pawn, from, to, isDirectCheck, isDiscoveredCheck));
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
	bool isOrthogCheck = (toBb & yourKingRookAttacksBb) != BbNone;
	bool isDiagCheck = (toBb & yourKingBishopAttacksBb) != BbNone;
	const BitBoardT fromBb = bbForSquare(from);
	if(MoveType == PushMove) {
	  // Can get check from pawn pushing away from your King to promo Rook or Queen
	  if((fromBb & yourKingRookAttacksBb) != BbNone && fileOf(yourKingSq) == fileOf(from)) {
	    isOrthogCheck = true;
	  }
	} else {
	  //Can get check from pawn capturing away from your King to promo Bishop or Queen
	  if((fromBb & yourKingBishopAttacksBb) != BbNone) {
	    // Capture must be in a direction away from your King
	    const BitBoardT captureDirectionRay = MoveGen::BishopRays[from] & MoveGen::BishopRays[to];
	    if((captureDirectionRay & bbForSquare(yourKingSq)) != BbNone) {
	      isDiagCheck = true;
	    }
	  }
	}
	
	const BoardT queenPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoQueen);
	const bool isQueenCheck = isOrthogCheck || isDiagCheck;
	ReversePosHandlerT::handlePos(state, queenPromoBoard, MoveInfoT(MoveType, Queen, from, to, isQueenCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT knightPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoKnight);
	const bool isKnightCheck = (MoveGen::KnightAttacks[yourKingSq] & toBb) != BbNone;
	ReversePosHandlerT::handlePos(state, knightPromoBoard, MoveInfoT(MoveType, Knight, from, to, isKnightCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT rookPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoRook);
	const bool isRookCheck = isOrthogCheck;
	ReversePosHandlerT::handlePos(state, rookPromoBoard, MoveInfoT(MoveType, Rook, from, to, isRookCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT bishopPromoBoard = PawnPromoMoveFn::fn(board, yourPieceMap, from, to, PromoBishop);
	const bool isBishopCheck = isDiagCheck;
	ReversePosHandlerT::handlePos(state, bishopPromoBoard, MoveInfoT(MoveType, Bishop, from, to, isBishopCheck, isDiscoveredCheck, /*isPromo*/true));
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
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(PushMove, Pawn, from, to, isDirectCheck, isDiscoveredCheck));
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
    inline SquareT pawnAttackLeftFrom2To(const SquareT square);
    template <> inline SquareT pawnAttackLeftFrom2To<White>(const SquareT square) { return square + 7; }
    template <> inline SquareT pawnAttackLeftFrom2To<Black>(const SquareT square) { return square - 9; }

    template <ColorT Color>
    inline SquareT pawnAttackRightTo2From(const SquareT square);
    template <> inline SquareT pawnAttackRightTo2From<White>(const SquareT square) { return square - 9; }
    template <> inline SquareT pawnAttackRightTo2From<Black>(const SquareT square) { return square + 7; }

    template <ColorT Color>
    struct PawnAttackRightTo2FromFn {
      static inline SquareT fn(const SquareT from) { return pawnAttackRightTo2From<Color>(from); }
    };

    template <ColorT Color>
    inline SquareT pawnAttackRightFrom2To(const SquareT square);
    template <> inline SquareT pawnAttackRightFrom2To<White>(const SquareT square) { return square + 9; }
    template <> inline SquareT pawnAttackRightFrom2To<Black>(const SquareT square) { return square - 7; }

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
	const SquareT captureSq = PawnMove::to2FromSq<Color, PawnMove::PushOne>(to);

	const BoardT newBoard = captureEp<BoardT, Color>(board, from, to, captureSq);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = isEpDiscovery || (bbForSquare(from) & discoveriesBb) != BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(EpCaptureMove, Pawn, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void handlePawnNonPromoMoves(const PosTag&, StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const MoveGen::PawnPushesAndCapturesT& pawnMoves, const BitBoardT directChecksBb, const BitBoardT pushDiscoveriesBb, const BitBoardT leftDiscoveriesBb, const BitBoardT rightDiscoveriesBb, const bool isLeftEpDiscovery, const bool isRightEpDiscovery) {

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

    template <ColorT> BitBoardT pawnsLeftAttacksTo2FromBb(const BitBoardT leftAttacksBb);
    template <ColorT> BitBoardT pawnsRightAttacksTo2FromBb(const BitBoardT rightAttacksBb);
    template <ColorT> BitBoardT pawnsPushOneTo2FromBb(const BitBoardT pushesOneBb);
    template <ColorT> BitBoardT pawnsPushTwoTo2FromBb(const BitBoardT pushesTwoBb);

    template <> inline BitBoardT pawnsLeftAttacksTo2FromBb<White>(const BitBoardT leftAttacksBb) {
      return leftAttacksBb >> 7;
    }
    
    template <> inline BitBoardT pawnsRightAttacksTo2FromBb<White>(const BitBoardT rightAttacksBb) {
      return rightAttacksBb >> 9;
    }
    
    template <> inline BitBoardT pawnsPushOneTo2FromBb<White>(const BitBoardT pushesOneBb) {
      return pushesOneBb >> 8;
    }
    
    template <> inline BitBoardT pawnsPushTwoTo2FromBb<White>(const BitBoardT pushesTwoBb) {
      return pushesTwoBb >> 16;
    }

    template <> inline BitBoardT pawnsLeftAttacksTo2FromBb<Black>(const BitBoardT leftAttacksBb) {
      return leftAttacksBb << 9;
    }
    
    template <> inline BitBoardT pawnsRightAttacksTo2FromBb<Black>(const BitBoardT rightAttacksBb) {
      return rightAttacksBb << 7;
    }
    
    template <> inline BitBoardT pawnsPushOneTo2FromBb<Black>(const BitBoardT pushesOneBb) {
      return pushesOneBb << 8;
    }
    
    template <> inline BitBoardT pawnsPushTwoTo2FromBb<Black>(const BitBoardT pushesTwoBb) {
      return pushesTwoBb << 16;
    }

    template <typename BoardT, ColorT Color, typename To2FromFn, bool IsPushTwo>
    inline int countPawnsNonPromoPushCheckmates(const BoardT& board, BitBoardT pawnsPushBb) {
      int checkmates = 0;
      
      while(pawnsPushBb) {
	const SquareT to = Bits::popLsb(pawnsPushBb);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = pushPawn<BoardT, Color, IsPushTwo>(board, from, to);

	checkmates += !BoardUtils::hasLegalMoves<BoardT, OtherColorT<Color>::value>(newBoard);
      }

      return checkmates;
    }
    
    template <typename BoardT, ColorT Color, typename To2FromFn, typename PawnMoveFn>
    inline int countPawnsMoveCheckmates(const BoardT& board, const typename PawnMoveFn::PieceMapImplT& yourPieceMap, BitBoardT pawnsMoveBb) {
      int checkmates = 0;
      
      while(pawnsMoveBb) {
	const SquareT to = Bits::popLsb(pawnsMoveBb);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = PawnMoveFn::fn(board, yourPieceMap, from, to);

	checkmates += !BoardUtils::hasLegalMoves<BoardT, OtherColorT<Color>::value>(newBoard);
      }

      return checkmates;
    }
    
    template <ColorT Color, typename To2FromFn>
    inline BitBoardT countPawnsNonPromoCaptureOfPromosCheckmates(const BasicBoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, int& checkmates) {
      // No promos
      return pawnsCaptureBb;
    }

    template <ColorT Color, typename To2FromFn>
    inline BitBoardT countPawnsNonPromoCaptureOfPromosCheckmates(const FullBoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, int& checkmates) {
      BitBoardT promoPieceCapturesBb = pawnsCaptureBb & yourPieceMap.allPromoPiecesBb;
	
      typedef PawnMoveFn<FullBoardT, Color, PawnPromoCapture, ColorPieceMapT> PawnPromoCaptureFn;
      checkmates += countPawnsMoveCheckmates<FullBoardT, Color, To2FromFn, PawnPromoCaptureFn>(board, yourPieceMap, promoPieceCapturesBb);
	
      return pawnsCaptureBb & ~promoPieceCapturesBb;
    }

    template <typename BoardT, ColorT Color, typename To2FromFn>
    inline int countPawnsNonPromoCaptureCheckmates(const BoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb) {
      int checkmates = 0;
      
      const BitBoardT pawnsCaptureOfNonPromosBb = countPawnsNonPromoCaptureOfPromosCheckmates<Color, To2FromFn>(board, yourPieceMap, pawnsCaptureBb, checkmates);

      // (Non-promo-piece) capture (without pawn promo)
      typedef PawnMoveFn<BoardT, Color, PawnCapture, ColorPieceMapT> PawnCaptureFn;
      checkmates += countPawnsMoveCheckmates<BoardT, Color, To2FromFn, PawnCaptureFn>(board, yourPieceMap, pawnsCaptureOfNonPromosBb);

      return checkmates;
    }

    template <typename BoardT, ColorT Color, typename To2FromFn>
    inline int countPawnEpCaptureCheckmates(const BoardT& board, BitBoardT pawnsEpCaptureBb) {
      int checkmates = 0;
      
      // There can be only 1 en-passant capture, so no need to loop
      if(pawnsEpCaptureBb) {
	const SquareT to = Bits::lsb(pawnsEpCaptureBb);
	const SquareT from = To2FromFn::fn(to);
	const SquareT captureSq = PawnMove::to2FromSq<Color, PawnMove::PushOne>(to);

	const BoardT newBoard = captureEp<BoardT, Color>(board, from, to, captureSq);
	
	checkmates += !BoardUtils::hasLegalMoves<BoardT, OtherColorT<Color>::value>(newBoard);
      }

      return checkmates;
    }

    template <typename StateT, typename CountHandlerT, typename BoardT, ColorT Color>
    inline void handlePawnNonPromoMoves(const CountTag&, StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const MoveGen::PawnPushesAndCapturesT& pawnMoves, const BitBoardT directChecksBb, const BitBoardT pushDiscoveriesBb, const BitBoardT leftDiscoveriesBb, const BitBoardT rightDiscoveriesBb, const bool isLeftEpDiscovery, const bool isRightEpDiscovery) {
      const BitBoardT pawnPushesOneBb = pawnMoves.pushesOneBb & ~LastRankBbT<Color>::LastRankBb;
      const BitBoardT pawnPushesTwoBb = pawnMoves.pushesTwoBb; // pawn push two is never a promotion
      
      const BitBoardT pawnPushesBb = pawnPushesOneBb | pawnPushesTwoBb;
      const int pawnPushesCount = Bits::count(pawnPushesBb);

      const BitBoardT pawnCapturesLeftBb = pawnMoves.capturesLeftBb & ~LastRankBbT<Color>::LastRankBb;
      const int pawnCapturesLeftCount = Bits::count(pawnCapturesLeftBb);

      const BitBoardT pawnCapturesRightBb = pawnMoves.capturesRightBb & ~LastRankBbT<Color>::LastRankBb;
      const int pawnCapturesRightCount = Bits::count(pawnCapturesRightBb);

      const int pawnEpCount = (int)(pawnMoves.epCaptures.epLeftCaptureBb != BbNone) + (int)(pawnMoves.epCaptures.epRightCaptureBb != BbNone);

      const int captures = pawnCapturesLeftCount + pawnCapturesRightCount + pawnEpCount;
      const int nodes = pawnPushesCount + captures;
      const int eps = pawnEpCount;
      const int castles = 0;
      const int promos = 0;

      // Push checks
      
      const int pawnPushesDirectCheckCount = Bits::count(pawnPushesBb & directChecksBb);

      const BitBoardT pawnPushesOneFromBb = pawnsPushOneTo2FromBb<Color>(pawnPushesOneBb);
      const BitBoardT pawnPushesOneDiscoveriesFromBb = pawnPushesOneFromBb & pushDiscoveriesBb;
      const int pawnPushesOneDiscoveriesCount = Bits::count(pawnPushesOneDiscoveriesFromBb);

      const BitBoardT pawnPushesTwoFromBb = pawnsPushTwoTo2FromBb<Color>(pawnPushesTwoBb);
      const BitBoardT pawnPushesTwoDiscoveriesFromBb = pawnPushesTwoFromBb & pushDiscoveriesBb;
      const int pawnPushesTwoDiscoveriesCount = Bits::count(pawnPushesTwoDiscoveriesFromBb);

      const int pawnPushesDiscoveryChecksCount = pawnPushesOneDiscoveriesCount + pawnPushesTwoDiscoveriesCount;
      const int pawnPushesChecksCount = pawnPushesDirectCheckCount + pawnPushesDiscoveryChecksCount; // pawn push is never double check

      // Left-capture checks
      
      const BitBoardT pawnCapturesLeftDirectChecksBb = pawnCapturesLeftBb & directChecksBb;
      const int pawnCapturesLeftDirectChecksCount = Bits::count(pawnCapturesLeftDirectChecksBb);

      const BitBoardT pawnCapturesLeftFromBb = pawnsLeftAttacksTo2FromBb<Color>(pawnCapturesLeftBb);
      const BitBoardT pawnCapturesLeftDiscoveriesFromBb = pawnCapturesLeftFromBb & leftDiscoveriesBb;
      const int pawnCapturesLeftDiscoveriesCount = Bits::count(pawnCapturesLeftDiscoveriesFromBb);
      
      const BitBoardT pawnCapturesLeftDoubleChecksFromBb = pawnsLeftAttacksTo2FromBb<Color>(pawnCapturesLeftDirectChecksBb) & leftDiscoveriesBb;
      const int pawnCapturesLeftDoubleChecksCount = Bits::count(pawnCapturesLeftDoubleChecksFromBb);

      const int pawnCapturesLeftDiscoveredChecksCount = pawnCapturesLeftDiscoveriesCount - pawnCapturesLeftDoubleChecksCount;
      const int pawnCapturesLeftChecksCount = pawnCapturesLeftDirectChecksCount + pawnCapturesLeftDiscoveredChecksCount;

      // Right-capture checks

      const BitBoardT pawnCapturesRightDirectChecksBb = pawnCapturesRightBb & directChecksBb;
      const int pawnCapturesRightDirectChecksCount = Bits::count(pawnCapturesRightDirectChecksBb);

      const BitBoardT pawnCapturesRightFromBb = pawnsRightAttacksTo2FromBb<Color>(pawnCapturesRightBb);
      const BitBoardT pawnCapturesRightDiscoveriesFromBb = pawnCapturesRightFromBb & rightDiscoveriesBb;
      const int pawnCapturesRightDiscoveriesCount = Bits::count(pawnCapturesRightDiscoveriesFromBb);
      
      const BitBoardT pawnCapturesRightDoubleChecksFromBb = pawnsRightAttacksTo2FromBb<Color>(pawnCapturesRightDirectChecksBb) & rightDiscoveriesBb;
      const int pawnCapturesRightDoubleChecksCount = Bits::count(pawnCapturesRightDoubleChecksFromBb);

      const int pawnCapturesRightDiscoveredChecksCount = pawnCapturesRightDiscoveriesCount - pawnCapturesRightDoubleChecksCount;
      const int pawnCapturesRightChecksCount = pawnCapturesRightDirectChecksCount + pawnCapturesRightDiscoveredChecksCount;
      
      // EP check left

      const BitBoardT epLeftCaptureBb = pawnMoves.epCaptures.epLeftCaptureBb;
      const BitBoardT epLeftDirectChecksBb = epLeftCaptureBb & directChecksBb;
      const int epLeftDirectChecksCount = (int)(epLeftDirectChecksBb != BbNone);

      const BitBoardT epLeftCaptureFromBb = pawnsLeftAttacksTo2FromBb<Color>(epLeftCaptureBb);
      const BitBoardT epLeftDiscoveriesFromBb = isLeftEpDiscovery ? epLeftCaptureFromBb : (epLeftCaptureFromBb & leftDiscoveriesBb);
      const int epLeftDiscoveriesCount = (int)(epLeftDiscoveriesFromBb != BbNone);

      const int epLeftDoubleChecksCount = epLeftDirectChecksCount & epLeftDiscoveriesCount;

      const int epLeftDiscoveredChecksCount = epLeftDiscoveriesCount - epLeftDoubleChecksCount;
      const int epLeftChecksCount = epLeftDirectChecksCount + epLeftDiscoveredChecksCount;
      
      // EP check right

      const BitBoardT epRightCaptureBb = pawnMoves.epCaptures.epRightCaptureBb;
      const BitBoardT epRightDirectChecksBb = epRightCaptureBb & directChecksBb;
      const int epRightDirectChecksCount = (int)(epRightDirectChecksBb != BbNone);

      const BitBoardT epRightCaptureFromBb = pawnsRightAttacksTo2FromBb<Color>(epRightCaptureBb);
      const BitBoardT epRightDiscoveriesFromBb = isRightEpDiscovery ? epRightCaptureFromBb : (epRightCaptureFromBb & rightDiscoveriesBb);
      const int epRightDiscoveriesCount = (int)(epRightDiscoveriesFromBb != BbNone);

      const int epRightDoubleChecksCount = epRightDirectChecksCount & epRightDiscoveriesCount;

      const int epRightDiscoveredChecksCount = epRightDiscoveriesCount - epRightDoubleChecksCount;
      const int epRightChecksCount = epRightDirectChecksCount + epRightDiscoveredChecksCount;
      
      // And finally the check totals
      
      const int checks = pawnPushesChecksCount + pawnCapturesLeftChecksCount + pawnCapturesRightChecksCount + epLeftChecksCount + epRightChecksCount;
      const int discoverychecks = pawnPushesDiscoveryChecksCount + pawnCapturesLeftDiscoveredChecksCount + pawnCapturesRightDiscoveredChecksCount + epLeftDiscoveredChecksCount + epRightDiscoveredChecksCount;
      const int doublechecks = pawnCapturesLeftDoubleChecksCount + pawnCapturesRightDoubleChecksCount + epLeftDoubleChecksCount + epRightDoubleChecksCount;

      int checkmates = 0;

      // The path less taken
      if(checks != 0) {
	// Push checks
	if(pawnPushesChecksCount != 0) {
	  // Push one checks
	  const BitBoardT pawnPushesOneChecksBb = (pawnPushesOneBb & directChecksBb) | PawnMove::from2ToBb<Color, PawnMove::PushOne>(pawnPushesOneDiscoveriesFromBb);
	  if(pawnPushesOneChecksBb != BbNone) {
	    checkmates += countPawnsNonPromoPushCheckmates<BoardT, Color, MoveGen::PawnPushOneTo2FromFn<Color>, /*IsPushTwo =*/false>(board, pawnPushesOneChecksBb);
	  }

	  const BitBoardT pawnPushesTwoChecksBb = (pawnPushesTwoBb & directChecksBb) | PawnMove::from2ToBb<Color, PawnMove::PushOne>(PawnMove::from2ToBb<Color, PawnMove::PushOne>(pawnPushesTwoDiscoveriesFromBb));
	  if(pawnPushesTwoChecksBb != BbNone) {
	    checkmates += countPawnsNonPromoPushCheckmates<BoardT, Color, MoveGen::PawnPushTwoTo2FromFn<Color>, /*IsPushTwo =*/true>(board, pawnPushesTwoChecksBb);
	  }
	}

	// Left-capture checks
	if(pawnCapturesLeftChecksCount != 0) {
	  const BitBoardT pawnCapturesLeftChecksBb = pawnCapturesLeftDirectChecksBb | PawnMove::from2ToBb<Color, PawnMove::AttackLeft>(pawnCapturesLeftDiscoveriesFromBb);
	  checkmates += countPawnsNonPromoCaptureCheckmates<BoardT, Color, PawnAttackLeftTo2FromFn<Color>>(board, yourPieceMap, pawnCapturesLeftChecksBb);
	}

	// Right-capture checks
	if(pawnCapturesRightChecksCount != 0) {
	  const BitBoardT pawnCapturesRightChecksBb = pawnCapturesRightDirectChecksBb | PawnMove::from2ToBb<Color, PawnMove::AttackRight>(pawnCapturesRightDiscoveriesFromBb);
	  checkmates += countPawnsNonPromoCaptureCheckmates<BoardT, Color, PawnAttackRightTo2FromFn<Color>>(board, yourPieceMap, pawnCapturesRightChecksBb);
	}

	// EP check left
	if(epLeftChecksCount != 0) {
	  const BitBoardT epLeftChecksBb = epLeftDirectChecksBb | PawnMove::from2ToBb<Color, PawnMove::AttackLeft>(epLeftDiscoveriesFromBb);
	  checkmates += countPawnEpCaptureCheckmates<BoardT, Color, PawnAttackLeftTo2FromFn<Color>>(board, epLeftChecksBb);
	}

	// EP check right
	if(epRightChecksCount != 0) {
	  const BitBoardT epRightChecksBb = epRightDirectChecksBb | PawnMove::from2ToBb<Color, PawnMove::AttackRight>(epRightDiscoveriesFromBb);
	  checkmates += countPawnEpCaptureCheckmates<BoardT, Color, PawnAttackRightTo2FromFn<Color>>(board, epRightChecksBb);
	}
      }
      
      CountHandlerT::handleCount(state, nodes, captures, eps, castles, promos, checks, discoverychecks, doublechecks, checkmates);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void handlePawnPromoMoves(const PosTag&, StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const MoveGen::PawnPushesAndCapturesT& pawnMoves, const BitBoardT directChecksBb, const BitBoardT pushDiscoveriesBb, const BitBoardT leftDiscoveriesBb, const BitBoardT rightDiscoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {

      // Pawn pushes one square forward
      handlePawnsPromoPush<StateT, PosHandlerT, Color, MoveGen::PawnPushOneTo2FromFn<Color>, /*IsPushTwo =*/false>(state, board, pawnMoves.pushesOneBb & LastRankBbT<Color>::LastRankBb, directChecksBb, pushDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
	
      // Pawn captures left
      handlePawnsPromoCapture<StateT, PosHandlerT, Color, PawnAttackLeftTo2FromFn<Color>>(state, board, yourPieceMap, pawnMoves.capturesLeftBb & LastRankBbT<Color>::LastRankBb, directChecksBb, leftDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
      // Pawn captures right
      handlePawnsPromoCapture<StateT, PosHandlerT, Color, PawnAttackRightTo2FromFn<Color>>(state, board, yourPieceMap, pawnMoves.capturesRightBb & LastRankBbT<Color>::LastRankBb, directChecksBb, rightDiscoveriesBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
    }

    enum PawnPromoMoveDirT {
      PawnPromoPushOneDir,
      PawnPromoCaptureLeftDir,
      PawnPromoCaptureRightDir
    };

    template <ColorT Color, PawnPromoMoveDirT> struct PawnPromoMoveDirFns {
      static inline BitBoardT to2FromBb(BitBoardT);
      static inline SquareT from2To(SquareT);
      static inline BitBoardT orthogDirectChecksFromBbFn(const BitBoardT pawnMovesBb, const SquareT yourKingSq, const BitBoardT yourKingRookAttacksBb);
      static inline BitBoardT diagDirectChecksFromBbFn(const BitBoardT pawnMovesBb, const SquareT yourKingSq, const BitBoardT yourKingBishopAttacksBb);
    };

    template <ColorT Color> struct PawnPromoMoveDirFns<Color, PawnPromoPushOneDir> {
      static inline BitBoardT to2FromBb(BitBoardT pawnPushesBb) {
	return pawnsPushOneTo2FromBb<Color>(pawnPushesBb);
      }

      static inline SquareT from2ToBb(SquareT sq) {
	return PawnMove::from2ToSq<Color, PawnMove::PushOne>(sq);
      }
      
      static inline BitBoardT orthogDirectChecksFromBbFn(const BitBoardT pawnPushesBb, const BitBoardT pushesFromBb, const SquareT yourKingSq, const BitBoardT yourKingRookAttacksBb) {
	const BitBoardT horizDirectChecksToBb = pawnPushesBb & yourKingRookAttacksBb;
	const BitBoardT verticDirectChecksFromBb = pushesFromBb & yourKingRookAttacksBb & FileBbs[fileOf(yourKingSq)];
      
	return to2FromBb(horizDirectChecksToBb) | verticDirectChecksFromBb;
      }

      static inline BitBoardT diagDirectChecksFromBbFn(const BitBoardT pawnPushesBb, const BitBoardT pushesFromBb, const SquareT yourKingSq, const BitBoardT yourKingBishopAttacksBb) {
	return to2FromBb(pawnPushesBb & yourKingBishopAttacksBb);
      }
      
    };

    template <ColorT Color> struct PawnPromoMoveDirFns<Color, PawnPromoCaptureLeftDir> {
      static inline BitBoardT to2FromBb(BitBoardT pawnCapturesLeftBb) {
	return pawnsLeftAttacksTo2FromBb<Color>(pawnCapturesLeftBb);
      }

      static inline SquareT from2ToBb(SquareT sq) {
	return pawnAttackLeftFrom2To<Color>(sq);
      }
      
      static inline BitBoardT orthogDirectChecksFromBbFn(const BitBoardT pawnCapturesLeftBb, const BitBoardT capturesLeftFromBb, const SquareT yourKingSq, const BitBoardT yourKingRookAttacksBb) {
	return to2FromBb(pawnCapturesLeftBb & yourKingRookAttacksBb);
      }
      
      static inline BitBoardT diagDirectChecksFromBbFn(const BitBoardT pawnCapturesLeftBb, const BitBoardT capturesLeftFromBb, const SquareT yourKingSq, const BitBoardT yourKingBishopAttacksBb) {
	const BitBoardT rightDirectChecksToBb = pawnCapturesLeftBb & yourKingBishopAttacksBb;
	const BitBoardT leftDirectChecksFromBb = capturesLeftFromBb & yourKingBishopAttacksBb & MoveGen::BishopUniRays[Color == White ? MoveGen::Left : MoveGen::Right][yourKingSq];

	return to2FromBb(rightDirectChecksToBb) | leftDirectChecksFromBb;
      }
    };

    template <ColorT Color> struct PawnPromoMoveDirFns<Color, PawnPromoCaptureRightDir> {
      static inline BitBoardT to2FromBb(BitBoardT pawnCapturesRightBb) {
	return pawnsRightAttacksTo2FromBb<Color>(pawnCapturesRightBb);
      }

      static inline SquareT from2ToBb(SquareT sq) {
	return pawnAttackRightFrom2To<Color>(sq);
      }
      
      static inline BitBoardT orthogDirectChecksFromBbFn(const BitBoardT pawnCapturesRightBb, const BitBoardT capturesRightFromBb, const SquareT yourKingSq, const BitBoardT yourKingRookAttacksBb) {
	return to2FromBb(pawnCapturesRightBb & yourKingRookAttacksBb);
      }

      static inline BitBoardT diagDirectChecksFromBbFn(const BitBoardT pawnCapturesRightBb, const BitBoardT capturesRightFromBb, const SquareT yourKingSq, const BitBoardT yourKingBishopAttacksBb) {
	const BitBoardT rightDirectChecksToBb = pawnCapturesRightBb & yourKingBishopAttacksBb;
	const BitBoardT rightDirectChecksFromBb = capturesRightFromBb & yourKingBishopAttacksBb & MoveGen::BishopUniRays[Color == White ? MoveGen::Right : MoveGen::Left][yourKingSq];

	return to2FromBb(rightDirectChecksToBb) | rightDirectChecksFromBb;
      }      
    };

    template <typename BoardT, ColorT Color, PawnPromoMoveDirT PawnPromoMoveDir>
    inline int countPromoCheckmates(const BoardT& board, const BitBoardT checksFromBb, const PromoPieceT promoPiece) {
      int checkmates = 0;

      // TODO TODO TODO
      // while(checksFromBb) {
      // 	const SquareT from = Bits::popLsb(checksFromBb);
      // 	const SquareT to = $%^#$%^To2FromFn::fn(to);

      // 	const BoardT newBoard = pushPawn<BoardT, Color, IsPushTwo>(board, from, to);

      // 	checkmates += !BoardUtils::hasLegalMoves<BoardT, OtherColorT<Color>::value>(newBoard);
      // }

      return checkmates;
    }
    
    template <typename BoardT, ColorT Color, PawnPromoMoveDirT PawnPromoMoveDir>
    void calculatePromoChecks(const BoardT& board, const BitBoardT pawnMovesBb, const BitBoardT discoveriesBb, const SquareT yourKingSq, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb, int& checks, int& discoverychecks, int& doublechecks, int& checkmates) {
      const BitBoardT movesFromBb = PawnPromoMoveDirFns<Color, PawnPromoMoveDir>::to2FromBb(pawnMovesBb);
      const BitBoardT movesDiscoveriesFromBb = movesFromBb & discoveriesBb;
      const int movesDiscoveriesCount = Bits::count(movesDiscoveriesFromBb);
      
      const BitBoardT orthogDirectChecksFromBb = PawnPromoMoveDirFns<Color, PawnPromoMoveDir>::orthogDirectChecksFromBbFn(pawnMovesBb, movesFromBb, yourKingSq, yourKingRookAttacksBb);
      const BitBoardT diagDirectChecksFromBb = PawnPromoMoveDirFns<Color, PawnPromoMoveDir>::diagDirectChecksFromBbFn(pawnMovesBb, movesFromBb, yourKingSq, yourKingBishopAttacksBb);
      const BitBoardT knightDirectChecksFromBb = PawnPromoMoveDirFns<Color, PawnPromoMoveDir>::to2FromBb(pawnMovesBb & MoveGen::KnightAttacks[yourKingSq]);
      
      const BitBoardT rookDoubleChecksFromBb = orthogDirectChecksFromBb & movesDiscoveriesFromBb;
      const BitBoardT bishopDoubleChecksFromBb = diagDirectChecksFromBb & movesDiscoveriesFromBb;
      const BitBoardT queenDoubleChecksFromBb = rookDoubleChecksFromBb | bishopDoubleChecksFromBb;
      const BitBoardT knightDoubleChecksFromBb = knightDirectChecksFromBb & movesDiscoveriesFromBb;
      
      const int rookDirectChecksCount = Bits::count(orthogDirectChecksFromBb);
      const int bishopDirectChecksCount = Bits::count(diagDirectChecksFromBb);
      const int queenDirectChecksCount = rookDirectChecksCount + bishopDirectChecksCount;
      const int knightDirectChecksCount = Bits::count(knightDirectChecksFromBb);
      
      const int rookDoubleChecksCount = Bits::count(rookDoubleChecksFromBb);
      const int bishopDoubleChecksCount = Bits::count(bishopDoubleChecksFromBb);
      const int queenDoubleChecksCount = Bits::count(queenDoubleChecksFromBb);
      const int knightDoubleChecksCount = Bits::count(knightDoubleChecksFromBb);
      
      const int rookDiscoveredChecksCount = movesDiscoveriesCount - rookDoubleChecksCount;
      const int bishopDiscoveredChecksCount = movesDiscoveriesCount - bishopDoubleChecksCount;
      const int queenDiscoveredChecksCount = movesDiscoveriesCount - queenDoubleChecksCount;
      const int knightDiscoveredChecksCount = movesDiscoveriesCount - knightDoubleChecksCount;
      
      const int directChecksCount = queenDirectChecksCount + knightDirectChecksCount + rookDirectChecksCount + bishopDirectChecksCount;
      const int discoveredChecksCount = queenDiscoveredChecksCount + knightDiscoveredChecksCount + rookDiscoveredChecksCount + bishopDiscoveredChecksCount;
      const int doubleChecksCount = queenDoubleChecksCount + knightDoubleChecksCount + rookDoubleChecksCount + bishopDoubleChecksCount;

      const int checksCount = directChecksCount + discoveredChecksCount;
      checks += checksCount;
      discoverychecks += discoveredChecksCount;
      doublechecks += doubleChecksCount;

      // The path less taken
      if(checksCount != 0) {
	const BitBoardT rookChecksFromBb = movesDiscoveriesFromBb | orthogDirectChecksFromBb;
	if(rookChecksFromBb != BbNone) {
	  checkmates += countPromoCheckmates<BoardT, Color, PawnPromoMoveDir>(board, rookChecksFromBb, PromoRook);
	}

	const BitBoardT bishopChecksFromBb = movesDiscoveriesFromBb | diagDirectChecksFromBb;
	if(bishopChecksFromBb != BbNone) {
	  // TODO
	}

	const BitBoardT queenChecksFromBb = rookChecksFromBb | bishopChecksFromBb;
	if(queenChecksFromBb != BbNone) {
	  // TODO
	}

	const BitBoardT knightChecksFromBb = movesDiscoveriesFromBb | knightDirectChecksFromBb;
	if(knightChecksFromBb != BbNone) {
	  // TODO
	}
      }
    }

    template <typename StateT, typename CountHandlerT, typename BoardT, ColorT Color>
    inline void handlePawnPromoMoves(const CountTag&, StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const MoveGen::PawnPushesAndCapturesT& pawnMoves, const BitBoardT directChecksBb, const BitBoardT pushDiscoveriesBb, const BitBoardT leftDiscoveriesBb, const BitBoardT rightDiscoveriesBb, const BitBoardT yourKingRookAttacksBb, const BitBoardT yourKingBishopAttacksBb) {
      const BitBoardT pawnPushesBb = pawnMoves.pushesOneBb & LastRankBbT<Color>::LastRankBb;
      const int pawnPushesCount = Bits::count(pawnPushesBb);

      const BitBoardT pawnCapturesLeftBb = pawnMoves.capturesLeftBb & LastRankBbT<Color>::LastRankBb;
      const int pawnCapturesLeftCount = Bits::count(pawnCapturesLeftBb);

      const BitBoardT pawnCapturesRightBb = pawnMoves.capturesRightBb & LastRankBbT<Color>::LastRankBb;
      const int pawnCapturesRightCount = Bits::count(pawnCapturesRightBb);

      const int captures = (pawnCapturesLeftCount + pawnCapturesRightCount)*4/*qnrb*/;
      const int nodes = pawnPushesCount*4/*qnrb*/ + captures;
      const int eps = 0;
      const int castles = 0;
      const int promos = nodes;
      
      int checks = 0;
      int discoverychecks = 0;
      int doublechecks = 0;
      int checkmates = 0;

      const SquareT yourKingSq = board.state[(size_t)OtherColorT<Color>::value].basic.pieceSquares[TheKing];
      
      // Push promos
      if(pawnPushesBb != BbNone) {
	calculatePromoChecks<BoardT, Color, PawnPromoPushOneDir>(board, pawnPushesBb, pushDiscoveriesBb, yourKingSq, yourKingRookAttacksBb, yourKingBishopAttacksBb, checks, discoverychecks, doublechecks, checkmates);
      }

      // Left captures
      if(pawnCapturesLeftBb != BbNone) {
	calculatePromoChecks<BoardT, Color, PawnPromoCaptureLeftDir>(board, pawnCapturesLeftBb, leftDiscoveriesBb, yourKingSq, yourKingRookAttacksBb, yourKingBishopAttacksBb, checks, discoverychecks, doublechecks, checkmates);
      }

      // Right captures
      if(pawnCapturesRightBb != BbNone) {
	calculatePromoChecks<BoardT, Color, PawnPromoCaptureRightDir>(board, pawnCapturesRightBb, rightDiscoveriesBb, yourKingSq, yourKingRookAttacksBb, yourKingBishopAttacksBb, checks, discoverychecks, doublechecks, checkmates);
      }

      CountHandlerT::handleCount(state, nodes, captures, eps, castles, promos, checks, discoverychecks, doublechecks, checkmates);
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

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, typename PieceMoveFn, MoveTypeT MoveType, PieceT Piece>
    inline void handlePieceMoves(StateT state, const BoardT& board, const typename PieceMoveFn::PieceMapImplT& yourPieceMap, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const bool isDiscoveredCheck) {
      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
	
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);

	const BoardT newBoard = PieceMoveFn::fn(board, yourPieceMap, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(MoveType, PieceTypeForPieceT<Piece>::value, from, to, isDirectCheck, isDiscoveredCheck));
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

      // (Non-promo-)piece captures of promo pieces
      typedef PieceMoveFn<FullBoardT, Color, Piece, PiecePromoCapture, ColorPieceMapT> PiecePromoCaptureFn;
      handlePieceMoves<StateT, PosHandlerT, FullBoardT, Color, PiecePromoCaptureFn, CaptureMove, Piece>(state, board, yourPieceMap, from, promoPieceCapturesBb, directChecksBb, isDiscoveredCheck);

      return pieceCapturesBb & ~promoPieceCapturesBb;
    }

    // Perform piece moves and send them to PosHandlerT
    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, PieceT Piece>
    inline void handlePieceMoves(const PosTag&, StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {
      typedef typename BoardT::ColorStateT ColorStateT;

      const ColorStateT& myState = board.state[(size_t)Color];
      const SquareT from = myState.basic.pieceSquares[Piece];
      const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;

      const BitBoardT piecePushesBb = movesBb & ~allYourPiecesBb;

      // (Non-promo-)piece pushes
      typedef PieceMoveFn<BoardT, Color, Piece, PiecePush, NoPieceMapT> PiecePushFn;
      handlePieceMoves<StateT, PosHandlerT, BoardT, Color, PiecePushFn, PushMove, Piece>(state, board, NoPieceMapT(), from, piecePushesBb, directChecksBb, isDiscoveredCheck);

      const BitBoardT pieceCapturesBb = movesBb & allYourPiecesBb;

      // (Non-promo-)piece captures of promo pieces
      const BitBoardT pieceNonPromoCapturesBb = handlePiecePromoCaptures<StateT, PosHandlerT, Color, Piece>(state, board, yourPieceMap, from, pieceCapturesBb, directChecksBb, isDiscoveredCheck);

      // (Non-promo-)piece captures of non-promo pieces
      typedef PieceMoveFn<BoardT, Color, Piece, PieceCapture, ColorPieceMapT> PieceCaptureFn;
      handlePieceMoves<StateT, PosHandlerT, BoardT, Color, PieceCaptureFn, CaptureMove, Piece>(state, board, yourPieceMap, from, pieceNonPromoCapturesBb, directChecksBb, isDiscoveredCheck);
    }

    template <typename BoardT, ColorT Color, typename PieceMoveFn>
    inline int countPieceMoveCheckmates(const BoardT& board, const typename PieceMoveFn::PieceMapImplT& yourPieceMap, const SquareT from, BitBoardT toBb) {
      int checkmates = 0;
      
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);

	const BoardT newBoard = PieceMoveFn::fn(board, yourPieceMap, from, to);

	checkmates += !BoardUtils::hasLegalMoves<BoardT, OtherColorT<Color>::value>(newBoard);
      }

      return checkmates;
    }
    
    template <ColorT Color, PieceT Piece>
    inline BitBoardT countPiecePromoCaptureCheckmates(const BasicBoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const BitBoardT pieceCapturesBb, int& checkmates) {
      // No promo pieces
      return pieceCapturesBb;
    }
    
    template <ColorT Color, PieceT Piece>
    inline BitBoardT countPiecePromoCaptureCheckmates(const FullBoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const BitBoardT pieceCapturesBb, int& checkmates) {
      const BitBoardT promoPieceCapturesBb = pieceCapturesBb & yourPieceMap.allPromoPiecesBb;

      // (Non-promo-)piece captures of promo pieces
      typedef PieceMoveFn<FullBoardT, Color, Piece, PiecePromoCapture, ColorPieceMapT> PiecePromoCaptureFn;
      checkmates += countPieceMoveCheckmates<FullBoardT, Color, PiecePromoCaptureFn>(board, yourPieceMap, from, promoPieceCapturesBb);

      return pieceCapturesBb & ~promoPieceCapturesBb;
    }

    // Count piece moves and send them to CountHandlerT
    template <typename StateT, typename CountHandlerT, typename BoardT, ColorT Color, PieceT Piece>
    inline void handlePieceMoves(const CountTag&, StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {
      typedef typename BoardT::ColorStateT ColorStateT;
      
      const int nodes = Bits::count(movesBb);

      const BitBoardT capturesBb = movesBb & allYourPiecesBb;
      const int captures = Bits::count(capturesBb);

      const int eps = 0;
      const int castles = 0;
      const int promos = 0;

      const BitBoardT directCheckMovesBb = movesBb & directChecksBb;
      const int directchecks = Bits::count(directCheckMovesBb);
      
      const ColorStateT& myState = board.state[(size_t)Color];
      const SquareT from = myState.basic.pieceSquares[Piece];
      const int discoveries = ((bbForSquare(from) & discoveriesBb) == BbNone) ? 0 : nodes;

      const int checks = discoveries == 0 ? directchecks : discoveries;
      const int doublechecks = discoveries == 0 ? 0 : directchecks;
      const int discoverychecks = discoveries - doublechecks;

      int checkmates = 0;

      if(checks != 0) {
	const BitBoardT checkMovesBb = discoveries == 0 ? directCheckMovesBb : movesBb;

	const BitBoardT checkPushesBb = checkMovesBb & ~allYourPiecesBb;
	// (Non-promo-)piece pushes
	typedef PieceMoveFn<BoardT, Color, Piece, PiecePush, NoPieceMapT> PiecePushFn;
	checkmates += countPieceMoveCheckmates<BoardT, Color, PiecePushFn>(board, NoPieceMapT(), from, checkPushesBb);
	
	const BitBoardT checkCapturesBb = checkMovesBb & allYourPiecesBb;
	
	// (Non-promo-)piece captures of promo pieces
	const BitBoardT checkNonPromoCapturesBb = countPiecePromoCaptureCheckmates<Color, Piece>(board, yourPieceMap, from, checkCapturesBb, checkmates);
	
	// (Non-promo-)piece captures of non-promo pieces
	typedef PieceMoveFn<BoardT, Color, Piece, PieceCapture, ColorPieceMapT> PieceCaptureFn;
	checkmates += countPieceMoveCheckmates<BoardT, Color, PieceCaptureFn>(board, yourPieceMap, from, checkNonPromoCapturesBb);
      }

      CountHandlerT::handleCount(state, nodes, captures, eps, castles, promos, checks, discoverychecks, doublechecks, checkmates);
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
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(MoveType, King, from, to, false/*isDirectCheck*/, isDiscoveredCheck));
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
    inline void handleKingMoves(const PosTag&, StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb, const SquareT yourKingSq) {
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
    
    template <typename BoardT, ColorT Color, typename KingMoveFn>
    inline int countKingMoveCheckmates(const BoardT& board, const typename KingMoveFn::PieceMapImplT& yourPieceMap, const SquareT from, BitBoardT toBb) {
      int checkmates = 0;
      
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);

	const BoardT newBoard = KingMoveFn::fn(board, yourPieceMap, from, to);

	checkmates += !BoardUtils::hasLegalMoves<BoardT, OtherColorT<Color>::value>(newBoard);
      }

      return checkmates;
    }
    
    template <ColorT Color>
    inline BitBoardT countKingPromoCaptureCheckmates(const BasicBoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const BitBoardT kingCapturesBb, int& checkmates) {
      // No promo pieces
      return kingCapturesBb;
    }
    
    template <ColorT Color>
    inline BitBoardT countKingPromoCaptureCheckmates(const FullBoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, const BitBoardT kingCapturesBb, int& checkmates) {
      const BitBoardT promoKingCapturesBb = kingCapturesBb & yourPieceMap.allPromoPiecesBb;

      // (Non-promo-)king captures of promo pieces
      typedef KingMoveFn<FullBoardT, Color, KingPromoCapture, ColorPieceMapT> KingPromoCaptureFn;
      checkmates += countKingMoveCheckmates<FullBoardT, Color, KingPromoCaptureFn>(board, yourPieceMap, from, promoKingCapturesBb);

      return kingCapturesBb & ~promoKingCapturesBb;
    }

    template <typename StateT, typename CountHandlerT, typename BoardT, ColorT Color>
    inline void handleKingMoves(const CountTag&, StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb, const SquareT yourKingSq) {
      const int nodes = Bits::count(movesBb);

      const BitBoardT capturesBb = movesBb & allYourPiecesBb;
      const int captures = Bits::count(capturesBb);

      const int eps = 0;
      const int castles = 0;
      const int promos = 0;

      typedef typename BoardT::ColorStateT ColorStateT;
      const ColorStateT& myState = board.state[(size_t)Color];
      
      const SquareT from = myState.basic.pieceSquares[TheKing];
      const BitBoardT fromBb = bbForSquare(from);
      
      const BitBoardT yourKingRaysBb = MoveGen::BishopRays[yourKingSq] | MoveGen::RookRays[yourKingSq];
      const BitBoardT checkMovesBb = movesBb & ~yourKingRaysBb;
      
      const int discoverychecks = (fromBb & discoveriesBb) == BbNone ? 0 : Bits::count(checkMovesBb);
      const int checks = discoverychecks;
      const int doublechecks = 0;
      
      int checkmates = 0;

      if(checks != 0) {
	const BitBoardT checkPushesBb = checkMovesBb & ~allYourPiecesBb;
	// (Non-promo-)king pushes
	typedef KingMoveFn<BoardT, Color, KingPush, NoPieceMapT> KingPushFn;
	checkmates += countKingMoveCheckmates<BoardT, Color, KingPushFn>(board, NoPieceMapT(), from, checkPushesBb);
	
	const BitBoardT checkCapturesBb = checkMovesBb & allYourPiecesBb;
	
	// King captures of promo pieces
	const BitBoardT checkNonPromoCapturesBb = countKingPromoCaptureCheckmates<Color>(board, yourPieceMap, from, checkCapturesBb, checkmates);
	
	// (Non-promo-)king captures of non-promo pieces
	typedef KingMoveFn<BoardT, Color, KingCapture, ColorPieceMapT> KingCaptureFn;
	checkmates += countKingMoveCheckmates<BoardT, Color, KingCaptureFn>(board, yourPieceMap, from, checkNonPromoCapturesBb);
      }
      
      CountHandlerT::handleCount(state, nodes, captures, eps, castles, promos, checks, discoverychecks, doublechecks, checkmates);
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
	
	ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(MoveType, PieceTypeForPromoPiece[promoPiece], from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void handlePromoPieceMoves(const PosTag&, StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const int promoIndex, const PromoPieceT promoPiece, const SquareT from, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {
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

    template <typename BoardT, ColorT Color, typename PromoPieceMoveFn>
    inline int countPromoPieceMoveCheckmates(const BoardT& board, const typename PromoPieceMoveFn::PieceMapImplT& yourPieceMap, const int promoIndex, const PromoPieceT promoPiece, const SquareT from, BitBoardT toBb) {
      int checkmates = 0;
      
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);

	const BoardT newBoard = PromoPieceMoveFn::fn(board, promoIndex, promoPiece, yourPieceMap, from, to);

	checkmates += !BoardUtils::hasLegalMoves<BoardT, OtherColorT<Color>::value>(newBoard);
      }

      return checkmates;
    }
    
    template <typename StateT, typename CountHandlerT, typename BoardT, ColorT Color>
    inline void handlePromoPieceMoves(const CountTag&, StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const int promoIndex, const PromoPieceT promoPiece, const SquareT from, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {
      const int nodes = Bits::count(movesBb);

      const BitBoardT capturesBb = movesBb & allYourPiecesBb;
      const int captures = Bits::count(capturesBb);

      const int eps = 0;
      const int castles = 0;
      const int promos = 0;

      const BitBoardT directCheckMovesBb = movesBb & directChecksBb;
      const int directchecks = Bits::count(directCheckMovesBb);
      
      const int discoveries = ((bbForSquare(from) & discoveriesBb) == BbNone) ? 0 : nodes;

      const int checks = discoveries == 0 ? directchecks : discoveries;
      const int doublechecks = discoveries == 0 ? 0 : directchecks;
      const int discoverychecks = discoveries - doublechecks;
      
      int checkmates = 0;

      if(checks != 0) {
	const BitBoardT checkMovesBb = discoveries == 0 ? directCheckMovesBb : movesBb;

	const BitBoardT checkPushesBb = checkMovesBb & ~allYourPiecesBb;
	
	// Promo-piece pushes
	typedef PromoPieceMoveFn<BoardT, Color, PromoPiecePush, NoPieceMapT> PromoPiecePushFn;
	checkmates += countPromoPieceMoveCheckmates<BoardT, Color, PromoPiecePushFn>(board, NoPieceMapT(), promoIndex, promoPiece, from, checkPushesBb);
	
	const BitBoardT checkCapturesBb = checkMovesBb & allYourPiecesBb;
	
	// Promo-piece captures of promo pieces
	const BitBoardT checkPromoCapturesBb = checkCapturesBb & yourPieceMap.allPromoPiecesBb;
	typedef PromoPieceMoveFn<BoardT, Color, PromoPiecePromoCapture, ColorPieceMapT> PromoPiecePromoCaptureFn;
	checkmates += countPromoPieceMoveCheckmates<BoardT, Color, PromoPiecePromoCaptureFn>(board, yourPieceMap, promoIndex, promoPiece, from, checkPromoCapturesBb);

	// Promo-piece captures of non-promo pieces
	const BitBoardT checkNonPromoCapturesBb = checkCapturesBb & ~checkPromoCapturesBb;
	typedef PromoPieceMoveFn<BoardT, Color, PromoPieceCapture, ColorPieceMapT> PromoPieceCaptureFn;
	checkmates += countPromoPieceMoveCheckmates<BoardT, Color, PromoPieceCaptureFn>(board, yourPieceMap, promoIndex, promoPiece, from, checkNonPromoCapturesBb);
      }
      
      CountHandlerT::handleCount(state, nodes, captures, eps, castles, promos, checks, discoverychecks, doublechecks, checkmates);
    }
    
    //
    // Castling moves
    //

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color, CastlingRightsT CastlingRight>
    inline void handleCastlingMove(const PosTag&, StateT state, const BoardT& board, const bool isDiscoveredCheck) {
      const BoardT newBoard1 = pushPiece<BoardT, Color, TheKing>(board, MoveGen::CastlingTraitsT<Color, CastlingRight>::KingFrom, MoveGen::CastlingTraitsT<Color, CastlingRight>::KingTo);
      const BoardT newBoard = pushPiece<BoardT, Color, MoveGen::CastlingTraitsT<Color, CastlingRight>::TheRook>(newBoard1, MoveGen::CastlingTraitsT<Color, CastlingRight>::RookFrom, MoveGen::CastlingTraitsT<Color, CastlingRight>::RookTo);

      typedef typename PosHandlerT::ReverseT ReversePosHandlerT;
 
      // We use the king (from and) to square by convention
      ReversePosHandlerT::handlePos(state, newBoard, MoveInfoT(CastlingMove, King, MoveGen::CastlingTraitsT<Color, CastlingRight>::KingFrom, MoveGen::CastlingTraitsT<Color, CastlingRight>::KingTo, /*isDirectCheck*/false, isDiscoveredCheck));
    }

    template <typename StateT, typename CountHandlerT, typename BoardT, ColorT Color, CastlingRightsT CastlingRight>
    inline void handleCastlingMove(const CountTag&, StateT state, const BoardT& board, const bool isDiscoveredCheck) {
      const int nodes = 1;
      const int captures = 0;
      const int eps = 0;
      const int castles = 1;
      const int promos = 0;
      const int checks = (int)isDiscoveredCheck;
      const int discoverychecks = checks;
      const int doublechecks = 0;
      int checkmates = 0;
      
      if(isDiscoveredCheck) {
	const BoardT newBoard1 = pushPiece<BoardT, Color, TheKing>(board, MoveGen::CastlingTraitsT<Color, CastlingRight>::KingFrom, MoveGen::CastlingTraitsT<Color, CastlingRight>::KingTo);
	const BoardT newBoard = pushPiece<BoardT, Color, MoveGen::CastlingTraitsT<Color, CastlingRight>::TheRook>(newBoard1, MoveGen::CastlingTraitsT<Color, CastlingRight>::RookFrom, MoveGen::CastlingTraitsT<Color, CastlingRight>::RookTo);

	// It's checkmate if there are no legal moves
	checkmates = (int) !BoardUtils::hasLegalMoves<BoardT, OtherColorT<Color>::value>(newBoard);
      }
      
      CountHandlerT::handleCount(state, nodes, captures, eps, castles, promos, checks, discoverychecks, doublechecks, checkmates);
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

    template <typename StateT, typename PosOrCountTag, typename PosOrCountHandlerT, ColorT Color>
    inline void handleLegalPromoPieceMoves(StateT state, const BasicBoardT& board, const typename MoveGen::LegalMovesImplType<BasicBoardT>::LegalMovesT& legalMoves, const ColorPieceMapT& yourPieceMap, const BitBoardT allYourPiecesBb) {
      // No promo pieces
    }
    
    template <typename StateT, typename PosOrCountTag, typename PosOrCountHandlerT, ColorT Color>
    inline void handleLegalPromoPieceMoves(StateT state, const FullBoardT& board, const typename MoveGen::LegalMovesImplType<FullBoardT>::LegalMovesT& legalMoves, const ColorPieceMapT& yourPieceMap, const BitBoardT allYourPiecesBb) {
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
	
	handlePromoPieceMoves<StateT, PosOrCountHandlerT, FullBoardT, Color>(PosOrCountTag(), state, board, yourPieceMap, promoIndex, promoPiece, promoPieceSq, legalMoves.promoPieceMoves[promoIndex], directChecksBb, discoveriesBb, allYourPiecesBb); 
      }
    }
    
    template <typename StateT, typename PosOrCountTag, typename PosOrCountHandlerT, typename BoardT, ColorT Color>
    inline void handleAllLegalMoves(StateT state, const BoardT& board) {
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
	handlePawnNonPromoMoves<StateT, PosOrCountHandlerT, BoardT, Color>(PosOrCountTag(), state, board, yourPieceMap, legalMoves.pawnMoves, legalMoves.directChecks.pawnChecksBb, legalMoves.discoveredChecks.pawnPushDiscoveryMasksBb, legalMoves.discoveredChecks.pawnLeftDiscoveryMasksBb, legalMoves.discoveredChecks.pawnRightDiscoveryMasksBb, legalMoves.discoveredChecks.isLeftEpDiscovery, legalMoves.discoveredChecks.isRightEpDiscovery);

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

	  handlePawnPromoMoves<StateT, PosOrCountHandlerT, BoardT, Color>(PosOrCountTag(), state, board, yourPieceMap, legalMoves.pawnMoves, legalMoves.directChecks.pawnChecksBb, legalMoves.discoveredChecks.pawnPushDiscoveryMasksBb, legalMoves.discoveredChecks.pawnLeftDiscoveryMasksBb, legalMoves.discoveredChecks.pawnRightDiscoveryMasksBb, yourKingRookAttacksBb, yourKingBishopAttacksBb);
	}
	
	// Knights
	handlePieceMoves<StateT, PosOrCountHandlerT, BoardT, Color, Knight1>(PosOrCountTag(), state, board, yourPieceMap, legalMoves.pieceMoves[Knight1], legalMoves.directChecks.knightChecksBb, (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb);
	handlePieceMoves<StateT, PosOrCountHandlerT, BoardT, Color, Knight2>(PosOrCountTag(), state, board, yourPieceMap, legalMoves.pieceMoves[Knight2], legalMoves.directChecks.knightChecksBb, (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb);
	
	// Bishops
	handlePieceMoves<StateT, PosOrCountHandlerT, BoardT, Color, Bishop1>(PosOrCountTag(), state, board, yourPieceMap, legalMoves.pieceMoves[Bishop1], legalMoves.directChecks.bishopChecksBb, legalMoves.discoveredChecks.orthogDiscoveryPiecesBb, allYourPiecesBb); 
	handlePieceMoves<StateT, PosOrCountHandlerT, BoardT, Color, Bishop2>(PosOrCountTag(), state, board, yourPieceMap, legalMoves.pieceMoves[Bishop2], legalMoves.directChecks.bishopChecksBb, legalMoves.discoveredChecks.orthogDiscoveryPiecesBb, allYourPiecesBb); 

	// Rooks
	handlePieceMoves<StateT, PosOrCountHandlerT, BoardT, Color, Rook1>(PosOrCountTag(), state, board, yourPieceMap, legalMoves.pieceMoves[Rook1], legalMoves.directChecks.rookChecksBb, legalMoves.discoveredChecks.diagDiscoveryPiecesBb, allYourPiecesBb); 
	handlePieceMoves<StateT, PosOrCountHandlerT, BoardT, Color, Rook2>(PosOrCountTag(), state, board, yourPieceMap, legalMoves.pieceMoves[Rook2], legalMoves.directChecks.rookChecksBb, legalMoves.discoveredChecks.diagDiscoveryPiecesBb, allYourPiecesBb); 

	// Queen
	handlePieceMoves<StateT, PosOrCountHandlerT, BoardT, Color, TheQueen>(PosOrCountTag(), state, board, yourPieceMap, legalMoves.pieceMoves[TheQueen], (legalMoves.directChecks.bishopChecksBb | legalMoves.directChecks.rookChecksBb), /*discoveriesBb*/BbNone, allYourPiecesBb); 

	// Promo pieces
	handleLegalPromoPieceMoves<StateT, PosOrCountTag, PosOrCountHandlerT, Color>(state, board, legalMoves, yourPieceMap, allYourPiecesBb);
	
	// Castling
	CastlingRightsT canCastleFlags = legalMoves.canCastleFlags;
	if(canCastleFlags) {
	  if((canCastleFlags & CanCastleKingside)) {
	    handleCastlingMove<StateT, PosOrCountHandlerT, BoardT, Color, CanCastleKingside>(PosOrCountTag(), state, board, legalMoves.discoveredChecks.isKingsideCastlingDiscovery);
	  }	

	  if((canCastleFlags & CanCastleQueenside)) {
	    handleCastlingMove<StateT, PosOrCountHandlerT, BoardT, Color, CanCastleQueenside>(PosOrCountTag(), state, board, legalMoves.discoveredChecks.isQueensideCastlingDiscovery);
	  }
	}

      } // nChecks < 2
      
      // King - discoveries from king moves are a pain in the butt because each move direction is potentially different.
      handleKingMoves<StateT, PosOrCountHandlerT, BoardT, Color>(PosOrCountTag(), state, board, yourPieceMap, legalMoves.pieceMoves[TheKing], (legalMoves.discoveredChecks.diagDiscoveryPiecesBb | legalMoves.discoveredChecks.orthogDiscoveryPiecesBb), allYourPiecesBb, yourState.basic.pieceSquares[TheKing]); 
    }

    //
    // Consumer of makeAllLegalMoves must provide a position handler type that provides a color-reversed pos handler type,
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

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void makeAllLegalMoves(StateT state, const BoardT& board) {
      handleAllLegalMoves<StateT, PosTag, PosHandlerT, BoardT, Color>(state, board);
    }
    
    //
    // Consumer of countAllLegalMoves must provide a count handler type that provides a color-reversed pos handler type,
    //   and a pos handler type that does promotion/demotion between board representatives.
    //
    // e.g.
    //
    // typedef blah MyStateT;
    //
    // template <typename BoardT, Color>
    // struct MyCountHandlerT {
    //
    //   typedef MyCountHandlerT<BoardT, OtherColorT<Color>::value> ReverseT;
    //   typedef MyCountHandlerT<typename BoardType<BoardT>::WithPromosT, Color> WithPromosT;
    //   typedef MyCountHandlerT<typename BoardType<BoardT>::WithoutPromosT, Color> WithoutPromosT;
    //
    //   static void handleCount(const MyStateT state, u64 nodes, ...) { ... }
    //
    // };

    template <typename StateT, typename PosHandlerT, typename BoardT, ColorT Color>
    inline void countAllLegalMoves(StateT state, const BoardT& board) {
      handleAllLegalMoves<StateT, CountTag, PosHandlerT, BoardT, Color>(state, board);
    }
    
  } // namespace MakeMove
} // namespace Chess

#endif //ndef MAKE_MOVE_HPP
