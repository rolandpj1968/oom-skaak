#ifndef TYPES_HPP
#define TYPES_HPP

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

  template <ColorT color> struct OtherColorT { static const ColorT value; };
  template <> struct OtherColorT<White> { static const ColorT value = Black; };
  template <> struct OtherColorT<Black> { static const ColorT value = White; };
  
  typedef u64 BitBoardT;

  const BitBoardT BbNone = (BitBoardT)0;
  const BitBoardT BbAll = ~BbNone;
  const BitBoardT BbOne = (BitBoardT)1ULL;

  const BitBoardT FileA = 0x0101010101010101ULL;
  const BitBoardT FileB = 0x0202020202020202ULL;
  const BitBoardT FileC = 0x0404040404040404ULL;
  const BitBoardT FileD = 0x0808080808080808ULL;
  const BitBoardT FileE = 0x1010101010101010ULL;
  const BitBoardT FileF = 0x2020202020202020ULL;
  const BitBoardT FileG = 0x4040404040404040ULL;
  const BitBoardT FileH = 0x8080808080808080ULL;

  const BitBoardT FileBbs[8] = { FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH };
  
  const BitBoardT Rank1 = 0x00000000000000ffULL;
  const BitBoardT Rank2 = 0X000000000000ff00ULL;
  const BitBoardT Rank3 = 0X0000000000ff0000ULL;
  const BitBoardT Rank4 = 0X00000000ff000000ULL;
  const BitBoardT Rank5 = 0X000000ff00000000ULL;
  const BitBoardT Rank6 = 0X0000ff0000000000ULL;
  const BitBoardT Rank7 = 0X00ff000000000000ULL;
  const BitBoardT Rank8 = 0Xff00000000000000ULL;

  const BitBoardT RankBbs[8] = { Rank1, Rank2, Rank3, Rank4, Rank5, Rank6, Rank7, Rank8 };

  const BitBoardT EdgeSquaresBb = FileA | FileH | Rank1 | Rank8;

  typedef u8 SquareT;

  const SquareT A1 = 0,    B1 = A1+1, C1 = A1+2, D1 = A1+3, E1 = A1+4, F1 = A1+5, G1 = A1+6, H1 = A1+7;
  const SquareT A2 = A1+8, B2 = A2+1, C2 = A2+2, D2 = A2+3, E2 = A2+4, F2 = A2+5, G2 = A2+6, H2 = A2+7;
  const SquareT A3 = A2+8, B3 = A3+1, C3 = A3+2, D3 = A3+3, E3 = A3+4, F3 = A3+5, G3 = A3+6, H3 = A2+7;
  const SquareT A4 = A3+8, B4 = A4+1, C4 = A4+2, D4 = A4+3, E4 = A4+4, F4 = A4+5, G4 = A4+6, H4 = A2+7;
  const SquareT A5 = A4+8, B5 = A5+1, C5 = A5+2, D5 = A5+3, E5 = A5+4, F5 = A5+5, G5 = A5+6, H5 = A5+7;
  const SquareT A6 = A5+8, B6 = A6+1, C6 = A6+2, D6 = A6+3, E6 = A6+4, F6 = A6+5, G6 = A6+6, H6 = A6+7;
  const SquareT A7 = A6+8, B7 = A7+1, C7 = A7+2, D7 = A7+3, E7 = A7+4, F7 = A7+5, G7 = A7+6, H7 = A7+7;
  const SquareT A8 = A7+8, B8 = A8+1, C8 = A8+2, D8 = A8+3, E8 = A8+4, F8 = A8+5, G8 = A8+6, H8 = A8+7;

  const SquareT InvalidSquare = H8+1;

  inline u8 rankOf(SquareT square) {
    return square >> 3;
  }

  inline u8 fileOf(SquareT square) {
    return square & 0x7;
  }

  inline BitBoardT bbForSquare(const SquareT square) {
    return BbOne << square;
  }

  enum DirT {
    N,
    S,
    E,
    W,
    NE,
    NW,
    SE,
    SW,
    NDirs
  };

  enum PieceT {
    NoPiece,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    NPieceTypes,
    AllPieces = NoPiece,
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
                     //   We have to look at the (simple) piece type bb's (for example) to see what kind of piece it is.
                     // TODO - fix this!
    NSpecificPieceTypes,
  };

  const PieceT PieceForSpecificPiece[NSpecificPieceTypes] = {
    NoPiece,       // SpecificNoPiece,
    Pawn,          // SpecificPawn,
    Knight,        // QueenKnight,
    Knight,        // KingKnight,
    Bishop,        // BlackBishop,
    Bishop,        // WhiteBishop,
    Rook,          // QueenRook,
    Rook,          // KingRook,
    Queen,         // SpecificQueen,
    Queen,         // PromoQueen,
    King,          // SpecificKing,
    NoPiece,       // TODO - OtherPromoPiece, // Denotes a promo piece that is not a queen or is a 2nd or subsequent piece promo.
  };

  template <SpecificPieceT SpecificPiece> struct PieceForSpecificPieceT { static const PieceT value; };
  template <> struct PieceForSpecificPieceT<SpecificNoPiece> { static const PieceT value = NoPiece; };
  template <> struct PieceForSpecificPieceT<SpecificPawn> { static const PieceT value = Pawn; };
  template <> struct PieceForSpecificPieceT<QueenKnight> { static const PieceT value = Knight; };
  template <> struct PieceForSpecificPieceT<KingKnight> { static const PieceT value = Knight; };
  template <> struct PieceForSpecificPieceT<BlackBishop> { static const PieceT value = Bishop; };
  template <> struct PieceForSpecificPieceT<WhiteBishop> { static const PieceT value = Bishop; };
  template <> struct PieceForSpecificPieceT<QueenRook> { static const PieceT value = Rook; };
  template <> struct PieceForSpecificPieceT<KingRook> { static const PieceT value = Rook; };
  template <> struct PieceForSpecificPieceT<SpecificQueen> { static const PieceT value = Queen; };
  template <> struct PieceForSpecificPieceT<PromoQueen> { static const PieceT value = Queen; };
  template <> struct PieceForSpecificPieceT<SpecificKing> { static const PieceT value = King; };
  // TODO - OtherPromoPiece, // Denotes a promo piece that is not a queen or is a 2nd or subsequent piece promo.

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

  static const int NPawns = 8;

  enum PushOrCaptureT {
    Push,
    Capture
  };

  enum CastlingRightsT {
    NoCastlingRights = 0,
    CanCastleQueenside = 1,
    CanCastleKingside = 2,
  };

  enum MoveTypeT {
    PushMove = 0,
    CaptureMove,
    EpCaptureMove,
    CastlingMove
  };

  struct MoveInfoT {
    MoveTypeT moveType;
    // For castling this is the king 'from' square.
    SquareT from;
    // For castling this is the king 'to' square;
    SquareT to;
    // SpecificNoPiece if not a capture.
    SpecificPieceT capturedSpecificPiece;
    // Generally same as 'to' for a capture except for en-passent.
    // InvalidSquare is this is not a capture.
    SquareT captureSq;
    // True iff the moved piece delivers check from the 'to' square.
    bool isDirectCheck;
    // The square containing the piece delivering discovered check.
    // InvalidSquare if not in check.
    // For castling giving check this is the rook 'to' square.
    SquareT discoveredCheckerSq;

    // Generic ctor
    MoveInfoT(MoveTypeT moveType, SquareT from, SquareT to, SpecificPieceT capturedSpecificPiece, SquareT captureSq, bool isDirectCheck, SquareT discoveredCheckerSq):
      moveType(moveType), from(from), to(to), capturedSpecificPiece(capturedSpecificPiece), captureSq(captureSq), isDirectCheck(isDirectCheck), discoveredCheckerSq(discoveredCheckerSq)
    {}

    // Push move ctor
    MoveInfoT(SquareT from, SquareT to, bool isDirectCheck = false, SquareT discoveredCheckSq = InvalidSquare) :
      MoveInfoT(PushMove, from, to, SpecificNoPiece, InvalidSquare, isDirectCheck, discoveredCheckSq)
    {}

    // Capture move ctor
    MoveInfoT(SquareT from, SquareT to, SpecificPieceT capturedSpecificPiece, bool isDirectCheck = false, SquareT discoveredCheckSq = InvalidSquare) :
      MoveInfoT(CaptureMove, from, to, capturedSpecificPiece, to, isDirectCheck, discoveredCheckSq)
    {}

    // Temporary working ctor
    MoveInfoT(MoveTypeT moveType, SquareT to): moveType(moveType), to(to) {}
  };
}

#endif //ndef TYPES_HPP
