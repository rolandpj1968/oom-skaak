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
      // Note that 'discoverychecks' here includes checks delivered by castling, which is contraversial.
      // According to stats from: https://www.chessprogramming.org/Perft_Results
      // Would probably be better to separate this out into real discoveries and checks-from-castling
      u64 discoverychecks;
      u64 doublechecks;
      u64 checkmates;
      u64 invalids;
      u64 invalidsnon0;
      u64 non0inpinpath;
      u64 non0withdiagpins;
      u64 non0withorthogpins;
      u64 l0nondiscoveries;
      u64 l0checkertakable;
      u64 l0checkkingcanmove;
    };

    struct PiecePinMasksT {
      // Pawn pin masks - single bit board for all pawns for each move type.
      BitBoardT pawnsLeftPinMask;
      BitBoardT pawnsRightPinMask;
      BitBoardT pawnsPushOnePinMask;
      BitBoardT pawnsPushTwoPinMask;

      // Per-piece pin masks
      BitBoardT piecePinMasks[NSpecificPieceTypes];
      
      // Uncommon promo piece moves - one for each pawn - one for each promo piece except 2nd queen.
      // BitBoardT promoPiecePinMasks[NPawns];
      
      // BitBoardT allPinMasks;
    };

    struct PushesAndCapturesT {
      BitBoardT pushesBb;
      BitBoardT capturesBb;

      PushesAndCapturesT():
	pushesBb(BbNone), capturesBb(BbNone) {}
      
      PushesAndCapturesT(const BitBoardT pushesBb, const BitBoardT capturesBb):
	pushesBb(pushesBb), capturesBb(capturesBb) {}
    };

    struct EpPawnCapturesT {
      BitBoardT epLeftCaptureBb;
      BitBoardT epRightCaptureBb;

      EpPawnCapturesT():
	epLeftCaptureBb(BbNone), epRightCaptureBb(BbNone) {}

      EpPawnCapturesT(const BitBoardT epLeftCaptureBb, const BitBoardT epRightCaptureBb):
	epLeftCaptureBb(epLeftCaptureBb), epRightCaptureBb(epRightCaptureBb) {}
    };

    struct PawnPushesAndCapturesT {
      BitBoardT pushesOneBb;
      BitBoardT pushesTwoBb;
      BitBoardT capturesLeftBb;
      BitBoardT capturesRightBb;
      EpPawnCapturesT epCaptures;

      PawnPushesAndCapturesT():
	pushesOneBb(0), pushesTwoBb(0), capturesLeftBb(0), capturesRightBb(0), epCaptures() {}
      
      PawnPushesAndCapturesT(const BitBoardT pushesOneBb, const BitBoardT pushesTwoBb, const BitBoardT capturesLeftBb, const BitBoardT capturesRightBb, const EpPawnCapturesT epCaptures):
	pushesOneBb(pushesOneBb), pushesTwoBb(pushesTwoBb), capturesLeftBb(capturesLeftBb), capturesRightBb(capturesRightBb), epCaptures(epCaptures) {}
    };

    struct LegalMovesT {
      bool isIllegalPos; // true iff opposition king is (already) in check
      int nChecks; // side-channel info - if nChecks >= 2 then there are only king moves
      CastlingRightsT canCastleFlags; // note not actually 'rights' per se but actually legal moves
      PawnPushesAndCapturesT pawnMoves;
      PushesAndCapturesT specificPieceMoves[NSpecificPieceTypes];

      LegalMovesT():
	isIllegalPos(false), nChecks(0), canCastleFlags(NoCastlingRights), pawnMoves()/*, specificPieceMoves???*/ {}
    };

