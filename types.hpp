#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

namespace Chess {
  
  enum ColorT {
    Black,
    White,
    NColors,
  };

  typedef u64 BitBoardT;

  typedef u8 SquareT;

  static const SquareT InvalidSquare = (SquareT)64;

  enum PieceT {
    NoPiece,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    NPieceTypes,
  };

  enum SpecificPieceT {
    SpecificNoPiece,
    SpecificPawn,
    QueenKnight,
    KingKnight,
    BlackBishop,
    WhiteBishop,
    QueenRook,
    KingRook,
    SpecificQueen,
    PromoQueen,      // First queen promo piece - this captures the majority of actual promo's in real play.
    SpecificKing,
    OtherPromoPiece, // Denotes a promo piece that is not a queen or is a 2nd or subsequent piece promo.
                     //   We have to look at the (simple) piece type to see what kind of piece it is.
    NSpecificPieceTypes,
  };

  enum PiecePresentShiftsT {
    PawnsPresentShift,
    QueenKnightPresentShift,
    KingKnightPresentShift,
    BlackBishopPresentShift,
    WhiteBishopPresentShift,
    QueenRookPresentShift,
    KingRookPresentShift,
    QueenPresentShift,
    PromoQueenPresentShift,       // First queen promo piece.
    OtherPromoPiecesPresentShift, // Promo pieces that are not the first queen promo.
    PiecePresentLimitShift,
  };

  typedef u16 PiecePresentFlagsT;

  static const PiecePresentFlagsT PawnsPresentFlag = ((PiecePresentFlagsT)1 << PawnsPresentShift);
  static const PiecePresentFlagsT QueenKnightPresentFlag = ((PiecePresentFlagsT)1 << KingKnightPresentShift);
  static const PiecePresentFlagsT KingKnightPresentFlag = ((PiecePresentFlagsT)1 << KingKnightPresentShift);
  static const PiecePresentFlagsT BlackBishopPresentFlag = ((PiecePresentFlagsT)1 << BlackBishopPresentShift);
  static const PiecePresentFlagsT WhiteBishopPresentFlag = ((PiecePresentFlagsT)1 << WhiteBishopPresentShift);
  static const PiecePresentFlagsT QueenRookPresentFlag = ((PiecePresentFlagsT)1 << QueenRookPresentShift);
  static const PiecePresentFlagsT KingRookPresentFlag = ((PiecePresentFlagsT)1 << KingRookPresentShift);
  static const PiecePresentFlagsT QueenPresentFlag = ((PiecePresentFlagsT)1 << QueenPresentShift);
  static const PiecePresentFlagsT PromoQueenPresentFlag = ((PiecePresentFlagsT)1 << PromoQueenPresentShift);
  static const PiecePresentFlagsT OtherPromoPiecesPresentFlag = ((PiecePresentFlagsT)1 << OtherPromoPiecesPresentShift);
  static const PiecePresentFlagsT AllPiecesPresentFlags = ((PiecePresentFlagsT)1 << PiecePresentLimitShift) - 1;

  static const int NPawns = 8;
}

#endif //ndef TYPES_H
