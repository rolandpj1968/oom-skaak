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

  typedef u16 PiecePresentFlagsT;

  static const PiecePresentFlagsT PawnsPresentFlag = ((PiecePresentFlagsT)1 << PawnsPresentShift);
  static const PiecePresentFlagsT KingKnightPresentFlag = ((PiecePresentFlagsT)1 << KingKnightPresentShift);
  static const PiecePresentFlagsT BlackBishopPresentFlag = ((PiecePresentFlagsT)1 << BlackBishopPresentShift);
  static const PiecePresentFlagsT WhiteBishopPresentFlag = ((PiecePresentFlagsT)1 << WhiteBishopPresentShift);
  static const PiecePresentFlagsT QueenRookPresentFlag = ((PiecePresentFlagsT)1 << QueenRookPresentShift);
  static const PiecePresentFlagsT KingRookPresentFlag = ((PiecePresentFlagsT)1 << KingRookPresentShift);
  static const PiecePresentFlagsT QueenPresentFlag = ((PiecePresentFlagsT)1 << QueenPresentShift);
  static const PiecePresentFlagsT PromoQueenPresentFlag = ((PiecePresentFlagsT)1 << PromoQueenPresentShift);
  static const PiecePresentFlagsT PromoPiecesPresentFlag = ((PiecePresentFlagsT)1 << PromoPiecesPresentShift);
  static const PiecePresentFlagsT AllPiecesPresentFlags = ((PiecePresentFlagsT)1 << PiecePresentLimitShift) - 1;

}

#endif //ndef TYPES_H
