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
    QueenKnight,
    KingKnight,
    BlackBishop,
    WhiteBishop,
    QueenRook,
    KingRook,
    Queen,
    PromoQueen,  // 2nd queen after pawn promo
    King,
    NSpecificPieceTypes,
  }

  enum PiecePresentShiftsT {
    PawnsPresentShift,
    QueenKnightPresentShift,
    KingKnightPresentShift,
    BlackBishopPresentShift,
    WhiteBishopPresentShift,
    QueenRookPresentShift,
    KingRookPresentShift,
    QueenPresentShift,
    PromoQueenPresentShift,  // 2nd queen after pawn promo
    PromoPiecesPresentShift, // Sundry other promo pieces - 3rd+ queen(s) and/or under-promotions
    PiecePresentLimitShift,
  };

  typedef u16 PiecePresentBitmapT;

  static const PiecePresentBitmapT PawnsPresentBit = ((PiecePresentBitmapT)1 << PawnsPresentShift);
  static const PiecePresentBitmapT KingKnightPresentBit = ((PiecePresentBitmapT)1 << KingKnightPresentShift);
  static const PiecePresentBitmapT BlackBishopPresentBit = ((PiecePresentBitmapT)1 << BlackBishopPresentShift);
  static const PiecePresentBitmapT WhiteBishopPresentBit = ((PiecePresentBitmapT)1 << WhiteBishopPresentShift);
  static const PiecePresentBitmapT QueenRookPresentBit = ((PiecePresentBitmapT)1 << QueenRookPresentShift);
  static const PiecePresentBitmapT KingRookPresentBit = ((PiecePresentBitmapT)1 << KingRookPresentShift);
  static const PiecePresentBitmapT QueenPresentBit = ((PiecePresentBitmapT)1 << QueenPresentShift);
  static const PiecePresentBitmapT PromoQueenPresentBit = ((PiecePresentBitmapT)1 << PromoQueenPresentShift);
  static const PiecePresentBitmapT PromoPiecesPresentBit = ((PiecePresentBitmapT)1 << PromoPiecesPresentShift);
  static const PiecePresentBitmapT AllPiecesPresentBits = ((PiecePresentBitmapT)1 << PiecePresentLimitShift) - 1;

}

#endif //ndef TYPES_H