#include <boost/preprocessor/iteration/local.hpp>
    const u8 QueensideCastleSpaceBits = 0x07;
    const u8 KingsideCastleSpaceBits = 0x30;

    // Fast lookup for castling potential.
    // Lookup is on castling rights bitmap and on backrank occupancy from B to G files.
    // Result is bitmap of castling rights that have space between king and rook.
    const CastlingRightsT CastlingRightsWithSpace[4][64] = {
      // castlingRights == NoCastlingRights
      {
#define BOOST_PP_LOCAL_MACRO(n) \
	NoCastlingRights,
#define BOOST_PP_LOCAL_LIMITS (0, 63)
#include BOOST_PP_LOCAL_ITERATE()
      },
      // castlingRights == CanCastleQueenside
      {
#define BOOST_PP_LOCAL_MACRO(n) \
	((n) & QueensideCastleSpaceBits) == 0x0 ? CanCastleQueenside : NoCastlingRights,
#define BOOST_PP_LOCAL_LIMITS (0, 63)
#include BOOST_PP_LOCAL_ITERATE()
      },
      // castlingRights == CanCastleKingside
      {
#define BOOST_PP_LOCAL_MACRO(n) \
	((n) & KingsideCastleSpaceBits) == 0x0 ? CanCastleKingside : NoCastlingRights,
#define BOOST_PP_LOCAL_LIMITS (0, 63)
#include BOOST_PP_LOCAL_ITERATE()
      },
      // castlingRights == CanCastleQueenside | CanCastleKingside
      {
#define BOOST_PP_LOCAL_MACRO(n) \
	(CastlingRightsT)((((n) & QueensideCastleSpaceBits) == 0x0 ? CanCastleQueenside : NoCastlingRights) | (((n) & KingsideCastleSpaceBits) == 0x0 ? CanCastleKingside : NoCastlingRights)),
#define BOOST_PP_LOCAL_LIMITS (0, 63)
#include BOOST_PP_LOCAL_ITERATE()
      },
    };

    template <ColorT Color> struct CastlingSpaceTraitsT;

    template <> struct CastlingSpaceTraitsT<White> {
      static const SquareT backrankShift = B1;
    };
    
    template <> struct CastlingSpaceTraitsT<Black> {
      static const SquareT backrankShift = B8;
    };

    template <ColorT Color> inline CastlingRightsT castlingRightsWithSpace(CastlingRightsT castlingRights, BitBoardT allPieces) {
      return CastlingRightsWithSpace[castlingRights][(allPieces >> CastlingSpaceTraitsT<Color>::backrankShift) & 0x3f];
    }
    
    template <ColorT Color, CastlingRightsT CastlingRights> struct CastlingTraitsT;

    template <> struct CastlingTraitsT<White, CanCastleQueenside> {
      // B1, C1 and D1 must be open to castle queenside
      static const BitBoardT CastlingOpenBbMask = (BbOne << B1) | (BbOne << C1) | (BbOne << D1);
      // C1, D1, E1 must not be under attack in order to castle queenside
      static const BitBoardT CastlingThruCheckBbMask = (BbOne << C1) | (BbOne << D1) | (BbOne << E1);

      // King move
      static const SquareT KingFrom = E1;
      static const SquareT KingTo = C1;

      // Rook move
      static const SpecificPieceT SpecificRook = QueenRook;
      static const SquareT RookFrom = A1;
      static const SquareT RookTo = D1;
    };

    template <> struct CastlingTraitsT<White, CanCastleKingside> {
      // F1 and G1 must be open to castle kingside
      static const BitBoardT CastlingOpenBbMask = (BbOne << F1) | (BbOne << G1);
      // E1, F1 and G1 must not be under attack in order to castle kingside
      static const BitBoardT CastlingThruCheckBbMask = (BbOne << E1) | (BbOne << F1) | (BbOne << G1);

      // King move
      static const SquareT KingFrom = E1;
      static const SquareT KingTo = G1;

      // Rook move
      static const SpecificPieceT SpecificRook = KingRook;
      static const SquareT RookFrom = H1;
      static const SquareT RookTo = F1;
    };

    template <> struct CastlingTraitsT<Black, CanCastleQueenside> {
      // B8, C8 and D8 must be open to castle queenside
      static const BitBoardT CastlingOpenBbMask = (BbOne << B8) | (BbOne << C8) | (BbOne << D8);
      // C8, D8, E8 must not be under attack in order to castle queenside
      static const BitBoardT CastlingThruCheckBbMask = (BbOne << C8) | (BbOne << D8) | (BbOne << E8);

      // King move
      static const SquareT KingFrom = E8;
      static const SquareT KingTo = C8;

      // Rook move
      static const SpecificPieceT SpecificRook = QueenRook;
      static const SquareT RookFrom = A8;
      static const SquareT RookTo = D8;
    };

    template <> struct CastlingTraitsT<Black, CanCastleKingside> {
      // F8 and G8 must be open to castle kingside
      static const BitBoardT CastlingOpenBbMask = (BbOne << F8) | (BbOne << G8);
      // E8, F8 and G8 must not be under attack in order to castle kingside
      static const BitBoardT CastlingThruCheckBbMask = (BbOne << E8) | (BbOne << F8) | (BbOne << G8);

      // King move
      static const SquareT KingFrom = E8;
      static const SquareT KingTo = G8;

      // Rook move
      static const SpecificPieceT SpecificRook = KingRook;
      static const SquareT RookFrom = H8;
      static const SquareT RookTo = F8;
    };

    template <typename BoardTraitsT>
    inline PerftStatsT perft(const BoardT& board, const int depthToGo);
    
    template <typename BoardTraitsT>
    inline void perftImpl(PerftStatsT& stats, const BoardT& board, const int depthToGo, const MoveInfoT moveInfo);
  
    template <ColorT Color>
    SquareT pawnPushOneTo2From(const SquareT square);
    template <> SquareT pawnPushOneTo2From<White>(const SquareT square) { return square - 8; }
    template <> SquareT pawnPushOneTo2From<Black>(const SquareT square) { return square + 8; }

    template <ColorT Color>
    struct PawnPushOneTo2FromFn {
      static SquareT fn(const SquareT from) { return pawnPushOneTo2From<Color>(from); }
    };

    template <ColorT Color>
    SquareT pawnPushTwoTo2From(const SquareT square);
    template <> SquareT pawnPushTwoTo2From<White>(const SquareT square) { return square - 16; }
    template <> SquareT pawnPushTwoTo2From<Black>(const SquareT square) { return square + 16; }

    template <ColorT Color>
    struct PawnPushTwoTo2FromFn {
      static SquareT fn(const SquareT from) { return pawnPushTwoTo2From<Color>(from); }
    };

    template <typename BoardTraitsT, typename To2FromFn, bool IsPushTwo>
    inline void perftImplPawnsPush(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPush) {
      // No captures here - these are just pawn pushes and already filtered from all target clashes.
      while(pawnsPush) {
	const SquareT to = Bits::popLsb(pawnsPush);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = moveSpecificPiece<BoardTraitsT::Color, SpecificPawn, Push, IsPushTwo>(board, from, to);

	perftImpl<typename BoardTraitsT::ReverseT>(stats, newBoard, depthToGo-1, MoveInfoT(PushMove, to));
      }
    }

    template <typename BoardTraitsT>
    inline void perftImplPawnsPushOne(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPushOne) {
      perftImplPawnsPush<BoardTraitsT, PawnPushOneTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/false>(stats, board, depthToGo, pawnsPushOne);
    }
    
    template <typename BoardTraitsT>
    inline void perftImplPawnsPushTwo(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPushTwo) {
      perftImplPawnsPush<BoardTraitsT, PawnPushTwoTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/true>(stats, board, depthToGo, pawnsPushTwo);
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

    template <typename BoardTraitsT, typename To2FromFn>
    inline void perftImplPawnsCapture(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCapture) {
      while(pawnsCapture) {
	const SquareT to = Bits::popLsb(pawnsCapture);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = moveSpecificPiece<BoardTraitsT::Color, SpecificPawn, Capture>(board, from, to);

	perftImpl<typename BoardTraitsT::ReverseT>(stats, newBoard, depthToGo-1, MoveInfoT(CaptureMove, to));
      }
    }

    template <typename BoardTraitsT>
    inline void perftImplPawnsCaptureLeft(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureLeft) {
      perftImplPawnsCapture<BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(stats, board, depthToGo, pawnsCaptureLeft);
    }
    
    template <typename BoardTraitsT>
    inline void perftImplPawnsCaptureRight(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureRight) {
      perftImplPawnsCapture<BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(stats, board, depthToGo, pawnsCaptureRight);
    }

    template <typename BoardTraitsT, typename To2FromFn>
    inline void perftImplPawnEpCapture(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsEpCaptureBb) {
      // There can be only 1 en-passant capture, so no need to loop
      if(pawnsEpCaptureBb) {
	const SquareT to = Bits::lsb(pawnsEpCaptureBb);
	const SquareT from = To2FromFn::fn(to);
	const SquareT captureSq = pawnPushOneTo2From<BoardTraitsT::Color>(to);

	const BoardT newBoard = captureEp<BoardTraitsT::Color>(board, from, to, captureSq);

	perftImpl<typename BoardTraitsT::ReverseT>(stats, newBoard, depthToGo-1, MoveInfoT(EpCaptureMove, to));
      }
    }

    template <typename BoardTraitsT>
    inline void perftImplPawnEpCaptureLeft(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureLeft) {
      perftImplPawnEpCapture<BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(stats, board, depthToGo, pawnsCaptureLeft);
    }
    
    template <typename BoardTraitsT>
    inline void perftImplPawnEpCaptureRight(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureRight) {
      perftImplPawnEpCapture<BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(stats, board, depthToGo, pawnsCaptureRight);
    }

    template <SliderDirectionT SliderDirection>
    inline BitBoardT genSliderAttacksBb(const SquareT square, const BitBoardT allPiecesBb) {
      return SliderDirection == Diagonal ? bishopAttacks(square, allPiecesBb) : rookAttacks(square, allPiecesBb);
    }

    template <SliderDirectionT SliderDirection> inline BitBoardT genPinnedPiecesBb(const SquareT myKingSq, const BitBoardT allPiecesBb, const BitBoardT allMyPiecesBb, const ColorStateT& yourState) {
      const BitBoardT myKingSliderAttackersBb = genSliderAttacksBb<SliderDirection>(myKingSq, allPiecesBb);
      // Potentially pinned pieces are my pieces that are on an open ray from my king
      const BitBoardT myCandidateSliderPinnedPiecesBb = myKingSliderAttackersBb & allMyPiecesBb;
      // Your pinning pieces are those that attack my king once my candidate pinned pieces are removed from the board
      const BitBoardT myKingSliderXrayAttackersBb = genSliderAttacksBb<SliderDirection>(myKingSq, (allPiecesBb & ~myCandidateSliderPinnedPiecesBb));
      // Your sliders of the required slider direction
      const BitBoardT yourSlidersBb = yourState.bbs[SliderDirection == Diagonal ? Bishop : Rook] | yourState.bbs[Queen];
      // We don't want direct attackers of the king, but only attackers that were exposed by removing our candidate pins.
      const BitBoardT yourSliderPinnersBb = myKingSliderXrayAttackersBb & ~myKingSliderAttackersBb & yourSlidersBb;
      // Then my pinned pieces are those candidate pinned pieces that lie on one of your pinners' rays
      BitBoardT pinnerRaysBb = BbNone;
      for(BitBoardT bb = yourSliderPinnersBb; bb;) {
	const SquareT pinnerSq = Bits::popLsb(bb);
	pinnerRaysBb |= genSliderAttacksBb<SliderDirection>(pinnerSq, allPiecesBb);
      }
      const BitBoardT mySliderPinnedPiecesBb = myCandidateSliderPinnedPiecesBb & pinnerRaysBb;
      
      return mySliderPinnedPiecesBb;
    }

    template <typename BoardTraitsT>
    inline void perftImplPawnEpMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, SquareT epSquare, const BitBoardT allPiecesBb, const BitBoardT legalPawnsLeftBb, const BitBoardT legalPawnsRightBb) {
      const BitBoardT epSquareBb = bbForSquare(epSquare);

      const BitBoardT semiLegalEpCaptureLeftBb = legalPawnsLeftBb & epSquareBb;
      const BitBoardT semiLegalEpCaptureRightBb = legalPawnsRightBb & epSquareBb;

      // Only do the heavy lifting of detecting discovered check through the captured pawn if there really is an en-passant opportunity
      if((semiLegalEpCaptureLeftBb | semiLegalEpCaptureRightBb) != BbNone) {
	const ColorStateT& yourState = board.pieces[BoardTraitsT::OtherColor];
	const ColorStateT& myState = board.pieces[BoardTraitsT::Color];
	const SquareT myKingSq = myState.pieceSquares[SpecificKing];
	  
	const SquareT to = Bits::lsb(semiLegalEpCaptureLeftBb | semiLegalEpCaptureRightBb);
	const SquareT captureSq = pawnPushOneTo2From<BoardTraitsT::Color>(to);
	const BitBoardT captureSquareBb = bbForSquare(captureSq);

	// Note that a discovered check can only be diagonal or horizontal, because the capturing pawn ends up on the same file as the captured pawn.
	const BitBoardT diagPinnedEpPawnBb = genPinnedPiecesBb<Diagonal>(myKingSq, allPiecesBb, captureSquareBb, yourState);
	// Horizontal is really tricky because it involves both capturing and captured pawn.
	// We detect it by removing them both and looking for a king attack - could optimise this... TODO anyhow
	const BitBoardT orthogPinnedEpPawnBb = BbNone; //genPinnedPiecesBb<Orthogonal>(myKingSq, allPiecesBb, captureSquareBb, yourState);

	if((diagPinnedEpPawnBb | orthogPinnedEpPawnBb) != BbNone) {
	  static bool done = false;
	  if(!done) {
	    printf("\n============================================== EP avoidance EP square is %d ===================================\n\n", epSquare);
	    printBoard(board);
	    printf("\n");
	    done = true;
	  }
	}
	
	if((diagPinnedEpPawnBb | orthogPinnedEpPawnBb) == BbNone) {
	  perftImplPawnEpCaptureLeft<BoardTraitsT>(stats, board, depthToGo, semiLegalEpCaptureLeftBb);
	  perftImplPawnEpCaptureRight<BoardTraitsT>(stats, board, depthToGo, semiLegalEpCaptureRightBb);
	}
      }
    }
    
    template <typename BoardTraitsT>
    inline void perftImplPawnMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const PieceAttacksT& myAttacks, SquareT epSquare, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT legalMoveMaskBb, const PiecePinMasksT& pinMasks) {

      // Pawn pushes
	
      const BitBoardT legalPawnsPushOneBb = myAttacks.pawnsPushOne & legalMoveMaskBb & pinMasks.pawnsPushOnePinMask;
      perftImplPawnsPushOne<BoardTraitsT>(stats, board, depthToGo, legalPawnsPushOneBb);

      const BitBoardT legalPawnsPushTwoBb = myAttacks.pawnsPushTwo & legalMoveMaskBb & pinMasks.pawnsPushTwoPinMask;
      perftImplPawnsPushTwo<BoardTraitsT>(stats, board, depthToGo, legalPawnsPushTwoBb);
	
      // Pawn captures

      const BitBoardT legalPawnsLeftBb = myAttacks.pawnsLeftAttacks & legalMoveMaskBb & pinMasks.pawnsLeftPinMask;
      perftImplPawnsCaptureLeft<BoardTraitsT>(stats, board, depthToGo, legalPawnsLeftBb & allYourPiecesBb);
      
      const BitBoardT legalPawnsRightBb = myAttacks.pawnsRightAttacks & legalMoveMaskBb & pinMasks.pawnsRightPinMask;
      perftImplPawnsCaptureRight<BoardTraitsT>(stats, board, depthToGo, legalPawnsRightBb & allYourPiecesBb);
      
      // Pawn en-passant captures
      // En-passant is tricky because the captured pawn is not on the same square as the capturing piece, and might expose a discovered check itself.
      if(epSquare != InvalidSquare) {
	perftImplPawnEpMoves<BoardTraitsT>(stats, board, depthToGo, epSquare, allPiecesBb, legalPawnsLeftBb, legalPawnsRightBb);
      }
    }
    
    template <typename BoardTraitsT>
    inline void perftImplPawnMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const PawnPushesAndCapturesT& pawnMoves) {

      // Pawn pushes
      perftImplPawnsPushOne<BoardTraitsT>(stats, board, depthToGo, pawnMoves.pushesOneBb);
      perftImplPawnsPushTwo<BoardTraitsT>(stats, board, depthToGo, pawnMoves.pushesTwoBb);
	
      // Pawn captures
      perftImplPawnsCaptureLeft<BoardTraitsT>(stats, board, depthToGo, pawnMoves.capturesLeftBb);
      perftImplPawnsCaptureRight<BoardTraitsT>(stats, board, depthToGo, pawnMoves.capturesRightBb);
      
      // Pawn en-passant captures
      perftImplPawnEpCaptureLeft<BoardTraitsT>(stats, board, depthToGo, pawnMoves.epCaptures.epLeftCaptureBb);
      perftImplPawnEpCaptureRight<BoardTraitsT>(stats, board, depthToGo, pawnMoves.epCaptures.epRightCaptureBb);
    }
    
    template <typename BoardTraitsT, SpecificPieceT SpecificPiece, PushOrCaptureT PushOrCapture>
    inline void perftImplSpecificPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT toBb, const MoveTypeT moveType) {
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);

	const BoardT newBoard = moveSpecificPiece<BoardTraitsT::Color, SpecificPiece, PushOrCapture>(board, from, to);

	perftImpl<typename BoardTraitsT::ReverseT>(stats, newBoard, depthToGo-1, MoveInfoT(moveType, to));
      }
    }
    
    template <typename BoardTraitsT, SpecificPieceT SpecificPiece>
    inline void perftImplSpecificPiecePushes(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, const BitBoardT pushesBb) {
      perftImplSpecificPieceMoves<BoardTraitsT, SpecificPiece, Push>(stats, board, depthToGo, from, pushesBb, PushMove);
    }
    
    template <typename BoardTraitsT, SpecificPieceT SpecificPiece>
    inline void perftImplSpecificPieceCaptures(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, const BitBoardT capturesBb) {
      perftImplSpecificPieceMoves<BoardTraitsT, SpecificPiece, Capture>(stats, board, depthToGo, from, capturesBb, CaptureMove);
    }

    template <typename BoardTraitsT, SpecificPieceT SpecificPiece>
    inline void perftImplSpecificPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const PushesAndCapturesT pushesAndCaptures) {
      const ColorStateT& myState = board.pieces[BoardTraitsT::Color];
      const SquareT from = myState.pieceSquares[SpecificPiece];

      perftImplSpecificPiecePushes<BoardTraitsT, SpecificPiece>(stats, board, depthToGo, from, pushesAndCaptures.pushesBb);
      
      perftImplSpecificPieceCaptures<BoardTraitsT, SpecificPiece>(stats, board, depthToGo, from, pushesAndCaptures.capturesBb);
    }
    
    template <typename BoardTraitsT, SpecificPieceT SpecificPiece>
    inline void perftImplSpecificPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const PieceAttacksT& myAttacks, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT legalMoveMaskBb, const PiecePinMasksT& pinMasks) {

      const ColorStateT& myState = board.pieces[BoardTraitsT::Color];
      const SquareT from = myState.pieceSquares[SpecificPiece];

      const BitBoardT legalAttacksBb = myAttacks.pieceAttacks[SpecificPiece] & legalMoveMaskBb & pinMasks.piecePinMasks[SpecificPiece];
      
      perftImplSpecificPiecePushes<BoardTraitsT, SpecificPiece>(stats, board, depthToGo, from, legalAttacksBb & ~allPiecesBb);
      
      perftImplSpecificPieceCaptures<BoardTraitsT, SpecificPiece>(stats, board, depthToGo, from, legalAttacksBb & allYourPiecesBb);
    }
    
    // Ugh - king still uses this - TODO!
    template <typename BoardTraitsT, SpecificPieceT SpecificPiece>
    inline void perftImplSpecificPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, const BitBoardT attacksBb, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT legalMoveMaskBb, const BitBoardT pinnedMoveMaskBb) {
      const BitBoardT legalAttacksBb = attacksBb & legalMoveMaskBb & pinnedMoveMaskBb;
      perftImplSpecificPiecePushes<BoardTraitsT, SpecificPiece>(stats, board, depthToGo, from, legalAttacksBb & ~allPiecesBb);
      perftImplSpecificPieceCaptures<BoardTraitsT, SpecificPiece>(stats, board, depthToGo, from, legalAttacksBb & allYourPiecesBb);
    }

    // Pinned move mask generation - TODO factor out for pawns too
    
    template <PieceT Piece>
    inline BitBoardT genPinnedMoveMask(const SquareT pieceSq, const SquareT myKingSq, const BitBoardT myDiagPinnedPiecesBb, const BitBoardT myOrthogPinnedPiecesBb);
    
    // Knights
    //  - pinned knights can never move
    template <> inline BitBoardT genPinnedMoveMask<Knight>(const SquareT knightSq, const SquareT myKingSq, const BitBoardT myDiagPinnedPiecesBb, const BitBoardT myOrthogPinnedPiecesBb) {
      const BitBoardT knightBb = bbForSquare(knightSq);
      return (knightBb & (myDiagPinnedPiecesBb | myOrthogPinnedPiecesBb)) == BbNone ? BbAll : BbNone;
    }
    
    // Bishops
    //   - diagonally pinned bishops can only move along the king's bishop rays
    //   - orthogonally pinned bishops cannot move
    template <> inline BitBoardT genPinnedMoveMask<Bishop>(const SquareT bishopSq, const SquareT myKingSq, const BitBoardT myDiagPinnedPiecesBb, const BitBoardT myOrthogPinnedPiecesBb) {
      const BitBoardT bishopBb = bbForSquare(bishopSq);
      const BitBoardT diagPinnedMoveMask = (bishopBb & myDiagPinnedPiecesBb) == BbNone ? BbAll : BishopRays[myKingSq];
      const BitBoardT orthogPinnedMoveMaskBb = (bishopBb & myOrthogPinnedPiecesBb) == BbNone ? BbAll : BbNone;
      return diagPinnedMoveMask & orthogPinnedMoveMaskBb;
    }
    
    // Rooks
    //   - diagonally pinned rooks cannot move
    //   - orthogonally pinned rooks can only move along the king's rook rays
    template <> inline BitBoardT genPinnedMoveMask<Rook>(const SquareT rookSq, const SquareT myKingSq, const BitBoardT myDiagPinnedPiecesBb, const BitBoardT myOrthogPinnedPiecesBb) {
      const BitBoardT rookBb = bbForSquare(rookSq);
      const BitBoardT diagPinnedMoveMask = (rookBb & myDiagPinnedPiecesBb) == BbNone ? BbAll : BbNone;
      const BitBoardT orthogPinnedMoveMaskBb = (rookBb & myOrthogPinnedPiecesBb) == BbNone ? BbAll : RookRays[myKingSq];
      return diagPinnedMoveMask & orthogPinnedMoveMaskBb;
    }
    
    // Queen Moves
    //   - diagonally pinned queens can only move along the king's bishop rays
    //   - orthogonally pinned queens can only move along the king's rook rays
    template <> inline BitBoardT genPinnedMoveMask<Queen>(const SquareT queenSq, const SquareT myKingSq, const BitBoardT myDiagPinnedPiecesBb, const BitBoardT myOrthogPinnedPiecesBb) {
      const BitBoardT queenBb = bbForSquare(queenSq);
      const BitBoardT diagPinnedMoveMask = (queenBb & myDiagPinnedPiecesBb) == BbNone ? BbAll : BishopRays[myKingSq] & ~RookRays[queenSq];
      const BitBoardT orthogPinnedMoveMaskBb = (queenBb & myOrthogPinnedPiecesBb) == BbNone ? BbAll : RookRays[myKingSq] & ~BishopRays[queenSq];
      return diagPinnedMoveMask & orthogPinnedMoveMaskBb;
    }
    
    // Fill a pin mask structure with BbAll for all pieces.
    template <typename MyColorTraitsT>
    inline void genDefaultPiecePinMasks(PiecePinMasksT& pinMasks) {
      // Pawns
      pinMasks.pawnsPushOnePinMask = BbAll;
      pinMasks.pawnsPushTwoPinMask = BbAll;
      pinMasks.pawnsLeftPinMask = BbAll;
      pinMasks.pawnsRightPinMask = BbAll;
      
      // Knights
      pinMasks.piecePinMasks[QueenKnight] = BbAll;
      pinMasks.piecePinMasks[KingKnight] = BbAll;
	
      // Bishops
      pinMasks.piecePinMasks[BlackBishop] = BbAll;
      pinMasks.piecePinMasks[WhiteBishop] = BbAll;

      // Rooks
      pinMasks.piecePinMasks[QueenRook] = BbAll;
      pinMasks.piecePinMasks[KingRook] = BbAll;

      // Queens
      //   - diagonally pinned queens can only move along the king's bishop rays
      //   - orthogonally pinned queens can only move along the king's rook rays
      pinMasks.piecePinMasks[SpecificQueen] = BbAll;

      // TODO other promo pieces
      if(MyColorTraitsT::HasPromos) {
	if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
	  pinMasks.piecePinMasks[PromoQueen] = BbAll;
	}
      }
    }
    
    // Generate the pin masks for all pieces
    // TODO - pawns too
    template <typename ColorTraitsT>
    inline void genPiecePinMasks(PiecePinMasksT& pinMasks, const ColorStateT& myState, const BitBoardT myDiagPinnedPiecesBb, const BitBoardT myOrthogPinnedPiecesBb) {
      const ColorT Color = ColorTraitsT::Color;
      
      const SquareT myKingSq = myState.pieceSquares[SpecificKing];
      
      // Pawn pushes - remove pawns with diagonal pins, and pawns with orthogonal pins along the rank of the king
	
      const BitBoardT myDiagAndKingRankPinsBb = myDiagPinnedPiecesBb | (myOrthogPinnedPiecesBb & RankBbs[rankOf(myKingSq)]);
      const BitBoardT myDiagAndKingRankPinsPushOneBb = pawnsPushOne<Color>(myDiagAndKingRankPinsBb, BbNone);
      pinMasks.pawnsPushOnePinMask = ~myDiagAndKingRankPinsPushOneBb;

      const BitBoardT myDiagAndKingRankPinsPushTwoBb = pawnsPushOne<Color>(myDiagAndKingRankPinsPushOneBb, BbNone);
      pinMasks.pawnsPushTwoPinMask = ~myDiagAndKingRankPinsPushTwoBb;
	
      // Pawn captures - remove pawns with orthogonal pins, and pawns with diagonal pins in the other direction from the capture.
      // Pawn captures on the king's bishop rays are always safe, so we want to remove diagonal pins that are NOT on the king's bishop rays
      
      const BitBoardT myOrthogPinsLeftAttacksBb = pawnsLeftAttacks<Color>(myOrthogPinnedPiecesBb);
      const BitBoardT myDiagPinsLeftAttacksBb = pawnsLeftAttacks<Color>(myDiagPinnedPiecesBb);
      const BitBoardT myUnsafeDiagPinsLeftAttacksBb = myDiagPinsLeftAttacksBb & ~BishopRays[myKingSq];
      pinMasks.pawnsLeftPinMask = ~(myOrthogPinsLeftAttacksBb | myUnsafeDiagPinsLeftAttacksBb);
      
      const BitBoardT myOrthogPinsRightAttacksBb = pawnsRightAttacks<Color>(myOrthogPinnedPiecesBb);
      const BitBoardT myDiagPinsRightAttacksBb = pawnsRightAttacks<Color>(myDiagPinnedPiecesBb);
      const BitBoardT myUnsafeDiagPinsRightAttacksBb = myDiagPinsRightAttacksBb & ~BishopRays[myKingSq];
      pinMasks.pawnsRightPinMask = ~(myOrthogPinsRightAttacksBb | myUnsafeDiagPinsRightAttacksBb); 
      
      // Knights
      pinMasks.piecePinMasks[QueenKnight] = genPinnedMoveMask<Knight>(myState.pieceSquares[QueenKnight], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);
      pinMasks.piecePinMasks[KingKnight] = genPinnedMoveMask<Knight>(myState.pieceSquares[KingKnight], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);
	
      // Bishops
      pinMasks.piecePinMasks[BlackBishop] = genPinnedMoveMask<Bishop>(myState.pieceSquares[BlackBishop], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);
      pinMasks.piecePinMasks[WhiteBishop] = genPinnedMoveMask<Bishop>(myState.pieceSquares[WhiteBishop], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);

      // Rooks
      pinMasks.piecePinMasks[QueenRook] = genPinnedMoveMask<Rook>(myState.pieceSquares[QueenRook], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);
      pinMasks.piecePinMasks[KingRook] = genPinnedMoveMask<Rook>(myState.pieceSquares[KingRook], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);

      // Queens
      //   - diagonally pinned queens can only move along the king's bishop rays
      //   - orthogonally pinned queens can only move along the king's rook rays
      pinMasks.piecePinMasks[SpecificQueen] = genPinnedMoveMask<Queen>(myState.pieceSquares[SpecificQueen], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);

      // TODO other promo pieces
      if(ColorTraitsT::HasPromos) {
	if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
	  pinMasks.piecePinMasks[PromoQueen] = genPinnedMoveMask<Queen>(myState.pieceSquares[PromoQueen], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);
	}
      }
    }
    
    template <typename BoardTraitsT, CastlingRightsT CastlingRight>
    inline void perftImplCastlingMove(PerftStatsT& stats, const BoardT& board, const int depthToGo) {
      const ColorT Color = BoardTraitsT::Color;
      
      const BoardT newBoard1 = moveSpecificPiece<Color, SpecificKing, Push>(board, CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo);
      const BoardT newBoard = moveSpecificPiece<Color, CastlingTraitsT<Color, CastlingRight>::SpecificRook, Push>(newBoard1, CastlingTraitsT<Color, CastlingRight>::RookFrom, CastlingTraitsT<Color, CastlingRight>::RookTo);

      // We pass the rook 'to' square cos we use it for discovered check and check from castling is not considered 'discovered'
      // Or maybe not - getting wrong discoveries count compared to wiki lore - let's try the king instead.
      perftImpl<typename BoardTraitsT::ReverseT>(stats, newBoard, depthToGo-1, MoveInfoT(CastlingMove, CastlingTraitsT<Color, CastlingRight>::KingTo));
    }

    template <typename BoardTraitsT>
    inline void perft0Impl(PerftStatsT& stats, const BoardT& board, const MoveInfoT moveInfo) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;

      const ColorStateT& myState = board.pieces[Color];
      const ColorStateT& yourState = board.pieces[OtherColor];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieces];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieces];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      // It is strictly a bug if we encounter an invalid position - we are doing legal (only) move evaluation.
      const bool CheckForInvalid = false;
      if(CheckForInvalid) {
	// Is your king in check? If so we got here via an illegal move of the pseudo-move-generator
	const SquareAttackersT yourKingAttackers = genSquareAttackers<MyColorTraitsT>(yourState.pieceSquares[SpecificKing], myState, allPiecesBb);
	if(yourKingAttackers.pieceAttackers[AllPieces] != 0) {
	  // Illegal position - doesn't count
	  stats.invalids++;
	  static bool done = false;
	  if(!done) {
	    printf("\n============================================== Invalid Depth 0 - last move to %d! ===================================\n\n", moveInfo.to);
	    printBoard(board);
	    printf("\n");
	    done = true;
	  }
	  return;
	}
      }
	
      stats.nodes++;

      if(moveInfo.moveType == CaptureMove) {
	stats.captures++;
      }

      if(moveInfo.moveType == EpCaptureMove) {
	stats.captures++;
	stats.eps++;
      }

      if(moveInfo.moveType == CastlingMove) {
	stats.castles++;
      }

      const bool DoCheckStats = true;
      const bool DoCheckMateStats = true;
      if(!DoCheckStats) {
	return;
      }

      // Is my king in check?
      const SquareAttackersT myKingAttackers = genSquareAttackers<YourColorTraitsT>(myState.pieceSquares[SpecificKing], yourState, allPiecesBb);
      const BitBoardT allMyKingAttackers = myKingAttackers.pieceAttackers[AllPieces];
      if(allMyKingAttackers != 0) {
	stats.checks++;

	// If the moved piece is not attacking the king then this is a discovered check
	bool isDiscovery = false;
	if((bbForSquare(moveInfo.to) & allMyKingAttackers) == 0) {
	  isDiscovery = true;
	  stats.discoverychecks++;
	}
	  
	// If there are multiple king attackers then we have a double check
	bool isDoubleCheck = false;
	if(Bits::count(allMyKingAttackers) != 1) {
	  isDoubleCheck = true;
	  stats.doublechecks++;
	}

	if(DoCheckMateStats) {
	  bool isPossibleCheckmate = true;
	  // If it's a non-discovery and we can take the checker then it's not checkmate
	  if(isPossibleCheckmate && !isDiscovery && !isDoubleCheck) {
	    // See if the checking piece can be taken
	    stats.l0nondiscoveries++;
	    const SquareAttackersT checkerAttackers = genSquareAttackers<MyColorTraitsT>(moveInfo.to, myState, allPiecesBb);
	    // It's not safe for the king to capture cos he might still be in check
	    if(checkerAttackers.pieceAttackers[AllPieces] &~ checkerAttackers.pieceAttackers[SpecificKing]) {
	      stats.l0checkertakable++;
	      isPossibleCheckmate = false;
	    }
	  }
	  // It's not checkmate if the king can move to safety
	  if(isPossibleCheckmate) {
	    // Remove my king to expose moving away from a slider checker
	    const PieceAttacksT yourAttacks = genPieceAttacks<YourColorTraitsT>(yourState, allPiecesBb & ~myState.bbs[King]);
	    const SquareT myKingSq = myState.pieceSquares[SpecificKing];
	    const BitBoardT myKingAttacksBb = KingAttacks[myKingSq];
	    const BitBoardT myKingMovesBb = myKingAttacksBb & ~allMyPiecesBb;
	    if(myKingMovesBb & ~yourAttacks.allAttacks) {
	      stats.l0checkkingcanmove++;
	      isPossibleCheckmate = false;
	    }
	  }
	  // It's checkmate if there are no valid child nodes.
	  if(isPossibleCheckmate) {
	    PerftStatsT childStats = perft<BoardTraitsT>(board, 1);
	    if(childStats.nodes == 0) {
	      stats.checkmates++;
	    }
	  }
	}
      }
    }

    // Generate a legal move mask for non-king moves - only valid for no check or single check.
    //   If we're not in check then all moves are legal.
    //   If we're in check then we must capture or block the checking piece.
    inline BitBoardT genLegalMoveMaskBb(const BoardT& board, const int nChecks, const BitBoardT allMyKingAttackersBb, const SquareT myKingSq, const BitBoardT allPiecesBb, const PieceAttacksT& yourAttacks) {
	BitBoardT legalMoveMaskBb = BbAll;
	
	// When in check, limit moves to captures of the checking piece, and blockers of the checking piece
	if(nChecks != 0) {
	  // We can evade check by capturing the checking piece
	  legalMoveMaskBb = allMyKingAttackersBb;
	  
	  // There can be only one piece delivering check in this path (nChecks < 2)
	  // If it is a contact check (including knight check) then only a capture (or king move) will evade check.
	  if(((KingAttacks[myKingSq] | KnightAttacks[myKingSq]) & allMyKingAttackersBb) == BbNone) {
	    // Distant check by a slider - we can also block the check.
	    // So here we want to generate all (open) squares between your checking piece and the king.
	    // Work backwards from the king
	    // Compute the check-blocking squares as the intersection of my king's slider 'view' and the checking piece's attack squares.
	    // Note for queens we need to restrict to the slider direction otherwise we get bogus 'blocking' squares in the other queen direction.
	    const SquareT checkingPieceSq = Bits::lsb(allMyKingAttackersBb);
	    const SpecificPieceT checkingSpecificPiece = squarePieceSpecificPiece(board.board[checkingPieceSq]);
	    const BitBoardT diagAttacksFromMyKingBb = bishopAttacks(myKingSq, allPiecesBb);
	    if(allMyKingAttackersBb & diagAttacksFromMyKingBb) {
	      legalMoveMaskBb |= diagAttacksFromMyKingBb & yourAttacks.pieceAttacks[checkingSpecificPiece] & BishopRays[checkingPieceSq];
	    }
	    const BitBoardT orthogAttacksFromMyKingBb = rookAttacks(myKingSq, allPiecesBb);
	    if(allMyKingAttackersBb & orthogAttacksFromMyKingBb) {
	      legalMoveMaskBb |= orthogAttacksFromMyKingBb & yourAttacks.pieceAttacks[checkingSpecificPiece] & RookRays[checkingPieceSq];
	    }
	  }
	}

	return legalMoveMaskBb;
    }

    template <typename BoardTraitsT>
    inline PiecePinMasksT genPinMasks(const BoardT& board) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& myState = board.pieces[Color];
      const ColorStateT& yourState = board.pieces[OtherColor];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieces];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieces];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      const SquareT myKingSq = myState.pieceSquares[SpecificKing];

      // Find my pinned pieces - used to mask out invalid moves due to discovered check on my king
      const BitBoardT myDiagPinnedPiecesBb = genPinnedPiecesBb<Diagonal>(myKingSq, allPiecesBb, allMyPiecesBb, yourState);
      const BitBoardT myOrthogPinnedPiecesBb = genPinnedPiecesBb<Orthogonal>(myKingSq, allPiecesBb, allMyPiecesBb, yourState);
      
      // Generate pinned piece move masks for each piece
      PiecePinMasksT pinMasks = {0};
      // Majority of positions have no pins
      if((myDiagPinnedPiecesBb | myOrthogPinnedPiecesBb) == BbNone) {
	genDefaultPiecePinMasks<MyColorTraitsT>(pinMasks);
      } else {
	genPiecePinMasks<MyColorTraitsT>(pinMasks, myState, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);
      }
      
      return pinMasks;
    }

    template <SpecificPieceT SpecificPiece>
    inline PushesAndCapturesT genSpecificPieceLegalMoves(const PieceAttacksT myAttacks, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT legalMoveMaskBb, const PiecePinMasksT pinMasks) {
      const BitBoardT legalAttacksBb = myAttacks.pieceAttacks[SpecificPiece] & legalMoveMaskBb & pinMasks.piecePinMasks[SpecificPiece];
      const BitBoardT legalPushesBb = legalAttacksBb & ~allPiecesBb;
      const BitBoardT legalCapturesBb = legalAttacksBb & allYourPiecesBb;

      return PushesAndCapturesT(legalPushesBb, legalCapturesBb);
    }

    template <typename BoardTraitsT>
    inline EpPawnCapturesT genLegalPawnEpCaptures(const BoardT& board, const PieceAttacksT myAttacks, const SquareT epSquare, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT legalPawnsLeftBb, const BitBoardT legalPawnsRightBb) {
      const BitBoardT epSquareBb = bbForSquare(epSquare);

      const BitBoardT semiLegalEpCaptureLeftBb = legalPawnsLeftBb & epSquareBb;
      const BitBoardT semiLegalEpCaptureRightBb = legalPawnsRightBb & epSquareBb;

      BitBoardT legalEpCaptureLeftBb = BbNone;
      BitBoardT legalEpCaptureRightBb = BbNone;

      // Only do the heavy lifting of detecting discovered check through the captured pawn if there really is an en-passant opportunity
      // En-passant is tricky because the captured pawn is not on the same square as the capturing piece, and might expose a discovered check itself.
      if((semiLegalEpCaptureLeftBb | semiLegalEpCaptureRightBb) != BbNone) {
	const ColorStateT& yourState = board.pieces[BoardTraitsT::OtherColor];
	const ColorStateT& myState = board.pieces[BoardTraitsT::Color];
	const SquareT myKingSq = myState.pieceSquares[SpecificKing];
	  
	const SquareT to = Bits::lsb(semiLegalEpCaptureLeftBb | semiLegalEpCaptureRightBb);
	const SquareT captureSq = pawnPushOneTo2From<BoardTraitsT::Color>(to);
	const BitBoardT captureSquareBb = bbForSquare(captureSq);

	// Note that a discovered check can only be diagonal or horizontal, because the capturing pawn ends up on the same file as the captured pawn.
	const BitBoardT diagPinnedEpPawnBb = genPinnedPiecesBb<Diagonal>(myKingSq, allPiecesBb, captureSquareBb, yourState);
	// Horizontal is really tricky because it involves both capturing and captured pawn.
	// We detect it by removing them both and looking for a king attack - could optimise this... TODO anyhow
	const BitBoardT orthogPinnedEpPawnBb = BbNone; //genPinnedPiecesBb<Orthogonal>(myKingSq, allPiecesBb, captureSquareBb, yourState);

	if((diagPinnedEpPawnBb | orthogPinnedEpPawnBb) != BbNone) {
	  static bool done = false;
	  if(!done) {
	    printf("\n============================================== EP avoidance EP square is %d ===================================\n\n", epSquare);
	    printBoard(board);
	    printf("\n");
	    done = true;
	  }
	}

	if((diagPinnedEpPawnBb | orthogPinnedEpPawnBb) == BbNone) {
	  legalEpCaptureLeftBb = semiLegalEpCaptureLeftBb;
	  legalEpCaptureRightBb = semiLegalEpCaptureRightBb;
	}
      }

      return EpPawnCapturesT(legalEpCaptureLeftBb, legalEpCaptureRightBb);
    }
    
    template <typename BoardTraitsT>
      inline PawnPushesAndCapturesT genLegalPawnMoves(const BoardT& board, const PieceAttacksT myAttacks, const SquareT epSquare, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT legalMoveMaskBb, const PiecePinMasksT pinMasks) {
      const BitBoardT legalPawnsPushOneBb = myAttacks.pawnsPushOne & legalMoveMaskBb & pinMasks.pawnsPushOnePinMask;
      const BitBoardT legalPawnsPushTwoBb = myAttacks.pawnsPushTwo & legalMoveMaskBb & pinMasks.pawnsPushTwoPinMask;
	
      // Pawn captures

      const BitBoardT legalPawnsLeftBb = myAttacks.pawnsLeftAttacks & legalMoveMaskBb & pinMasks.pawnsLeftPinMask;
      const BitBoardT legalPawnsRightBb = myAttacks.pawnsRightAttacks & legalMoveMaskBb & pinMasks.pawnsRightPinMask;

      // Pawn en-passant captures
      const EpPawnCapturesT legalEpPawnCaptures = (epSquare == InvalidSquare) ? EpPawnCapturesT() : genLegalPawnEpCaptures<BoardTraitsT>(board, myAttacks, epSquare, allYourPiecesBb, allPiecesBb, legalPawnsLeftBb, legalPawnsRightBb);
      
      return PawnPushesAndCapturesT(legalPawnsPushOneBb, legalPawnsPushTwoBb, legalPawnsLeftBb & allYourPiecesBb, legalPawnsRightBb & allYourPiecesBb, legalEpPawnCaptures);
    }
    
    template <typename BoardTraitsT> 
    inline void genLegalNonKingMoves(LegalMovesT& legalMoves, const BoardT& board, const PieceAttacksT& myAttacks, const BitBoardT legalMoveMaskBb, const PiecePinMasksT pinMasks) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& myState = board.pieces[Color];
      const ColorStateT& yourState = board.pieces[OtherColor];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieces];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieces];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      legalMoves.pawnMoves = genLegalPawnMoves<BoardTraitsT>(board, myAttacks, yourState.epSquare, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinMasks);
      
      legalMoves.specificPieceMoves[QueenKnight] = genSpecificPieceLegalMoves<QueenKnight>(myAttacks, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinMasks);
      legalMoves.specificPieceMoves[KingKnight] = genSpecificPieceLegalMoves<KingKnight>(myAttacks, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinMasks);

      legalMoves.specificPieceMoves[BlackBishop] = genSpecificPieceLegalMoves<BlackBishop>(myAttacks, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinMasks);
      legalMoves.specificPieceMoves[WhiteBishop] = genSpecificPieceLegalMoves<WhiteBishop>(myAttacks, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinMasks);
      
      legalMoves.specificPieceMoves[QueenRook] = genSpecificPieceLegalMoves<QueenRook>(myAttacks, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinMasks);
      legalMoves.specificPieceMoves[KingRook] = genSpecificPieceLegalMoves<KingRook>(myAttacks, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinMasks);

      legalMoves.specificPieceMoves[SpecificQueen] = genSpecificPieceLegalMoves<SpecificQueen>(myAttacks, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinMasks);
      
      // TODO other promo pieces
      if(MyColorTraitsT::HasPromos) {
	if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
	  legalMoves.specificPieceMoves[PromoQueen] = genSpecificPieceLegalMoves<PromoQueen>(myAttacks, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinMasks);
	}
      }
    }

    template <typename BoardTraitsT>
    inline CastlingRightsT genLegalCastlingFlags(const BoardT& board, const PieceAttacksT& yourAttacks, const BitBoardT allPiecesBb) {
      const ColorT Color = BoardTraitsT::Color;
      const ColorStateT& myState = board.pieces[Color];

      CastlingRightsT canCastleFlags = NoCastlingRights;
      
      CastlingRightsT castlingRights = castlingRightsWithSpace<Color>(myState.castlingRights, allPiecesBb);
      if(castlingRights) {
	
	if((castlingRights & CanCastleQueenside) && (yourAttacks.allAttacks & CastlingTraitsT<Color, CanCastleQueenside>::CastlingThruCheckBbMask) == BbNone) {
	  canCastleFlags = (CastlingRightsT)(canCastleFlags | CanCastleQueenside);
	}
	
	if((castlingRights & CanCastleKingside) && (yourAttacks.allAttacks & CastlingTraitsT<Color, CanCastleKingside>::CastlingThruCheckBbMask) == BbNone) {
	  canCastleFlags = (CastlingRightsT)(canCastleFlags | CanCastleKingside);
	}	
      }
      
      return canCastleFlags;
    }
    
    template <typename BoardTraitsT>
    inline PushesAndCapturesT genLegalKingMoves(const BoardT& board, const PieceAttacksT& yourAttacks, const BitBoardT allMyKingAttackersBb) {
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& myState = board.pieces[Color];
      const ColorStateT& yourState = board.pieces[OtherColor];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieces];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieces];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;
      
      // King cannot move into check
      // King also cannot move away from a checking slider cos it's still in check.
      BitBoardT illegalKingSquaresBb = BbNone;
      const BitBoardT myKingBb = myState.bbs[King];
      BitBoardT diagSliderCheckersBb = allMyKingAttackersBb & (yourState.bbs[Bishop] | yourState.bbs[Queen]);
      while(diagSliderCheckersBb) {
	const SquareT sliderSq = Bits::popLsb(diagSliderCheckersBb);
	illegalKingSquaresBb |= bishopAttacks(sliderSq, allPiecesBb & ~myKingBb);
      }
      BitBoardT orthogSliderCheckersBb = allMyKingAttackersBb & (yourState.bbs[Rook] | yourState.bbs[Queen]);
      while(orthogSliderCheckersBb) {
	const SquareT sliderSq = Bits::popLsb(orthogSliderCheckersBb);
	illegalKingSquaresBb |= rookAttacks(sliderSq, allPiecesBb & ~myKingBb);
      }

      const SquareT kingSq = myState.pieceSquares[SpecificKing];
      const BitBoardT legalKingMovesBb = KingAttacks[kingSq] & ~yourAttacks.allAttacks & ~illegalKingSquaresBb;

      const BitBoardT legalPushesBb = legalKingMovesBb & ~allPiecesBb;
      const BitBoardT legalCapturesBb = legalKingMovesBb & allYourPiecesBb;

      return PushesAndCapturesT(legalPushesBb, legalCapturesBb);
    }
    
    template <typename BoardTraitsT>
    inline LegalMovesT genLegalMoves(const BoardT& board) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& myState = board.pieces[Color];
      const ColorStateT& yourState = board.pieces[OtherColor];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieces];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieces];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;
      
      LegalMovesT legalMoves;
      
      // Generate moves
      const PieceAttacksT myAttacks = genPieceAttacks<MyColorTraitsT>(myState, allPiecesBb);

      // Is your king in check? If so we got here via an illegal move of the move-generator
      if((myAttacks.allAttacks & yourState.bbs[King]) != 0) {
	// Illegal position - doesn't count
	legalMoves.isIllegalPos = true;
      }

      // This is now a legal position.

      // Evaluate check - eventually do this in the parent

      const SquareT myKingSq = myState.pieceSquares[SpecificKing];
      const SquareAttackersT myKingAttackers = genSquareAttackers<YourColorTraitsT>(myKingSq, yourState, allPiecesBb);
      const BitBoardT allMyKingAttackersBb = myKingAttackers.pieceAttackers[AllPieces];

      const int nChecks = Bits::count(allMyKingAttackersBb);

      // Needed for castling and for king moves so evaluate this here.
      const PieceAttacksT yourAttacks = genPieceAttacks<YourColorTraitsT>(yourState, allPiecesBb);

      // Double check can only be evaded by moving the king so only bother with other pieces if nChecks < 2
      if(nChecks < 2) {

	// If we're in check then the only legal moves are capture or blocking of the checking piece.
	const BitBoardT legalMoveMaskBb = genLegalMoveMaskBb(board, nChecks, allMyKingAttackersBb, myKingSq, allPiecesBb, yourAttacks);
	  
	// Calculate pinned piece move restrictions.
	const PiecePinMasksT pinMasks = genPinMasks<BoardTraitsT>(board);

	// Filter legal non-king moves
	genLegalNonKingMoves<BoardTraitsT>(legalMoves, board, myAttacks, legalMoveMaskBb, pinMasks);

	// Castling
	legalMoves.canCastleFlags = genLegalCastlingFlags<BoardTraitsT>(board, yourAttacks, allMyKingAttackersBb);
      }

      legalMoves.specificPieceMoves[SpecificKing] = genLegalKingMoves<BoardTraitsT>(board, yourAttacks, allMyKingAttackersBb);
      
      return legalMoves;
    }

    template <typename BoardTraitsT>
    inline void perftImplFull(PerftStatsT& stats, const BoardT& board, const int depthToGo, const MoveInfoT moveInfo) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      
      // Generate (legal) moves
      const LegalMovesT legalMoves = genLegalMoves<BoardTraitsT>(board);

      // Is this an illegal pos - note this should never happen(tm)
      if(legalMoves.isIllegalPos) {
	// Illegal position - don't count
	stats.invalidsnon0++;
	static bool done = false;
	if(!done) {
	  printf("\n============================================== Invalid - last move to %d! ===================================\n\n", moveInfo.to);
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
	
	//perftImplPawnMoves<BoardTraitsT>(stats, board, depthToGo, myAttacks, yourState.epSquare, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinMasks);
	perftImplPawnMoves<BoardTraitsT>(stats, board, depthToGo, legalMoves.pawnMoves);
	
	// Knights

	perftImplSpecificPieceMoves<BoardTraitsT, QueenKnight>(stats, board, depthToGo, legalMoves.specificPieceMoves[QueenKnight]);

	perftImplSpecificPieceMoves<BoardTraitsT, KingKnight>(stats, board, depthToGo, legalMoves.specificPieceMoves[KingKnight]);
	
	// Bishops
      
	perftImplSpecificPieceMoves<BoardTraitsT, BlackBishop>(stats, board, depthToGo, legalMoves.specificPieceMoves[BlackBishop]); 

	perftImplSpecificPieceMoves<BoardTraitsT, WhiteBishop>(stats, board, depthToGo, legalMoves.specificPieceMoves[WhiteBishop]); 

	// Rooks

	perftImplSpecificPieceMoves<BoardTraitsT, QueenRook>(stats, board, depthToGo, legalMoves.specificPieceMoves[QueenRook]); 

	perftImplSpecificPieceMoves<BoardTraitsT, KingRook>(stats, board, depthToGo, legalMoves.specificPieceMoves[KingRook]); 

	// Queens

	perftImplSpecificPieceMoves<BoardTraitsT, SpecificQueen>(stats, board, depthToGo, legalMoves.specificPieceMoves[SpecificQueen]); 

	// TODO other promo pieces
	if(MyColorTraitsT::HasPromos) {
	  if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
	    perftImplSpecificPieceMoves<BoardTraitsT, PromoQueen>(stats, board, depthToGo, legalMoves.specificPieceMoves[PromoQueen]);  
	  }
	}

	// Castling
	CastlingRightsT canCastleFlags = legalMoves.canCastleFlags;
	if(canCastleFlags) {

	  if((canCastleFlags & CanCastleQueenside)) {
	    perftImplCastlingMove<BoardTraitsT, CanCastleQueenside>(stats, board, depthToGo);
	  }

	  if((canCastleFlags & CanCastleKingside)) {
	    perftImplCastlingMove<BoardTraitsT, CanCastleKingside>(stats, board, depthToGo);
	  }	
	}

      } // nChecks < 2
      
      // King
      perftImplSpecificPieceMoves<BoardTraitsT, SpecificKing>(stats, board, depthToGo, legalMoves.specificPieceMoves[SpecificKing]); 


    }

    template <typename BoardTraitsT>
    inline void perftImpl(PerftStatsT& stats, const BoardT& board, const int depthToGo, const MoveInfoT moveInfo) {
      // If this is a leaf node, gather stats.
      if(depthToGo == 0) {
	perft0Impl<BoardTraitsT>(stats, board, moveInfo);
      } else {
	perftImplFull<BoardTraitsT>(stats, board, depthToGo, moveInfo);
      }
    }
      
    template <typename BoardTraitsT> inline PerftStatsT perft(const BoardT& board, const int depthToGo) {
      PerftStatsT stats = {0};

      perftImpl<BoardTraitsT>(stats, board, depthToGo, MoveInfoT(PushMove, InvalidSquare));

      return stats;
    }

  } // namespace Perf
} // namespace Chess

#endif //ndef PERFT_HPP
