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

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn>
    inline void handlePawnsPushToPromo(StateT state, const BoardT& board, BitBoardT pawnsPushBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {

      while(pawnsPushBb) {
	const SquareT to = Bits::popLsb(pawnsPushBb);
	const SquareT from = To2FromFn::fn(to);

	const bool isDirectCheck = false; // TODO!!! (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	const BoardT queenPromoBoard = pushPawnToPromo<BoardTraitsT::Color>(board, from, to, PromoQueen);
	PosHandlerT::handlePos(state, queenPromoBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT knightPromoBoard = pushPawnToPromo<BoardTraitsT::Color>(board, from, to, PromoKnight);
	PosHandlerT::handlePos(state, knightPromoBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT rookPromoBoard = pushPawnToPromo<BoardTraitsT::Color>(board, from, to, PromoRook);
	PosHandlerT::handlePos(state, rookPromoBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT bishopPromoBoard = pushPawnToPromo<BoardTraitsT::Color>(board, from, to, PromoBishop);
	PosHandlerT::handlePos(state, bishopPromoBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck, /*isPromo*/true));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn, bool IsPushTwo>
    inline void handlePawnsPush(StateT state, const BoardT& board, BitBoardT pawnsPushBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {

      // Can't reach promotion on the first move
      if(!IsPushTwo) {
	const BitBoardT pawnsPushToPromoBb = pawnsPushBb & (BoardTraitsT::Color == White ? Rank8 : Rank1);
	handlePawnsPushToPromo<StateT, PosHandlerT, BoardTraitsT, To2FromFn>(state, board, pawnsPushToPromoBb, directChecksBb, discoveriesBb);
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

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnsPushOne(StateT state, const BoardT& board, BitBoardT pawnsPushOneBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      handlePawnsPush<StateT, PosHandlerT, BoardTraitsT, PawnPushOneTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/false>(state, board, pawnsPushOneBb, directChecksBb, discoveriesBb);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnsPushTwo(StateT state, const BoardT& board, BitBoardT pawnsPushTwoBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      handlePawnsPush<StateT, PosHandlerT, BoardTraitsT, PawnPushTwoTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/true>(state, board, pawnsPushTwoBb, directChecksBb, discoveriesBb);
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
    inline void handlePawnsCaptureToPromoOfPromoPiece(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      // TODO properly with promos
      while(pawnsCaptureBb) {
	const SquareT to = Bits::popLsb(pawnsCaptureBb);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = capturePromoPieceWithPawn<BoardTraitsT::Color>(board, yourPieceMap, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(CaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn>
    inline void handlePawnsCaptureOfPromoPiece(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {

      const BitBoardT pawnsCaptureToPromoBb = pawnsCaptureBb & (BoardTraitsT::Color == White ? Rank8 : Rank1);
      handlePawnsCaptureToPromoOfPromoPiece<StateT, PosHandlerT, BoardTraitsT, To2FromFn>(state, board, yourPieceMap, pawnsCaptureToPromoBb, directChecksBb, discoveriesBb);
      pawnsCaptureBb &= ~pawnsCaptureToPromoBb;
	
      while(pawnsCaptureBb) {
	const SquareT to = Bits::popLsb(pawnsCaptureBb);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = capturePromoPieceWithPawn<BoardTraitsT::Color>(board, yourPieceMap, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(CaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn>
    inline void handlePawnsCaptureToPromo(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      
      while(pawnsCaptureBb) {
	const SquareT to = Bits::popLsb(pawnsCaptureBb);
	const SquareT from = To2FromFn::fn(to);

	const bool isDirectCheck = false; // TODO!!! (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	const BoardT queenPromoBoard = captureWithPawnToPromo<BoardTraitsT::Color>(board, yourPieceMap, from, to, PromoQueen);
	PosHandlerT::handlePos(state, queenPromoBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT knightPromoBoard = captureWithPawnToPromo<BoardTraitsT::Color>(board, yourPieceMap, from, to, PromoKnight);
	PosHandlerT::handlePos(state, knightPromoBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT rookPromoBoard = captureWithPawnToPromo<BoardTraitsT::Color>(board, yourPieceMap, from, to, PromoRook);
	PosHandlerT::handlePos(state, rookPromoBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck, /*isPromo*/true));
	
	const BoardT bishopPromoBoard = captureWithPawnToPromo<BoardTraitsT::Color>(board, yourPieceMap, from, to, PromoBishop);
	PosHandlerT::handlePos(state, bishopPromoBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck, /*isPromo*/true));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, typename To2FromFn>
    inline void handlePawnsCapture(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      
      const BitBoardT promoPieceCaptureBb = pawnsCaptureBb & yourPieceMap.allPromoPiecesBb;
      handlePawnsCaptureOfPromoPiece<StateT, PosHandlerT, BoardTraitsT, To2FromFn>(state, board, yourPieceMap, promoPieceCaptureBb, directChecksBb, discoveriesBb);
      pawnsCaptureBb &= ~promoPieceCaptureBb;

      const BitBoardT pawnsCaptureToPromoBb = pawnsCaptureBb & (BoardTraitsT::Color == White ? Rank8 : Rank1);
      handlePawnsCaptureToPromo<StateT, PosHandlerT, BoardTraitsT, To2FromFn>(state, board, yourPieceMap, pawnsCaptureToPromoBb, directChecksBb, discoveriesBb);
      pawnsCaptureBb &= ~pawnsCaptureToPromoBb;
	
      while(pawnsCaptureBb) {
	const SquareT to = Bits::popLsb(pawnsCaptureBb);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = captureWithPawn<BoardTraitsT::Color>(board, yourPieceMap, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(CaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnsCaptureLeft(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureLeft, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      handlePawnsCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(state, board, yourPieceMap, pawnsCaptureLeft, directChecksBb, discoveriesBb);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnsCaptureRight(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, BitBoardT pawnsCaptureRight, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      handlePawnsCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(state, board, yourPieceMap, pawnsCaptureRight, directChecksBb, discoveriesBb);
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
    inline void handlePawnEpCaptureLeft(StateT state, const BoardT& board, BitBoardT pawnsCaptureLeft, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const bool isEpDiscovery) {
      handlePawnEpCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(state, board, pawnsCaptureLeft, directChecksBb, discoveriesBb, isEpDiscovery);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnEpCaptureRight(StateT state, const BoardT& board, BitBoardT pawnsCaptureRight, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const bool isEpDiscovery) {
      handlePawnEpCapture<StateT, PosHandlerT, BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(state, board, pawnsCaptureRight, directChecksBb, discoveriesBb, isEpDiscovery);
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePawnMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const PawnPushesAndCapturesT& pawnMoves, const BitBoardT directChecksBb, const BitBoardT pushDiscoveriesBb, const BitBoardT leftDiscoveriesBb, const BitBoardT rightDiscoveriesBb, const bool isLeftEpDiscovery, const bool isRightEpDiscovery) {

      // Pawn pushes
      handlePawnsPushOne<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.pushesOneBb, directChecksBb, pushDiscoveriesBb);
      handlePawnsPushTwo<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.pushesTwoBb, directChecksBb, pushDiscoveriesBb);
	
      // Pawn captures
      handlePawnsCaptureLeft<StateT, PosHandlerT, BoardTraitsT>(state, board, yourPieceMap, pawnMoves.capturesLeftBb, directChecksBb, leftDiscoveriesBb);
      handlePawnsCaptureRight<StateT, PosHandlerT, BoardTraitsT>(state, board, yourPieceMap, pawnMoves.capturesRightBb, directChecksBb, rightDiscoveriesBb);
      
      // Pawn en-passant captures
      handlePawnEpCaptureLeft<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.epCaptures.epLeftCaptureBb, directChecksBb, leftDiscoveriesBb, isLeftEpDiscovery);
      handlePawnEpCaptureRight<StateT, PosHandlerT, BoardTraitsT>(state, board, pawnMoves.epCaptures.epRightCaptureBb, directChecksBb, rightDiscoveriesBb, isRightEpDiscovery);
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, PieceT Piece>
    inline void handlePiecePushes(StateT state, const BoardT& board, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = pushPiece<BoardTraitsT::Color, Piece>(board, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, PieceT Piece>
    inline void handlePieceCapturesOfPromoPieces(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = capturePromoPieceWithPiece<BoardTraitsT::Color, Piece>(board, yourPieceMap, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(CaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, PieceT Piece>
    inline void handlePieceCaptures(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {

      const BitBoardT promoPieceCaptureBb = toBb & yourPieceMap.allPromoPiecesBb;
      handlePieceCapturesOfPromoPieces<StateT, PosHandlerT, BoardTraitsT, Piece>(state, board, yourPieceMap, from, promoPieceCaptureBb, directChecksBb, discoveriesBb);
      toBb &= ~promoPieceCaptureBb;

      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = captureWithPiece<BoardTraitsT::Color, Piece>(board, yourPieceMap, from, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(CaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT, PieceT Piece>
    inline void handlePieceMoves(StateT state, const BoardT& board, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {
      const ColorStateT& myState = board.pieces[(size_t)BoardTraitsT::Color];
      const SquareT from = myState.pieceSquares[Piece];

      handlePiecePushes<StateT, PosHandlerT, BoardTraitsT, Piece>(state, board, from, movesBb & ~allYourPiecesBb, directChecksBb, discoveriesBb);
      
      handlePieceCaptures<StateT, PosHandlerT, BoardTraitsT, Piece>(state, board, yourPieceMap, from, movesBb & allYourPiecesBb, directChecksBb, discoveriesBb);
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePromoPiecePushes(StateT state, const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = pushPromoPiece<BoardTraitsT::Color>(board, promoIndex, promoPiece, to);

	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(PushMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }
    
    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePromoPieceCapturesOfPromoPieces(StateT state, const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const ColorPieceMapT& yourPieceMap, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = capturePromoPieceWithPromoPiece<BoardTraitsT::Color>(board, promoIndex, promoPiece, yourPieceMap, from, to);
	
	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(CaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePromoPieceCaptures(StateT state, const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const ColorPieceMapT& yourPieceMap, const SquareT from, BitBoardT toBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb) {

      const BitBoardT promoPieceCaptureBb = toBb & yourPieceMap.allPromoPiecesBb;
      handlePromoPieceCapturesOfPromoPieces<StateT, PosHandlerT, BoardTraitsT>(state, board, promoIndex, promoPiece, yourPieceMap, from, promoPieceCaptureBb, directChecksBb, discoveriesBb);
      toBb &= ~promoPieceCaptureBb;
      
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);
	
	const BoardT newBoard = captureWithPromoPiece<BoardTraitsT::Color>(board, promoIndex, promoPiece, yourPieceMap, from, to);
	
	const bool isDirectCheck = (bbForSquare(to) & directChecksBb) != BbNone;
	const bool isDiscoveredCheck = (bbForSquare(from) & discoveriesBb) != BbNone;
	
	PosHandlerT::handlePos(state, newBoard, MoveInfoT(CaptureMove, from, to, isDirectCheck, isDiscoveredCheck));
      }
    }

    template <typename StateT, typename PosHandlerT, typename BoardTraitsT>
    inline void handlePromoPieceMoves(StateT state, const BoardT& board, const int promoIndex, const PromoPieceT promoPiece, const SquareT from, const ColorPieceMapT& yourPieceMap, const BitBoardT movesBb, const BitBoardT directChecksBb, const BitBoardT discoveriesBb, const BitBoardT allYourPiecesBb) {

      handlePromoPiecePushes<StateT, PosHandlerT, BoardTraitsT>(state, board, promoIndex, promoPiece, from, movesBb & ~allYourPiecesBb, directChecksBb, discoveriesBb);
      
      handlePromoPieceCaptures<StateT, PosHandlerT, BoardTraitsT>(state, board, promoIndex, promoPiece, yourPieceMap, from, movesBb & allYourPiecesBb, directChecksBb, discoveriesBb);
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
	handlePawnMoves<StateT, PosHandlerT, BoardTraitsT>(state, board, yourPieceMap, legalMoves.pawnMoves, legalMoves.directChecks.pawnChecksBb, legalMoves.discoveredChecks.pawnPushDiscoveryMasksBb, legalMoves.discoveredChecks.pawnLeftDiscoveryMasksBb, legalMoves.discoveredChecks.pawnRightDiscoveryMasksBb, legalMoves.discoveredChecks.isLeftEpDiscovery, legalMoves.discoveredChecks.isRightEpDiscovery);
	
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
