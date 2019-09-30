#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

namespace Chess {
  
  enum ColorT {
    White,
    Black,
    NColors,
  };

  typedef u64 BitBoardT;

  typedef u8 SquareT;

  const SquareT A1 = 0,    B1 = A1+1, C1 = A1+2, D1 = A1+3, E1 = A1+4, F1 = A1+5, G1 = A1+6, H1 = A1+7;
  const SquareT A2 = A1+8, B2 = A2+1, C2 = A2+2, D2 = A2+3, E2 = A2+4, F2 = A2+5, G2 = A2+6, H2 = A2+7;
  const SquareT A3 = A2+8, B3 = A3+1, C3 = A3+2, D3 = A3+3, E3 = A3+4, F3 = A3+5, G3 = A3+6, H3 = A2+7;
  const SquareT A4 = A3+8, B4 = A4+1, C4 = A4+2, D4 = A4+3, E4 = A4+4, F4 = A4+5, G4 = A4+6, H4 = A2+7;
  const SquareT A5 = A4+8, B5 = A5+1, C5 = A5+2, D5 = A5+3, E5 = A5+4, F5 = A5+5, G5 = A5+6, H5 = A5+7;
  const SquareT A6 = A5+8, B6 = A6+1, C6 = A6+2, D6 = A6+3, E6 = A6+4, F6 = A6+5, G6 = A6+6, H6 = A6+7;
  const SquareT A7 = A6+8, B7 = A7+1, C7 = A7+2, D7 = A7+3, E7 = A7+4, F7 = A7+5, G7 = A7+6, H7 = A7+7;
  const SquareT A8 = A7+8, B8 = A8+1, C8 = A8+2, D8 = A8+3, E8 = A8+4, F8 = A8+5, G8 = A8+6, H8 = A8+7;

  inline BitBoardT bbForSquare(const SquareT square) {
    return (BitBoardT)1 << square;
  }

  enum PieceT {
    NoPiece,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    AllPieces,
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
