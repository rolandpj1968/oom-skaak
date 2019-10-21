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
      // TODO perft(8) for this is out-by-4 - I suspect en-passant discoveries.
      u64 discoverychecks;
      u64 doublechecks;
      u64 checkmates;
      u64 invalids;
      u64 d0nodes;
      u64 d1nodes;
      u64 d1checks;
      u64 d1fast;
      u64 d1slow;
      u64 d1kinghasmoves;
      u64 d1kinghasmoves2;
      u64 d1kinghas1move;
      u64 d1kingfast;
      u64 d1kingslow;
      u64 d1nochecks;
      u64 d1nodiscoveredchecks;
      u64 d1noillegalmoves;
      u64 d1nonastymoves;
      
    };

    enum MoveTypeT {
      PushMove = 0,
      CaptureMove,
      EpCaptureMove,
      CastlingMove
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
    inline void perftImpl(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT moveTo, const MoveTypeT moveType);
  
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

	perftImpl<OtherColorT<Color>::value, YourBoardTraitsT, MyBoardTraitsT>(stats, newBoard, depthToGo-1, to, PushMove);
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

	perftImpl<OtherColorT<Color>::value, YourBoardTraitsT, MyBoardTraitsT>(stats, newBoard, depthToGo-1, to, CaptureMove);
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

	perftImpl<OtherColorT<Color>::value, YourBoardTraitsT, MyBoardTraitsT>(stats, newBoard, depthToGo-1, to, EpCaptureMove);
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

	perftImpl<OtherColorT<Color>::value, YourBoardTraitsT, MyBoardTraitsT>(stats, newBoard, depthToGo-1, to, moveType);
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
    
    template <ColorT Color, CastlingRightsT CastlingRight, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplCastlingMove(PerftStatsT& stats, const BoardT& board, const int depthToGo) {
      BoardT newBoard1 = moveSpecificPiece<Color, SpecificKing, Push>(board, CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo);
      BoardT newBoard = moveSpecificPiece<Color, CastlingTraitsT<Color, CastlingRight>::SpecificRook, Push>(newBoard1, CastlingTraitsT<Color, CastlingRight>::RookFrom, CastlingTraitsT<Color, CastlingRight>::RookTo);

      // We pass the rook 'to' square cos we use it for discovered check and check from castling is not considered 'discovered'
      // Or maybe not - getting wrong discoveries count compared to wiki lore - let's try the king instead.
      perftImpl<OtherColorT<Color>::value, YourBoardTraitsT, MyBoardTraitsT>(stats, newBoard, depthToGo-1, CastlingTraitsT<Color, CastlingRight>::KingTo, CastlingMove);
    }

    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImplFull(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT moveTo, const MoveTypeT moveType) {
      
      const ColorStateT& myState = board.pieces[Color];
      const ColorStateT& yourState = board.pieces[OtherColorT<Color>::value];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieces];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieces];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      // Generate moves
      PieceAttacksT myAttacks = genPieceAttacks<Color, MyBoardTraitsT>(myState, allPiecesBb);

      // Is your king in check? If so we got here via an illegal move of the pseudo-move-generator
      if((myAttacks.allAttacks & yourState.bbs[King]) != 0) {
	// Illegal position - doesn't count
	stats.invalids++;
	return;
      }

      // This is now a legal position.

      // Evaluate moves

      // Pawn pushes
      perftImplPawnsPushOne<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks.pawnsPushOne);
      perftImplPawnsPushTwo<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks.pawnsPushTwo);
      
      // Pawn captures
      perftImplPawnsCaptureLeft<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks.pawnsLeftAttacks & allYourPiecesBb);
      perftImplPawnsCaptureRight<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks.pawnsRightAttacks & allYourPiecesBb);
      
      // Pawn en-passant captures
      if(yourState.epSquare) {
	BitBoardT epSquareBb = bbForSquare(yourState.epSquare);
	
	perftImplPawnEpCaptureLeft<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks.pawnsLeftAttacks & epSquareBb);
	perftImplPawnEpCaptureRight<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myAttacks.pawnsRightAttacks & epSquareBb);
      }

      // Piece moves

      // Knights
      
      perftImplSpecificPieceMoves<Color, QueenKnight, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myState.pieceSquares[QueenKnight], myAttacks.pieceAttacks[QueenKnight], allYourPiecesBb, allPiecesBb);

      perftImplSpecificPieceMoves<Color, KingKnight, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, myState.pieceSquares[KingKnight], myAttacks.pieceAttacks[KingKnight], allYourPiecesBb, allPiecesBb);

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

    inline void countMoves(PerftStatsT& stats, const BitBoardT attacksBb, const BitBoardT allMyPiecesBb, const BitBoardT allYourPiecesBb) {
	BitBoardT movesBb = attacksBb & ~allMyPiecesBb;
	stats.nodes += Bits::count(movesBb);
	stats.captures += Bits::count(movesBb & allYourPiecesBb);
    }
    
    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImpl1(PerftStatsT& stats, const BoardT& board, const SquareT moveTo, const MoveTypeT moveType) {
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
	
      stats.d1nodes++;
      
      // Is my king in check?
      SquareAttackersT myKingAttackers = genSquareAttackers<OtherColorT<Color>::value, MyBoardTraitsT>(myState.pieceSquares[SpecificKing], yourState, allPiecesBb);
      BitBoardT allMyKingAttackers = myKingAttackers.pieceAttackers[AllPieces];
      if(allMyKingAttackers != 0) {
	// I'm in check - do full move evaluation to cope with invalid moves.
	// We could do special move generation for in-check but it's not worth it for now.
	perftImplFull<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, moveTo, moveType);

	stats.d1checks++;
	return;
      }

      // This is a valid node and I'm not in check - just count moves (except for tricky ones)

      // Generate moves
      PieceAttacksT myAttacks = genPieceAttacks<Color, MyBoardTraitsT>(myState, allPiecesBb);

      // Generate squares that will render check (used to treat discoveries properly)
      SquareAttackersT myKingAttackerSquares = genSquareAttackerSquares<OtherColorT<Color>::value, YourBoardTraitsT>(myState.pieceSquares[SpecificKing], allPiecesBb);
      SquareAttackersT yourKingAttackerSquares = genSquareAttackerSquares<Color, MyBoardTraitsT>(yourState.pieceSquares[SpecificKing], allPiecesBb);


      // printf("Board - %s to move:\n", (Color == White ? "white" : "black"));
      // printBoard(board);
      
      // printf("\n\nAll your king attacker squares:\n");
      // printBb(yourKingAttackerSquares.pieceAttackers[AllPieces]);

      // printf("\n\nAll my piece attack squares:\n");
      // printBb(myAttacks.allAttacks);

      // printf("\n\nOverlap:\n");
      // printBb((yourKingAttackerSquares.pieceAttackers[AllPieces] & myAttacks.allAttacks));
      
      // printf("\n-----------------------------------------------\n");
	
      // Can we deliver check at all?
      BitBoardT pawnChecksBb = yourKingAttackerSquares.pieceAttackers[Pawn] & (myAttacks.pawnsPushOne | myAttacks.pawnsPushTwo | myAttacks.pawnsLeftAttacks | myAttacks.pawnsRightAttacks) & ~myState.bbs[AllPieces];
      BitBoardT knightChecksBb = yourKingAttackerSquares.pieceAttackers[Knight] & (myAttacks.pieceAttacks[QueenKnight] | myAttacks.pieceAttacks[KingKnight]) & ~myState.bbs[AllPieces];
      BitBoardT bishopChecksBb = yourKingAttackerSquares.pieceAttackers[Bishop] & (myAttacks.pieceAttacks[BlackBishop] | myAttacks.pieceAttacks[WhiteBishop]) & ~myState.bbs[AllPieces];
      BitBoardT rookChecksBb = yourKingAttackerSquares.pieceAttackers[Rook] & (myAttacks.pieceAttacks[QueenRook] | myAttacks.pieceAttacks[KingRook]) & ~myState.bbs[AllPieces];
      BitBoardT queenChecksBb = yourKingAttackerSquares.pieceAttackers[Queen] & (myAttacks.pieceAttacks[SpecificQueen]) & ~myState.bbs[AllPieces];
      // Cannot deliver check with the king

      BitBoardT allPiecesChecksBb = pawnChecksBb | knightChecksBb | bishopChecksBb | rookChecksBb | queenChecksBb;
      
      // Can we deliver discovered check?
      // In order to deliver discovered check, we require (at least):
      //   1. One of our pieces on a slider attack path to your king
      //   2. One of our sliders on the same slider attack path.
      // More is required in practice but hopefully this will suffice to limit the incidence.
      BitBoardT myDiagPinnersBb = BishopRays[yourState.pieceSquares[SpecificKing]] & (myState.bbs[Bishop] | myState.bbs[Queen]);
      BitBoardT myDiagBlockingPiecessBb = myDiagPinnersBb ? (yourKingAttackerSquares.pieceAttackers[Bishop] & myState.bbs[AllPieces]) : BbNone;
      BitBoardT myOrthoPinnersBb = RookRays[yourState.pieceSquares[SpecificKing]] & (myState.bbs[Rook] | myState.bbs[Queen]);
      BitBoardT myOrthoBlockingPiecessBb = myOrthoPinnersBb ? (yourKingAttackerSquares.pieceAttackers[Rook] & myState.bbs[AllPieces]) : BbNone;

      BitBoardT myBlockingPiecesBb = myDiagBlockingPiecessBb | myOrthoBlockingPiecessBb;

      // Can we move into check through discovery?
      BitBoardT yourDiagPinnersBb = BishopRays[myState.pieceSquares[SpecificKing]] & (yourState.bbs[Bishop] | yourState.bbs[Queen]);
      BitBoardT myDiagPinnedPiecessBb = yourDiagPinnersBb ? (myKingAttackerSquares.pieceAttackers[Bishop] & myState.bbs[AllPieces]) : BbNone;
      BitBoardT yourOrthoPinnersBb = RookRays[myState.pieceSquares[SpecificKing]] & (yourState.bbs[Rook] | yourState.bbs[Queen]);
      BitBoardT myOrthoPinnedPiecessBb = yourOrthoPinnersBb ? (myKingAttackerSquares.pieceAttackers[Rook] & myState.bbs[AllPieces]) : BbNone;

      BitBoardT myPinnedPiecesBb = myDiagPinnedPiecessBb | myOrthoPinnedPiecessBb;

      // Fast path - if there are no possible checks and no possible discoveries on either king then we can simply count all moves
      if((allPiecesChecksBb | myBlockingPiecesBb | myPinnedPiecesBb) == BbNone) {
	stats.d1fast++;
	
	// Pawns
	stats.nodes += Bits::count(myAttacks.pawnsPushOne | myAttacks.pawnsPushTwo); // these are disjoint so can be counted together
	int pawnAttacksCount = Bits::count(myAttacks.pawnsLeftAttacks & allYourPiecesBb) + Bits::count(myAttacks.pawnsRightAttacks & allYourPiecesBb);
	stats.nodes += pawnAttacksCount;
	stats.captures += pawnAttacksCount;

	// Knights

	countMoves(stats, myAttacks.pieceAttacks[QueenKnight], allMyPiecesBb, allYourPiecesBb);
	countMoves(stats, myAttacks.pieceAttacks[KingKnight], allMyPiecesBb, allYourPiecesBb);

	// Bishops
	
	countMoves(stats, myAttacks.pieceAttacks[BlackBishop], allMyPiecesBb, allYourPiecesBb);
	countMoves(stats, myAttacks.pieceAttacks[WhiteBishop], allMyPiecesBb, allYourPiecesBb);

	// Rooks

	countMoves(stats, myAttacks.pieceAttacks[QueenRook], allMyPiecesBb, allYourPiecesBb);
	countMoves(stats, myAttacks.pieceAttacks[KingRook], allMyPiecesBb, allYourPiecesBb);

	// Queens
	
	countMoves(stats, myAttacks.pieceAttacks[SpecificQueen], allMyPiecesBb, allYourPiecesBb);
	
      } else {
	stats.d1slow++;
      
      // Count or evaluate moves.
      // We do full evaluation of moves that (might) generate check, including:
      //   - moves that directly generate check
      //   - moves from squares that might generate discovered check of either king

      // Moves from squares that might lead to discovered check of either side.
      //   - discovered check of our king is an invalid move which we'll check at level 0.
      //   - discovered check of your king is a check which we need to get further stats on.
      // TODO - we could filter this further to squares that are attacked by sliders (of the right type).
	BitBoardT discoverySquares = myBlockingPiecesBb | myPinnedPiecesBb; //myKingAttackerSquares.pieceAttackers[Queen] | yourKingAttackerSquares.pieceAttackers[Queen];
      //BitBoardT discoverySquares = BbAll;

      // Pawn pushes
      
      BitBoardT pawnCheckSquares = yourKingAttackerSquares.pieceAttackers[Pawn];
      BitBoardT discoveryPawns = myState.bbs[Pawn] & discoverySquares;
      
      BitBoardT discoveryPawnsPushOne = pawnsPushOne<Color>(discoveryPawns, allPiecesBb); // TODO could just pass in BbNone for allPiecesBb
      BitBoardT pawnsPushOneChecks = myAttacks.pawnsPushOne & (pawnCheckSquares | discoveryPawnsPushOne);
      
      // Do full evaluation of pawn pushes that (might) render check.
      perftImplPawnsPushOne<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, pawnsPushOneChecks);
      
      // Just count pawn moves that (definitely) won't render check.
      stats.nodes += Bits::count(myAttacks.pawnsPushOne & ~pawnsPushOneChecks);
      
      BitBoardT discoveryPawnsPushTwo = pawnsPushOne<Color>(discoveryPawnsPushOne, allPiecesBb); // TODO could just pass in BbNone for allPiecesBb
      BitBoardT pawnsPushTwoChecks = myAttacks.pawnsPushTwo & (pawnCheckSquares | discoveryPawnsPushTwo);
      
      // Do full evaluation of pawn pushes that (might) render check.
      perftImplPawnsPushTwo<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, pawnsPushTwoChecks);

      // Just count pawn moves that (definitely) won't render check.
      stats.nodes += Bits::count(myAttacks.pawnsPushTwo & ~pawnsPushTwoChecks);
      
      // Pawn captures

      BitBoardT pawnsCaptureLeft = myAttacks.pawnsLeftAttacks & allYourPiecesBb;
      BitBoardT discoveryPawnsCaptureLeft = pawnsLeftAttacks<Color>(discoveryPawns);
      BitBoardT pawnsCaptureLeftChecks = pawnsCaptureLeft & (pawnCheckSquares | discoveryPawnsCaptureLeft);
      
      // Do full evaluation of pawn captures that (might) render check.
      perftImplPawnsCaptureLeft<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, pawnsCaptureLeftChecks);
      
      // Just count pawn captures that (definitely) won't render check.
      u64 pawnsCaptureLeftCount = Bits::count(pawnsCaptureLeft & ~pawnsCaptureLeftChecks);
      stats.nodes += pawnsCaptureLeftCount;
      stats.captures += pawnsCaptureLeftCount;
      
      BitBoardT pawnsCaptureRight = myAttacks.pawnsRightAttacks & allYourPiecesBb;
      BitBoardT discoveryPawnsCaptureRight = pawnsRightAttacks<Color>(discoveryPawns);
      BitBoardT pawnsCaptureRightChecks = pawnsCaptureRight & (pawnCheckSquares | discoveryPawnsCaptureRight);
      
      // Do full evaluation of pawn captures that (might) render check.
      perftImplPawnsCaptureRight<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, pawnsCaptureRightChecks);
      
      // Just count pawn captures that (definitely) won't render check.
      u64 pawnsCaptureRightCount = Bits::count(pawnsCaptureRight & ~pawnsCaptureRightChecks);
      stats.nodes += pawnsCaptureRightCount;
      stats.captures += pawnsCaptureRightCount;
      
      // Piece moves

      // Knights

      BitBoardT knightCheckSquares = yourKingAttackerSquares.pieceAttackers[Knight];
      
      SquareT queenKnightSquare = myState.pieceSquares[QueenKnight];
      BitBoardT queenKnightBb = bbForSquare(queenKnightSquare);

      BitBoardT queenKnightAttacksBb = myAttacks.pieceAttacks[QueenKnight];
      BitBoardT queenKnightMovesBb = queenKnightAttacksBb & ~allMyPiecesBb;

      // Do full evaluation of moves that might render check.
      BitBoardT queenKnightCheckMovesBb = (queenKnightBb & discoverySquares) ? queenKnightAttacksBb : (queenKnightMovesBb & knightCheckSquares);
      perftImplSpecificPieceMoves<Color, QueenKnight, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, queenKnightSquare, queenKnightCheckMovesBb, allYourPiecesBb, allPiecesBb);

      // Just count moves that (definitely) won't render check.
      BitBoardT queenKnightNonCheckMovesBb = queenKnightMovesBb & ~queenKnightCheckMovesBb;
      stats.nodes += Bits::count(queenKnightNonCheckMovesBb);
      stats.captures += Bits::count(queenKnightNonCheckMovesBb & allYourPiecesBb);
      
      SquareT kingKnightSquare = myState.pieceSquares[KingKnight];
      BitBoardT kingKnightBb = bbForSquare(kingKnightSquare);

      BitBoardT kingKnightAttacksBb = myAttacks.pieceAttacks[KingKnight];
      BitBoardT kingKnightMovesBb = kingKnightAttacksBb & ~allMyPiecesBb;

      // Do full evaluation of moves that might render check.
      BitBoardT kingKnightCheckMovesBb = (kingKnightBb & discoverySquares) ? kingKnightAttacksBb : (kingKnightMovesBb & knightCheckSquares);
      perftImplSpecificPieceMoves<Color, KingKnight, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, kingKnightSquare, kingKnightCheckMovesBb, allYourPiecesBb, allPiecesBb);

      // Just count moves that (definitely) won't render check.
      BitBoardT kingKnightNonCheckMovesBb = kingKnightMovesBb & ~kingKnightCheckMovesBb;
      stats.nodes += Bits::count(kingKnightNonCheckMovesBb);
      stats.captures += Bits::count(kingKnightNonCheckMovesBb & allYourPiecesBb);
      //perftImplSpecificPieceMoves<Color, KingKnight, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, myState.pieceSquares[KingKnight], myAttacks.pieceAttacks[KingKnight], allYourPiecesBb, allPiecesBb);

      // Bishops
      
      perftImplSpecificPieceMoves<Color, BlackBishop, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, myState.pieceSquares[BlackBishop], myAttacks.pieceAttacks[BlackBishop], allYourPiecesBb, allPiecesBb);

      perftImplSpecificPieceMoves<Color, WhiteBishop, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, myState.pieceSquares[WhiteBishop], myAttacks.pieceAttacks[WhiteBishop], allYourPiecesBb, allPiecesBb);

      // Rooks

      perftImplSpecificPieceMoves<Color, QueenRook, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, myState.pieceSquares[QueenRook], myAttacks.pieceAttacks[QueenRook], allYourPiecesBb, allPiecesBb);

      perftImplSpecificPieceMoves<Color, KingRook, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, myState.pieceSquares[KingRook], myAttacks.pieceAttacks[KingRook], allYourPiecesBb, allPiecesBb);

      // Queens

      perftImplSpecificPieceMoves<Color, SpecificQueen, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, myState.pieceSquares[SpecificQueen], myAttacks.pieceAttacks[SpecificQueen], allYourPiecesBb, allPiecesBb);

      }
      // Repeated!
      //BitBoardT discoverySquares = myKingAttackerSquares.pieceAttackers[Queen] | yourKingAttackerSquares.pieceAttackers[Queen];

      // TODO other promo pieces
      if(MyBoardTraitsT::hasPromos) {
	if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
	  perftImplSpecificPieceMoves<Color, PromoQueen, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, myState.pieceSquares[PromoQueen], myAttacks.pieceAttacks[PromoQueen], allYourPiecesBb, allPiecesBb);
	}
      }

      // King always present; king cannot deliver check and king can always move to non-attacked squares so it's an easy count
      // Need your attacks for king moves and for castling so handle them together (and try to avoid the expensive move generation).
      BitBoardT kingMovesBb = myAttacks.pieceAttacks[SpecificKing] & ~allMyPiecesBb;

      // Special handling for a single king move -
      //   if the king has only one move and is not in check now then there cannot be any slider attacks so we just need to check pawns, knights and king.
      // Actually that's not true - there can be slider checks in a direction other than the moving direction, doh!
      // Mmm, this is miscounting; not sure why :( Think more.
      if(true) {
      if(kingMovesBb != BbNone) {
	stats.d1kinghasmoves++;
	BitBoardT yourPawnAndKnightAttacksBb = genPawnKnightKingAttacks<OtherColorT<Color>::value, YourBoardTraitsT>(yourState);
	kingMovesBb &= ~yourPawnAndKnightAttacksBb;

	if(kingMovesBb != BbNone) {
	  stats.d1kinghasmoves2++;
	
	  int nKingMoves = Bits::count(kingMovesBb);
	  if(nKingMoves == 1) {
	    stats.d1kinghas1move++;
	  
	    // Check whether there are any slider attacks.
	    SquareT kingTo = Bits::lsb(kingMovesBb);

	    BitBoardT possibleDiagAttacksFromBb = BishopRays[kingTo];
	    BitBoardT yourDiagSlidersBb = yourState.bbs[Bishop] | yourState.bbs[Queen];
	    BitBoardT possibleOrthoAttacksFromBb = RookRays[kingTo];
	    BitBoardT yourOrthoSlidersBb = yourState.bbs[Rook] | yourState.bbs[Queen];

	    BitBoardT possibleSliderAttackersBb = (possibleDiagAttacksFromBb & yourDiagSlidersBb) | (possibleOrthoAttacksFromBb & yourOrthoSlidersBb);

	    if(possibleSliderAttackersBb == BbNone) {
	      stats.d1kingfast++;
	      stats.nodes += Bits::count(kingMovesBb);
	      stats.captures += Bits::count(kingMovesBb & allYourPiecesBb);
	      kingMovesBb = BbNone;
	    }
	    
	  }
	}
      }}
      
      // Castling and slow king path
      CastlingRightsT castlingRights = castlingRightsWithSpace<Color>(myState.castlingRights, allPiecesBb);
      if(kingMovesBb != BbNone || castlingRights != NoCastlingRights) {
	stats.d1kingslow++;
	PieceAttacksT yourAttacks = genPieceAttacks<OtherColorT<Color>::value, YourBoardTraitsT>(yourState, allPiecesBb);
	
	BitBoardT legalKingMovesBb = kingMovesBb & ~yourAttacks.allAttacks;
	stats.nodes += Bits::count(legalKingMovesBb);
	stats.captures += Bits::count(legalKingMovesBb & allYourPiecesBb);

	if(castlingRights) {
	  if((castlingRights & CanCastleQueenside) && (yourAttacks.allAttacks & CastlingTraitsT<Color, CanCastleQueenside>::CastlingThruCheckBbMask) == BbNone) {
	    perftImplCastlingMove<Color, CanCastleQueenside, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1);
	  }
	  
	  if((castlingRights & CanCastleKingside) && (yourAttacks.allAttacks & CastlingTraitsT<Color, CanCastleKingside>::CastlingThruCheckBbMask) == BbNone) {
	    perftImplCastlingMove<Color, CanCastleKingside, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1);
	  }	
	}
      }

      // Pawn en-passant captures
      // TODO - could optimise this further but it's a marginal case.
      if(yourState.epSquare) {
	BitBoardT epSquareBb = bbForSquare(yourState.epSquare);
	
	perftImplPawnEpCaptureLeft<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, myAttacks.pawnsLeftAttacks & epSquareBb);
	perftImplPawnEpCaptureRight<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, 1, myAttacks.pawnsRightAttacks & epSquareBb);
      }

    }
      
    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT>
    inline void perftImpl0(PerftStatsT& stats, const BoardT& board, const SquareT moveTo, const MoveTypeT moveType) {
      stats.d0nodes++;
      
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

      if(moveType == CaptureMove) {
	stats.captures++;
      }

      if(moveType == EpCaptureMove) {
	stats.captures++;
	stats.eps++;
      }

      if(moveType == CastlingMove) {
	stats.castles++;
      }

      // Is my king in check?
      SquareAttackersT myKingAttackers = genSquareAttackers<OtherColorT<Color>::value, MyBoardTraitsT>(myState.pieceSquares[SpecificKing], yourState, allPiecesBb);
      BitBoardT allMyKingAttackers = myKingAttackers.pieceAttackers[AllPieces];
      if( allMyKingAttackers != 0) {
	stats.checks++;

	// If the moved piece is not attacking the king then this is a discovered check
	if((bbForSquare(moveTo) & allMyKingAttackers) == 0) {
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
    inline void perftImpl(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT moveTo, const MoveTypeT moveType) {

      if(depthToGo == 0) {
	// This is a leaf node; gather stats.
	perftImpl0<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, moveTo, moveType);
      } else if(depthToGo == 1) {
	// Try to count moves rather than doing full move execution for each move.
	perftImpl1<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, moveTo, moveType);
      } else {
	// Do the long haul for each move.
	perftImplFull<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, moveTo, moveType);
      }
    }
      
