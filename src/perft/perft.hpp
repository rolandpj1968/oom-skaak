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
    SquareT pawnPushOneTo2From(SquareT square);
    template <> SquareT pawnPushOneTo2From<White>(SquareT square) { return square - 8; }
    template <> SquareT pawnPushOneTo2From<Black>(SquareT square) { return square + 8; }

    template <ColorT Color>
    struct PawnPushOneTo2FromFn {
      static SquareT fn(SquareT from) { return pawnPushOneTo2From<Color>(from); }
    };

    template <ColorT Color>
    SquareT pawnPushTwoTo2From(SquareT square);
    template <> SquareT pawnPushTwoTo2From<White>(SquareT square) { return square - 16; }
    template <> SquareT pawnPushTwoTo2From<Black>(SquareT square) { return square + 16; }

    template <ColorT Color>
    struct PawnPushTwoTo2FromFn {
      static SquareT fn(SquareT from) { return pawnPushTwoTo2From<Color>(from); }
    };

    template <ColorT Color, typename To2FromFn, bool IsPushTwo, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplPawnsPush(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPush) {
      // No captures here - these are just pawn pushes and already filtered from all target clashes.
      while(pawnsPush) {
	SquareT to = Bits::popLsb(pawnsPush);
	SquareT from = To2FromFn::fn(to);

	BoardT newBoard = moveSpecificPiece<Color, SpecificPawn, Push, IsPushTwo>(board, from, to);

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
    inline SquareT pawnAttackLeftTo2From(SquareT square);
    template <> inline SquareT pawnAttackLeftTo2From<White>(SquareT square) { return square - 7; }
    template <> inline SquareT pawnAttackLeftTo2From<Black>(SquareT square) { return square + 9; }

    template <ColorT Color>
    struct PawnAttackLeftTo2FromFn {
      static inline SquareT fn(SquareT from) { return pawnAttackLeftTo2From<Color>(from); }
    };

    template <ColorT Color>
    inline SquareT pawnAttackRightTo2From(SquareT square);
    template <> inline SquareT pawnAttackRightTo2From<White>(SquareT square) { return square - 9; }
    template <> inline SquareT pawnAttackRightTo2From<Black>(SquareT square) { return square + 7; }

    template <ColorT Color>
    struct PawnAttackRightTo2FromFn {
      static inline SquareT fn(SquareT from) { return pawnAttackRightTo2From<Color>(from); }
    };

    template <ColorT Color, typename To2FromFn, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplPawnsCapture(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCapture) {
      while(pawnsCapture) {
	SquareT to = Bits::popLsb(pawnsCapture);
	SquareT from = To2FromFn::fn(to);

	BoardT newBoard = moveSpecificPiece<Color, SpecificPawn, Capture>(board, from, to);

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
    inline void perftImplPawnEpCapture(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsEpCapture) {
      // There can be only 1 en-passant capture, so no need to loop
      if(pawnsEpCapture) {
	SquareT to = Bits::popLsb(pawnsEpCapture);
	SquareT from = To2FromFn::fn(to);
	SquareT captureSquare = pawnPushOneTo2From<Color>(to);

	BoardT newBoard = captureEp<Color>(board, from, to, captureSquare);

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

    template <ColorT Color, SpecificPieceT SpecificPiece, PushOrCaptureT PushOrCapture, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplSpecificPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT toBb, const MoveTypeT moveType) {
      while(toBb) {
	SquareT to = Bits::popLsb(toBb);

	BoardT newBoard = moveSpecificPiece<Color, SpecificPiece, PushOrCapture>(board, from, to);

	perftImpl<OtherColorT<Color>::value, YourBoardTraitsT, MyBoardTraitsT>(stats, newBoard, depthToGo-1, MoveInfoT(moveType, to));
      }
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplSpecificPiecePushes(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT pushesBb) {
      perftImplSpecificPieceMoves<Color, SpecificPiece, Push, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, from, pushesBb, PushMove);
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplSpecificPieceCaptures(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT capturesBb) {
      perftImplSpecificPieceMoves<Color, SpecificPiece, Capture, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, from, capturesBb, CaptureMove);
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplSpecificPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT attacksBb, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb) {
      perftImplSpecificPiecePushes<Color, SpecificPiece, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, from, attacksBb & ~allPiecesBb);
      perftImplSpecificPieceCaptures<Color, SpecificPiece, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, from, attacksBb & allYourPiecesBb);
    }
    
    // Knights - pinned knights can never move
    template <ColorT Color, SpecificPieceT SpecificKnight, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplKnightMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const PieceAttacksT& myAttacks, const BitBoardT myPinnedPiecesBb, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb) {
      const ColorStateT& myState = board.pieces[Color];
      SquareT specificKnightSq = myState.pieceSquares[SpecificKnight];
      if(specificKnightSq != InvalidSquare) {
	BitBoardT specificKnightBb = bbForSquare(specificKnightSq);
	if((specificKnightBb & myPinnedPiecesBb) == BbNone) {
	  perftImplSpecificPieceMoves<Color, SpecificKnight, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, specificKnightSq, myAttacks.pieceAttacks[SpecificKnight], allYourPiecesBb, allPiecesBb);
	}
      }
    }
    
    template <ColorT Color, CastlingRightsT CastlingRight, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplCastlingMove(PerftStatsT& stats, const BoardT& board, const int depthToGo) {
      BoardT newBoard1 = moveSpecificPiece<Color, SpecificKing, Push>(board, CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo);
      BoardT newBoard = moveSpecificPiece<Color, CastlingTraitsT<Color, CastlingRight>::SpecificRook, Push>(newBoard1, CastlingTraitsT<Color, CastlingRight>::RookFrom, CastlingTraitsT<Color, CastlingRight>::RookTo);

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

      // Is your king in check? If so we got here via an illegal move of the pseudo-move-generator
      SquareAttackersT yourKingAttackers = genSquareAttackers<Color, MyBoardTraitsT>(yourState.pieceSquares[SpecificKing], myState, allPiecesBb);
      if(yourKingAttackers.pieceAttackers[AllPieces] != 0) {
	// Illegal position - doesn't count
	stats.invalids++;
	return;
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
      SquareAttackersT myKingAttackers = genSquareAttackers<OtherColorT<Color>::value, MyBoardTraitsT>(myState.pieceSquares[SpecificKing], yourState, allPiecesBb);
      BitBoardT allMyKingAttackers = myKingAttackers.pieceAttackers[AllPieces];
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
    
    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplFull(PerftStatsT& stats, const BoardT& board, const int depthToGo, const MoveInfoT moveInfo) {
      
      const ColorStateT& myState = board.pieces[Color];
      const ColorStateT& yourState = board.pieces[OtherColorT<Color>::value];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieces];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieces];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      // Find my pinned pieces - used to filter out invalid moves
      SquareT myKingSq = myState.pieceSquares[SpecificKing];

      // Diagonally pinned pieces
      BitBoardT myKingDiagAttackersBb = bishopAttacks(myKingSq, allPiecesBb);
      // Potentially pinned pieces are my pieces that are on an open diagonal from my king
      BitBoardT myCandidateDiagPinnedPiecesBb = myKingDiagAttackersBb & allMyPiecesBb;
      // Your pinning pieces are those that attack my king once my candidate pinned pieces are removed from the board
      BitBoardT myKingDiagXrayAttackersBb = bishopAttacks(myKingSq, (allPiecesBb & ~myCandidateDiagPinnedPiecesBb));
      // We don't want direct attackers of the king, but only attackers that were exposed by removing our candidate pins.
      BitBoardT yourDiagPinnersBb = myKingDiagXrayAttackersBb & ~myKingDiagAttackersBb & (yourState.bbs[Bishop] | yourState.bbs[Queen]);
      // Then my pinned pieces are those candidate pinned pieces that lie on one of your pinners' diagonals
      BitBoardT pinnerDiagonalsBb = BbNone;
      for(BitBoardT bb = yourDiagPinnersBb; bb;) {
	SquareT pinnerSq = Bits::popLsb(bb);
	pinnerDiagonalsBb |= bishopAttacks(pinnerSq, allPiecesBb);
      }
      BitBoardT myDiagPinnedPiecesBb = myCandidateDiagPinnedPiecesBb & pinnerDiagonalsBb;

      // Orthogonally pinned pieces
      BitBoardT myKingOrthogAttackersBb = rookAttacks(myKingSq, allPiecesBb);
      // Potentially pinned pieces are my pieces that are on an open orthogonal from my king
      BitBoardT myCandidateOrthogPinnedPiecesBb = myKingOrthogAttackersBb & allMyPiecesBb;
      // Your pinning pieces are those that attack my king once my candidate pinned pieces are removed from the board
      BitBoardT myKingOrthogXrayAttackersBb = rookAttacks(myKingSq, (allPiecesBb & ~myCandidateOrthogPinnedPiecesBb));
      BitBoardT yourOrthogPinnersBb = myKingOrthogXrayAttackersBb & ~myKingOrthogAttackersBb & (yourState.bbs[Rook] | yourState.bbs[Queen]);
      // Then my pinned pieces are those candidate pinned pieces that lie on one of your pinners' orthogonals
      BitBoardT pinnerOrthogonalsBb = BbNone;
      for(BitBoardT bb = yourOrthogPinnersBb; bb;) {
	SquareT pinnerSq = Bits::popLsb(bb);
	pinnerOrthogonalsBb |= rookAttacks(pinnerSq, allPiecesBb);
      }
      // Gack - picks up pieces on the other side of the king
      BitBoardT myOrthogPinnedPiecesBb = myCandidateOrthogPinnedPiecesBb & pinnerOrthogonalsBb;

      if((myDiagPinnedPiecesBb | myOrthogPinnedPiecesBb) != BbNone) {
	if(false && myDiagPinnedPiecesBb) {
	  static bool done = false;
	  if(!done) {
	    printf("\n==================== Color %s ========================= Diag Pins! ===================================\n\n", (Color == White ? "White" : "Black"));
	    printBoard(board);
	    printf("\n\n myKingDiagAttackersBb:\n");
	    printBb(myKingDiagAttackersBb);
	    printf("\n\n myCandidateDiagPinnedPiecesBb:\n");
	    printBb(myCandidateDiagPinnedPiecesBb);
	    printf("\n\n myKingDiagXrayAttackersBb:\n");
	    printBb(myKingDiagXrayAttackersBb);
	    printf("\n\n yourDiagPinnersBb:\n");
	    printBb(yourDiagPinnersBb);
	    printf("\n\n myDiagPinnedPiecesBb:\n");
	    printBb(myDiagPinnedPiecesBb);
	    printf("\n");
	    done = true;
	  }
	  stats.withyourpins++;
	}
	if(false && myOrthogPinnedPiecesBb) {
	  static bool done = false;
	  if(!done) {
	    printf("\n============================================== Orthog Pins! ===================================\n\n");
	    printBoard(board);
	    printf("\n\n myKingOrthogAttackersBb:\n");
	    printBb(myKingOrthogAttackersBb);
	    printf("\n\n myCandidateOrthogPinnedPiecesBb:\n");
	    printBb(myCandidateOrthogPinnedPiecesBb);
	    printf("\n\n myKingOrthogXrayAttackersBb:\n");
	    printBb(myKingOrthogXrayAttackersBb);
	    printf("\n\n yourOrthogPinnersBb:\n");
	    printBb(yourOrthogPinnersBb);
	    printf("\n\n myOrthogPinnedPiecesBb:\n");
	    printBb(myOrthogPinnedPiecesBb);
	    printf("\n");
	    done = true;
	  }
	  stats.withyourpins++;
	}
      }
      if((yourDiagPinnersBb | yourOrthogPinnersBb) != BbNone) {
	stats.withyourpins2++;
      }

      // Generate moves
      PieceAttacksT myAttacks = genPieceAttacks<Color, MyBoardTraitsT>(myState, allPiecesBb);

      // Is your king in check? If so we got here via an illegal move of the pseudo-move-generator
      if((myAttacks.allAttacks & yourState.bbs[King]) != 0) {
	// Illegal position - doesn't count
	stats.invalidsnon0++;
	return;
      }

      // This is now a legal position.

      // Evaluate moves

      // Pawn pushes - remove pawns with diagonal pins, and pawns with orthogonal pins along the rank of the king
      BitBoardT myDiagAndKingRankPinsBb = myDiagPinnedPiecesBb | (myOrthogPinnedPiecesBb & RankBbs[rankOf(myKingSq)]);
      BitBoardT myDiagAndKingRankPinsPushOneBb = pawnsPushOne<Color>(myDiagAndKingRankPinsBb, BbNone);
      BitBoardT nonPinnedPawnsPushOneBb = myAttacks.pawnsPushOne & ~myDiagAndKingRankPinsPushOneBb;
      perftImplPawnsPushOne<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, nonPinnedPawnsPushOneBb);
      BitBoardT myDiagAndKingRankPinsPushTwoBb = pawnsPushOne<Color>(myDiagAndKingRankPinsPushOneBb, BbNone);
      BitBoardT nonPinnedPawnsPushTwoBb = myAttacks.pawnsPushTwo & ~myDiagAndKingRankPinsPushTwoBb;
      perftImplPawnsPushTwo<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, nonPinnedPawnsPushTwoBb);
      
      // Pawn captures - remove pawns with orthogonal pins, and pawns with diagonal pins in the other direction from the capture.
      // Pawn captures on the king's bishop rays are always safe, so we want to remove diagonal pins that are NOT on the king's bishop rays
      BitBoardT myKingBishopRays = BishopRays[myKingSq];

      BitBoardT myOrthogPinsLeftAttacksBb = pawnsLeftAttacks<Color>(myOrthogPinnedPiecesBb);
      BitBoardT myDiagPinsLeftAttacksBb = pawnsLeftAttacks<Color>(myDiagPinnedPiecesBb);
      BitBoardT myUnsafeDiagPinsLeftAttacksBb = myDiagPinsLeftAttacksBb & ~myKingBishopRays;
      perftImplPawnsCaptureLeft<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks.pawnsLeftAttacks & allYourPiecesBb & ~(myOrthogPinsLeftAttacksBb | myUnsafeDiagPinsLeftAttacksBb));
      
      BitBoardT myOrthogPinsRightAttacksBb = pawnsRightAttacks<Color>(myOrthogPinnedPiecesBb);
      BitBoardT myDiagPinsRightAttacksBb = pawnsRightAttacks<Color>(myDiagPinnedPiecesBb);
      BitBoardT myUnsafeDiagPinsRightAttacksBb = myDiagPinsRightAttacksBb & ~myKingBishopRays;
      perftImplPawnsCaptureRight<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks.pawnsRightAttacks & allYourPiecesBb & ~(myOrthogPinsRightAttacksBb | myUnsafeDiagPinsRightAttacksBb));
      
      // Pawn en-passant captures
      if(yourState.epSquare) {
	BitBoardT epSquareBb = bbForSquare(yourState.epSquare);
	
	perftImplPawnEpCaptureLeft<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks.pawnsLeftAttacks & epSquareBb);
	perftImplPawnEpCaptureRight<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks.pawnsRightAttacks & epSquareBb);
      }

      // Piece moves

      // Knights - pinned knights can never move
      BitBoardT myPinnedPiecesBb = myDiagPinnedPiecesBb | myOrthogPinnedPiecesBb;

      
      
      SquareT queenKnightSq = myState.pieceSquares[QueenKnight];
      if(queenKnightSq != InvalidSquare) {
	BitBoardT queenKnightBb = bbForSquare(queenKnightSq);
	if((queenKnightBb & myPinnedPiecesBb) == BbNone) {
	  perftImplSpecificPieceMoves<Color, QueenKnight, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, queenKnightSq, myAttacks.pieceAttacks[QueenKnight], allYourPiecesBb, allPiecesBb);
	}
      }

      SquareT kingKnightSq = myState.pieceSquares[KingKnight];
      if(kingKnightSq != InvalidSquare) {
	BitBoardT kingKnightBb = bbForSquare(kingKnightSq);
	if((kingKnightBb & myPinnedPiecesBb) == BbNone) {
	  perftImplSpecificPieceMoves<Color, KingKnight, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, kingKnightSq, myAttacks.pieceAttacks[KingKnight], allYourPiecesBb, allPiecesBb);
	}
      }
      //perftImplSpecificPieceMoves<Color, KingKnight, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myState.pieceSquares[KingKnight], myAttacks.pieceAttacks[KingKnight], allYourPiecesBb, allPiecesBb);

      // Bishops
      
      perftImplSpecificPieceMoves<Color, BlackBishop, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myState.pieceSquares[BlackBishop], myAttacks.pieceAttacks[BlackBishop], allYourPiecesBb, allPiecesBb);

      perftImplSpecificPieceMoves<Color, WhiteBishop, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myState.pieceSquares[WhiteBishop], myAttacks.pieceAttacks[WhiteBishop], allYourPiecesBb, allPiecesBb);

      // Rooks

      perftImplSpecificPieceMoves<Color, QueenRook, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myState.pieceSquares[QueenRook], myAttacks.pieceAttacks[QueenRook], allYourPiecesBb, allPiecesBb);

      perftImplSpecificPieceMoves<Color, KingRook, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myState.pieceSquares[KingRook], myAttacks.pieceAttacks[KingRook], allYourPiecesBb, allPiecesBb);

      // Queens

      perftImplSpecificPieceMoves<Color, SpecificQueen, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myState.pieceSquares[SpecificQueen], myAttacks.pieceAttacks[SpecificQueen], allYourPiecesBb, allPiecesBb);

      // King always present
      perftImplSpecificPieceMoves<Color, SpecificKing, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myState.pieceSquares[SpecificKing], myAttacks.pieceAttacks[SpecificKing], allYourPiecesBb, allPiecesBb);

      // TODO other promo pieces
      if(MyBoardTraitsT::hasPromos) {
	if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
	  perftImplSpecificPieceMoves<Color, PromoQueen, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myState.pieceSquares[PromoQueen], myAttacks.pieceAttacks[PromoQueen], allYourPiecesBb, allPiecesBb);
	}
      }

      // Castling
      CastlingRightsT castlingRights2 = castlingRightsWithSpace<Color>(myState.castlingRights, allPiecesBb);
      if(castlingRights2) {
	PieceAttacksT yourAttacks = genPieceAttacks<OtherColorT<Color>::value, YourBoardTraitsT>(yourState, allPiecesBb);

	if((castlingRights2 & CanCastleQueenside) && (yourAttacks.allAttacks & CastlingTraitsT<Color, CanCastleQueenside>::CastlingThruCheckBbMask) == BbNone) {
	  perftImplCastlingMove<Color, CanCastleQueenside, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo);
	}

	if((castlingRights2 & CanCastleKingside) && (yourAttacks.allAttacks & CastlingTraitsT<Color, CanCastleKingside>::CastlingThruCheckBbMask) == BbNone) {
	  perftImplCastlingMove<Color, CanCastleKingside, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo);
	}	
      }
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
