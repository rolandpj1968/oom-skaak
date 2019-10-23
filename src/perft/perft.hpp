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
      u64 withyourpins;
      u64 withyourpins2;
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

    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline PerftStatsT perft(const BoardT& board, const int depthToGo);
    
    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
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

    template <ColorT Color, typename To2FromFn, bool IsPushTwo, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplPawnsPush(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPush) {
      // No captures here - these are just pawn pushes and already filtered from all target clashes.
      while(pawnsPush) {
	const SquareT to = Bits::popLsb(pawnsPush);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = moveSpecificPiece<Color, SpecificPawn, Push, IsPushTwo>(board, from, to);

	perftImpl<OtherColorT<Color>::value, YourBoardTraitsT, MyBoardTraitsT>(stats, newBoard, depthToGo-1, MoveInfoT(PushMove, to));
      }
    }

    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplPawnsPushOne(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPushOne) {
      perftImplPawnsPush<Color, PawnPushOneTo2FromFn<Color>, /*IsPushTwo =*/false, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, pawnsPushOne);
    }
    
    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplPawnsPushTwo(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPushTwo) {
      perftImplPawnsPush<Color, PawnPushTwoTo2FromFn<Color>, /*IsPushTwo =*/true, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, pawnsPushTwo);
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

    template <ColorT Color, typename To2FromFn, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplPawnsCapture(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCapture) {
      while(pawnsCapture) {
	const SquareT to = Bits::popLsb(pawnsCapture);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = moveSpecificPiece<Color, SpecificPawn, Capture>(board, from, to);

	perftImpl<OtherColorT<Color>::value, YourBoardTraitsT, MyBoardTraitsT>(stats, newBoard, depthToGo-1, MoveInfoT(CaptureMove, to));
      }
    }

    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplPawnsCaptureLeft(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureLeft) {
      perftImplPawnsCapture<Color, PawnAttackLeftTo2FromFn<Color>, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, pawnsCaptureLeft);
    }
    
    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplPawnsCaptureRight(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureRight) {
      perftImplPawnsCapture<Color, PawnAttackRightTo2FromFn<Color>, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, pawnsCaptureRight);
    }

    template <ColorT Color, typename To2FromFn, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplPawnEpCapture(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsEpCaptureBb) {
      // There can be only 1 en-passant capture, so no need to loop
      if(pawnsEpCaptureBb) {
	const SquareT to = Bits::lsb(pawnsEpCaptureBb);
	const SquareT from = To2FromFn::fn(to);
	const SquareT captureSq = pawnPushOneTo2From<Color>(to);

	const BoardT newBoard = captureEp<Color>(board, from, to, captureSq);

	perftImpl<OtherColorT<Color>::value, YourBoardTraitsT, MyBoardTraitsT>(stats, newBoard, depthToGo-1, MoveInfoT(EpCaptureMove, to));
      }
    }

    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplPawnEpCaptureLeft(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureLeft) {
      perftImplPawnEpCapture<Color, PawnAttackLeftTo2FromFn<Color>, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, pawnsCaptureLeft);
    }
    
    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplPawnEpCaptureRight(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureRight) {
      perftImplPawnEpCapture<Color, PawnAttackRightTo2FromFn<Color>, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, pawnsCaptureRight);
    }

    template <SliderDirectionT SliderDirection> inline BitBoardT genSliderAttacksBb(const SquareT square, const BitBoardT allPiecesBb) {
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

    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplPawnMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const PieceAttacksT& myAttacks, const SquareT myKingSq, const BitBoardT allMyPiecesBb, const BitBoardT allYourPiecesBb, const BitBoardT myDiagPinnedPiecesBb, const BitBoardT myOrthogPinnedPiecesBb, const SquareT epSquare, const BitBoardT legalMoveMaskBb) {

      // Pawn pushes - remove pawns with diagonal pins, and pawns with orthogonal pins along the rank of the king
	
      const BitBoardT myDiagAndKingRankPinsBb = myDiagPinnedPiecesBb | (myOrthogPinnedPiecesBb & RankBbs[rankOf(myKingSq)]);
      const BitBoardT myDiagAndKingRankPinsPushOneBb = pawnsPushOne<Color>(myDiagAndKingRankPinsBb, BbNone);
      const BitBoardT nonPinnedPawnsPushOneBb = myAttacks.pawnsPushOne & ~myDiagAndKingRankPinsPushOneBb;
      perftImplPawnsPushOne<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, nonPinnedPawnsPushOneBb & legalMoveMaskBb);
      const BitBoardT myDiagAndKingRankPinsPushTwoBb = pawnsPushOne<Color>(myDiagAndKingRankPinsPushOneBb, BbNone);
      const BitBoardT nonPinnedPawnsPushTwoBb = myAttacks.pawnsPushTwo & ~myDiagAndKingRankPinsPushTwoBb;
      perftImplPawnsPushTwo<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, nonPinnedPawnsPushTwoBb & legalMoveMaskBb);
	
      // Pawn captures - remove pawns with orthogonal pins, and pawns with diagonal pins in the other direction from the capture.
      // Pawn captures on the king's bishop rays are always safe, so we want to remove diagonal pins that are NOT on the king's bishop rays
      const BitBoardT myKingBishopRays = BishopRays[myKingSq];
      
      const BitBoardT myOrthogPinsLeftAttacksBb = pawnsLeftAttacks<Color>(myOrthogPinnedPiecesBb);
      const BitBoardT myDiagPinsLeftAttacksBb = pawnsLeftAttacks<Color>(myDiagPinnedPiecesBb);
      const BitBoardT myUnsafeDiagPinsLeftAttacksBb = myDiagPinsLeftAttacksBb & ~myKingBishopRays;
      perftImplPawnsCaptureLeft<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks.pawnsLeftAttacks & allYourPiecesBb & ~(myOrthogPinsLeftAttacksBb | myUnsafeDiagPinsLeftAttacksBb) & legalMoveMaskBb);
      
      const BitBoardT myOrthogPinsRightAttacksBb = pawnsRightAttacks<Color>(myOrthogPinnedPiecesBb);
      const BitBoardT myDiagPinsRightAttacksBb = pawnsRightAttacks<Color>(myDiagPinnedPiecesBb);
      const BitBoardT myUnsafeDiagPinsRightAttacksBb = myDiagPinsRightAttacksBb & ~myKingBishopRays;
      perftImplPawnsCaptureRight<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks.pawnsRightAttacks & allYourPiecesBb & ~(myOrthogPinsRightAttacksBb | myUnsafeDiagPinsRightAttacksBb) & legalMoveMaskBb);
      
      // Pawn en-passant captures
      // En-passant is tricky because the captured pawn is not on the same square as the capturing piece, and might expose a discovered check itself.
      if(epSquare != InvalidSquare) {
	const BitBoardT epSquareBb = bbForSquare(epSquare);

	const BitBoardT semiLegalEpCaptureLeftBb = myAttacks.pawnsLeftAttacks & epSquareBb & ~(myOrthogPinsLeftAttacksBb | myUnsafeDiagPinsLeftAttacksBb) & legalMoveMaskBb;
	const BitBoardT semiLegalEpCaptureRightBb = myAttacks.pawnsRightAttacks & epSquareBb & ~(myOrthogPinsRightAttacksBb | myUnsafeDiagPinsRightAttacksBb) & legalMoveMaskBb;

	// Only do the heavy lifting of detecting discovered check through the captured pawn if there really is an en-passant opportunity
	if((semiLegalEpCaptureLeftBb | semiLegalEpCaptureRightBb) != BbNone) {
	  const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;
	  const ColorStateT& yourState = board.pieces[OtherColorT<Color>::value];
	  
	  const SquareT to = Bits::lsb(semiLegalEpCaptureLeftBb | semiLegalEpCaptureRightBb);
	  const SquareT captureSq = pawnPushOneTo2From<Color>(to);
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
	    perftImplPawnEpCaptureLeft<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, semiLegalEpCaptureLeftBb);
	    perftImplPawnEpCaptureRight<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, semiLegalEpCaptureRightBb);
	  }
	}
      }
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece, PushOrCaptureT PushOrCapture, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplSpecificPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT toBb, const MoveTypeT moveType) {
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);

	const BoardT newBoard = moveSpecificPiece<Color, SpecificPiece, PushOrCapture>(board, from, to);

	perftImpl<OtherColorT<Color>::value, YourBoardTraitsT, MyBoardTraitsT>(stats, newBoard, depthToGo-1, MoveInfoT(moveType, to));
      }
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplSpecificPiecePushes(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, const BitBoardT pushesBb) {
      perftImplSpecificPieceMoves<Color, SpecificPiece, Push, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, from, pushesBb, PushMove);
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplSpecificPieceCaptures(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, const BitBoardT capturesBb) {
      perftImplSpecificPieceMoves<Color, SpecificPiece, Capture, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, from, capturesBb, CaptureMove);
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplSpecificPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, const BitBoardT attacksBb, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT legalMoveMaskBb, const BitBoardT pinnedMoveMaskBb = BbAll) {
      const BitBoardT legalAttacksBb = attacksBb & legalMoveMaskBb & pinnedMoveMaskBb;
      perftImplSpecificPiecePushes<Color, SpecificPiece, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, from, legalAttacksBb & ~allPiecesBb);
      perftImplSpecificPieceCaptures<Color, SpecificPiece, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, from, legalAttacksBb & allYourPiecesBb);
    }

    template <PieceT Piece>
    inline BitBoardT genPinnedMoveMask(const BitBoardT specificPieceBb, const BitBoardT myDiagPinnedPiecesBb, const BitBoardT myOrthogPinnedPiecesBb, const BitBoardT myKingBishopRays, const BitBoardT myKingRookRays);

    // Knights - pinned knights can never move
    template <> inline BitBoardT genPinnedMoveMask<Knight>(const BitBoardT specificKnightBb, const BitBoardT myDiagPinnedPiecesBb, const BitBoardT myOrthogPinnedPiecesBb, const BitBoardT myKingBishopRays, const BitBoardT myKingRookRays) {
      return (specificKnightBb & (myDiagPinnedPiecesBb | myOrthogPinnedPiecesBb)) == BbNone ? BbAll : BbNone;
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplSpecificPieceMovesWithPins(PerftStatsT& stats, const BoardT& board, const int depthToGo, const PieceAttacksT& myAttacks, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT legalMoveMaskBb, const BitBoardT myDiagPinnedPiecesBb, const BitBoardT myOrthogPinnedPiecesBb, const BitBoardT myKingBishopRays, const BitBoardT myKingRookRays) {
      
      const ColorStateT& myState = board.pieces[Color];
      const SquareT specificPieceSq = myState.pieceSquares[SpecificPiece];
      const BitBoardT specificPieceBb = bbForSquare(specificPieceSq);
      const BitBoardT pinnedMoveMaskBb = genPinnedMoveMask<PieceForSpecificPieceT<SpecificPiece>::value>(specificPieceBb, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb, myKingBishopRays, myKingRookRays);
      
      perftImplSpecificPieceMoves<Color, SpecificPiece, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, specificPieceSq, myAttacks.pieceAttacks[SpecificPiece], allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinnedMoveMaskBb);
    }
    
    // Bishop Moves
    //   - orthogonally pinned bishops cannot move
    //   - diagonally pinned bishops can only move along the king's bishop rays
    template <ColorT Color, SpecificPieceT SpecificBishop, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplBishopMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const PieceAttacksT& myAttacks, const BitBoardT myKingBishopRays, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT myDiagPinnedPiecesBb, const BitBoardT myOrthogPinnedPiecesBb, const BitBoardT legalMoveMaskBb) {
      const ColorStateT& myState = board.pieces[Color];
      const SquareT specificBishopSq = myState.pieceSquares[SpecificBishop];
      const BitBoardT specificBishopBb = bbForSquare(specificBishopSq);
      const BitBoardT orthogPinnedMoveMaskBb = (specificBishopBb & myOrthogPinnedPiecesBb) == BbNone ? BbAll : BbNone;
      const BitBoardT diagPinnedMoveMask = (specificBishopBb & myDiagPinnedPiecesBb) == BbNone ? BbAll : myKingBishopRays;
      perftImplSpecificPieceMoves<Color, SpecificBishop, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, specificBishopSq, myAttacks.pieceAttacks[SpecificBishop], allYourPiecesBb, allPiecesBb, legalMoveMaskBb, orthogPinnedMoveMaskBb & diagPinnedMoveMask);
    }
      
    // Rook Moves
    //   - diagonally pinned rooks cannot move
    //   - orthogonally pinned bishops can only move along the king's rook rays
    template <ColorT Color, SpecificPieceT SpecificRook, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplRookMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const PieceAttacksT& myAttacks, const BitBoardT myKingRookRays, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT myDiagPinnedPiecesBb, const BitBoardT myOrthogPinnedPiecesBb, const BitBoardT legalMoveMaskBb) {
      const ColorStateT& myState = board.pieces[Color];
      const SquareT specificRookSq = myState.pieceSquares[SpecificRook];
      if(specificRookSq != InvalidSquare) {
	const BitBoardT specificRookBb = bbForSquare(specificRookSq);
	if((specificRookBb & myDiagPinnedPiecesBb) == BbNone) {
	  BitBoardT specificRookAttacksBb = myAttacks.pieceAttacks[SpecificRook];
	  if(specificRookBb & myOrthogPinnedPiecesBb) {
	    specificRookAttacksBb &= myKingRookRays;
	  }
	  perftImplSpecificPieceMoves<Color, SpecificRook, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, specificRookSq, specificRookAttacksBb, allYourPiecesBb, allPiecesBb, legalMoveMaskBb);
	}
      }
    }
      
    // Queen Moves
    //   - diagonally pinned queens can only move along the king's bishop rays
    //   - orthogonally pinned queens can only move along the king's rook rays
    template <ColorT Color, SpecificPieceT SpecificQueenPiece, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplQueenMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const PieceAttacksT& myAttacks, const BitBoardT myKingBishopRays, const BitBoardT myKingRookRays, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT myDiagPinnedPiecesBb, const BitBoardT myOrthogPinnedPiecesBb, const BitBoardT legalMoveMaskBb) {
      const ColorStateT& myState = board.pieces[Color];
      const SquareT specificQueenSq = myState.pieceSquares[SpecificQueenPiece];
      if(specificQueenSq != InvalidSquare) {
	const BitBoardT specificQueenBb = bbForSquare(specificQueenSq);
	BitBoardT specificQueenAttacksBb = myAttacks.pieceAttacks[SpecificQueenPiece];
	if((specificQueenBb & myDiagPinnedPiecesBb) != BbNone) {
	  specificQueenAttacksBb &= myKingBishopRays & ~RookRays[specificQueenSq];
	}
	if((specificQueenBb & myOrthogPinnedPiecesBb) != BbNone) {
	  specificQueenAttacksBb &= myKingRookRays & ~BishopRays[specificQueenSq];
	}
	perftImplSpecificPieceMoves<Color, SpecificQueenPiece, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, specificQueenSq, specificQueenAttacksBb, allYourPiecesBb, allPiecesBb, legalMoveMaskBb);
      }
    }
      
    template <ColorT Color, CastlingRightsT CastlingRight, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplCastlingMove(PerftStatsT& stats, const BoardT& board, const int depthToGo) {
      const BoardT newBoard1 = moveSpecificPiece<Color, SpecificKing, Push>(board, CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo);
      const BoardT newBoard = moveSpecificPiece<Color, CastlingTraitsT<Color, CastlingRight>::SpecificRook, Push>(newBoard1, CastlingTraitsT<Color, CastlingRight>::RookFrom, CastlingTraitsT<Color, CastlingRight>::RookTo);

      // We pass the rook 'to' square cos we use it for discovered check and check from castling is not considered 'discovered'
      // Or maybe not - getting wrong discoveries count compared to wiki lore - let's try the king instead.
      perftImpl<OtherColorT<Color>::value, YourBoardTraitsT, MyBoardTraitsT>(stats, newBoard, depthToGo-1, MoveInfoT(CastlingMove, CastlingTraitsT<Color, CastlingRight>::KingTo));
    }

    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImpl0(PerftStatsT& stats, const BoardT& board, const MoveInfoT moveInfo) {
      const ColorStateT& myState = board.pieces[Color];
      const ColorStateT& yourState = board.pieces[OtherColorT<Color>::value];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieces];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieces];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      // It is strictly a bug if we encounter an invalid position - we are doing legal (only) move evaluation.
      const bool CheckForInvalid = false;
      if(CheckForInvalid) {
	// Is your king in check? If so we got here via an illegal move of the pseudo-move-generator
	const SquareAttackersT yourKingAttackers = genSquareAttackers<Color, MyBoardTraitsT>(yourState.pieceSquares[SpecificKing], myState, allPiecesBb);
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

      // Is my king in check?
      const SquareAttackersT myKingAttackers = genSquareAttackers<OtherColorT<Color>::value, MyBoardTraitsT>(myState.pieceSquares[SpecificKing], yourState, allPiecesBb);
      const BitBoardT allMyKingAttackers = myKingAttackers.pieceAttackers[AllPieces];
      if(allMyKingAttackers != 0) {
	stats.checks++;

	// If the moved piece is not attacking the king then this is a discovered check
	if((bbForSquare(moveInfo.to) & allMyKingAttackers) == 0) {
	  stats.discoverychecks++;
	}
	  
	// If there are multiple king attackers then we have a double check
	if(Bits::count(allMyKingAttackers) != 1) {
	  stats.doublechecks++;
	}
	  
	// It's checkmate if there are no valid child nodes.
	PerftStatsT childStats = perft<Color, YourBoardTraitsT, MyBoardTraitsT>(board, 1);
	if(childStats.nodes == 0) {
	  stats.checkmates++;
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
	    // So here we want to generate all (open) squares between the your checking piece and the king.
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

    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplFull(PerftStatsT& stats, const BoardT& board, const int depthToGo, const MoveInfoT moveInfo) {
      
      const ColorStateT& myState = board.pieces[Color];
      const ColorStateT& yourState = board.pieces[OtherColorT<Color>::value];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieces];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieces];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      // Check for position legality - eventually do this in the parent
      
      // Generate moves
      const PieceAttacksT myAttacks = genPieceAttacks<Color, MyBoardTraitsT>(myState, allPiecesBb);

      // Is your king in check? If so we got here via an illegal move of the pseudo-move-generator
      if((myAttacks.allAttacks & yourState.bbs[King]) != 0) {
	// Illegal position - doesn't count
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

      // This is now a legal position.

      // Evaluate check - eventually do this in the parent

      const SquareAttackersT myKingAttackers = genSquareAttackers<OtherColorT<Color>::value, MyBoardTraitsT>(myState.pieceSquares[SpecificKing], yourState, allPiecesBb);
      const BitBoardT allMyKingAttackersBb = myKingAttackers.pieceAttackers[AllPieces];
      const SquareT myKingSq = myState.pieceSquares[SpecificKing];

      const int nChecks = Bits::count(allMyKingAttackersBb);

      // Needed for castling and for king moves so evaluate this here.
      const PieceAttacksT yourAttacks = genPieceAttacks<OtherColorT<Color>::value, YourBoardTraitsT>(yourState, allPiecesBb);
      
      // Double check can only be evaded by moving the king
      if(nChecks < 2) {

	// If we're in check then the only legal moves are capture or blocking of the checking piece.
	const BitBoardT legalMoveMaskBb = genLegalMoveMaskBb(board, nChecks, allMyKingAttackersBb, myKingSq, allPiecesBb, yourAttacks);
	  
	// Calculate pins
      
	// Find my pinned pieces - used to mask out invalid moves due to discovered check on my king
	const BitBoardT myDiagPinnedPiecesBb = genPinnedPiecesBb<Diagonal>(myKingSq, allPiecesBb, allMyPiecesBb, yourState);
	const BitBoardT myOrthogPinnedPiecesBb = genPinnedPiecesBb<Orthogonal>(myKingSq, allPiecesBb, allMyPiecesBb, yourState);

	const BitBoardT myKingBishopRays = BishopRays[myKingSq];
	const BitBoardT myKingRookRays = RookRays[myKingSq];
	
	// Evaluate moves

	// Pawns
	
	perftImplPawnMoves<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks, myKingSq, allMyPiecesBb, allYourPiecesBb, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb, yourState.epSquare, legalMoveMaskBb);
	// Knights

	perftImplSpecificPieceMovesWithPins<Color, QueenKnight, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb, myKingBishopRays, myKingRookRays);

	perftImplSpecificPieceMovesWithPins<Color, KingKnight, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb, myKingBishopRays, myKingRookRays);
	
	// Bishops
	//   - diagonally pinned bishops can only move along the king's bishop rays
	//   - orthogonally pinned bishops cannot move
      
	perftImplBishopMoves<Color, BlackBishop, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks, myKingBishopRays, allYourPiecesBb, allPiecesBb, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb, legalMoveMaskBb);

	perftImplBishopMoves<Color, WhiteBishop, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks, myKingBishopRays, allYourPiecesBb, allPiecesBb, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb, legalMoveMaskBb);

	// Rooks
	//   - diagonally pinned rooks cannot move
	//   - orthogonally pinned bishops can only move along the king's rook rays

	perftImplRookMoves<Color, QueenRook, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks, myKingRookRays, allYourPiecesBb, allPiecesBb, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb, legalMoveMaskBb);

	perftImplRookMoves<Color, KingRook, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks, myKingRookRays, allYourPiecesBb, allPiecesBb, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb, legalMoveMaskBb);

	// Queens
	//   - diagonally pinned queens can only move along the king's bishop rays
	//   - orthogonally pinned queens can only move along the king's rook rays

	perftImplQueenMoves<Color, SpecificQueen, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks, myKingBishopRays, myKingRookRays, allYourPiecesBb, allPiecesBb, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb, legalMoveMaskBb);

	// TODO other promo pieces
	if(MyBoardTraitsT::hasPromos) {
	  if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
	    perftImplQueenMoves<Color, PromoQueen, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks, myKingBishopRays, myKingRookRays, allYourPiecesBb, allPiecesBb, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb, legalMoveMaskBb);
	  }
	}

	// Castling
	CastlingRightsT castlingRights2 = castlingRightsWithSpace<Color>(myState.castlingRights, allPiecesBb);
	if(castlingRights2) {

	  if((castlingRights2 & CanCastleQueenside) && (yourAttacks.allAttacks & CastlingTraitsT<Color, CanCastleQueenside>::CastlingThruCheckBbMask) == BbNone) {
	    perftImplCastlingMove<Color, CanCastleQueenside, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo);
	  }

	  if((castlingRights2 & CanCastleKingside) && (yourAttacks.allAttacks & CastlingTraitsT<Color, CanCastleKingside>::CastlingThruCheckBbMask) == BbNone) {
	    perftImplCastlingMove<Color, CanCastleKingside, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo);
	  }	
	}

      } // nChecks < 2
      
      // King - cannot move into check
      // King also cannot move away from a checking slider cos it's still in check.
      // TODO - this cuts out some legal squares squares.
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
      //illegalKingSquaresBb = BbNone;// TODO - the above cuts out some legal moves?
      const SquareT kingSq = myState.pieceSquares[SpecificKing];
      const BitBoardT legalKingMovesBb = KingAttacks[kingSq] & ~yourAttacks.allAttacks & ~illegalKingSquaresBb;
      perftImplSpecificPieceMoves<Color, SpecificKing, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myState.pieceSquares[SpecificKing], legalKingMovesBb, allYourPiecesBb, allPiecesBb, BbAll);

    }

    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImpl(PerftStatsT& stats, const BoardT& board, const int depthToGo, const MoveInfoT moveInfo) {
      // If this is a leaf node, gather stats.
      if(depthToGo == 0) {
	perftImpl0<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, moveInfo);
      } else {
	perftImplFull<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, moveInfo);
      }
    }
      
    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT> inline PerftStatsT perft(const BoardT& board, const int depthToGo) {
      PerftStatsT stats = {0};

      perftImpl<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, MoveInfoT(PushMove, InvalidSquare));

      return stats;
    }

  } // namespace Perf
} // namespace Chess

#endif //ndef PERFT_HPP