#ifdef IGNORE_UNTIL_IMPLEMENTING_PROMOS
    typedef void PerftImplFn(PerftStatsT& stats, const BoardT& board, const int depthToGo, const MoveTypeT moveType);

    template <ColorT Color>
    struct PerftImplDispatcherT {
      static const PerftImplFn* DispatchTable[];
    };

#include <boost/preprocessor/iteration/local.hpp>

#define BOOST_PP_LOCAL_MACRO(n) \
    perftImplTemplate<Color, (n), false>/*perftImplTemplate<Color, AllPiecesPresentFlags, true>*/,

#define BOOST_PP_LOCAL_LIMITS (0, MAX_PIECE_PRESENT_FLAGS)
    
    template <ColorT Color>
    const PerftImplFn* PerftImplDispatcherT<Color>::DispatchTable[] = {
#include BOOST_PP_LOCAL_ITERATE()
    };

    // Disappointingly the clever template specialisation produces code that runs slower than without???
#define PERFT_DIRECT_DISPATCH
    template <ColorT Color = false>
    inline void perftImpl(PerftStatsT& stats, const BoardT& board, const int depthToGo, const CaptureT moveType) {
#ifdef PERFT_DIRECT_DISPATCH
      perftImplTemplate<Color>(stats, board, depthToGo, moveType);
#else
      const ColorStateT& myState = board.pieces[Color];
      // if(myState.piecesPresent == StartingPiecesPresentFlags) {
      // 	perftImplTemplate<Color, StartingPiecesPresentFlags, false>(stats, board, depthToGo, moveType);
      // } else {
      // 	perftImplTemplate<Color>(stats, board, depthToGo, moveType);
      // }
      PerftImplDispatcherT<Color>::DispatchTable[myState.piecesPresent](stats, board, depthToGo, moveType);
#endif
    }

#endif //def IGNORE_UNTIL_IMPLEMENTING_PROMOS
    
    template <ColorT Color, typename MyBoardTraitsT, typename YourBoardTraitsT> inline PerftStatsT perft(const BoardT& board, const int depthToGo) {
      PerftStatsT stats = {0};

      perftImpl<Color, MyBoardTraitsT, YourBoardTraitsT>(stats, board, depthToGo, InvalidSquare, PushMove);

      return stats;
    }

  } // namespace Perf
} // namespace Chess

#endif //ndef PERFT_HPP
