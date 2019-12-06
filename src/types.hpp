#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

namespace Chess {
  
  enum class ColorT: u8 {
    White,
    Black,
    NColors,
  };

  const ColorT White = ColorT::White;
  const ColorT Black = ColorT::Black;
  const size_t NColors = (size_t)ColorT::NColors;

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

  const char* const SquareStr[65] = {
    "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
    "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
    "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
    "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
    "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
    "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
    "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
    "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8",
    "**" // invalid square
  };

  inline u8 rankOf(SquareT square) {
    return square >> 3;
  }

  inline u8 fileOf(SquareT square) {
    return square & 0x7;
  }

#include <boost/preprocessor/iteration/local.hpp>
  // Lookup table for single square -> bitboard (including InvalidSquare -> BbNone)
  const BitBoardT BbForSquare[64+1] = {
#define BOOST_PP_LOCAL_MACRO(n) \
    BbOne << (n),
#define BOOST_PP_LOCAL_LIMITS (0, 63)
#include BOOST_PP_LOCAL_ITERATE()
    BbNone // InvalidSquare
  };
  
  inline BitBoardT bbForSquareLookup(const SquareT square) {
    return BbForSquare[square];
  }

  inline BitBoardT bbForSquareShift(const SquareT square) {
    return BbOne << square;
  }

  // Lookup and shift seem like a wash performance-wise so leaving this as lookup to get safe bbForSquare(InvalidSquare) -> BbNone
  inline BitBoardT bbForSquare(const SquareT square) {
    return bbForSquareLookup(square);
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

  enum PieceTypeT {
    NoPieceType,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    NPieceTypes,
    AllPieceTypes = NoPieceType,
  };

  enum PieceT {
    NoPiece,
    SomePawns,
    QueenKnight,
    KingKnight,
    BlackBishop,
    WhiteBishop,
    QueenRook,
    KingRook,
    TheQueen,
    PromoQueen,      // First queen promo piece - this captures the majority of actual promo's in real play.
    TheKing,
    OtherPromoPiece, // Denotes a promo piece that is not a queen or is a 2nd or subsequent piece promo.
                     //   We have to look at the (simple) piece type bb's (for example) to see what kind of piece it is.
                     // TODO - fix this!
    NPieces,
  };

  const PieceTypeT PieceTypeForPiece[NPieces] = {
    NoPieceType,       // NoPiece,
    Pawn,          // Pawn,
    Knight,        // QueenKnight,
    Knight,        // KingKnight,
    Bishop,        // BlackBishop,
    Bishop,        // WhiteBishop,
    Rook,          // QueenRook,
    Rook,          // KingRook,
    Queen,         // Queen,
    Queen,         // PromoQueen,
    King,          // King,
    NoPieceType,       // TODO - OtherPromoPiece, // Denotes a promo piece that is not a queen or is a 2nd or subsequent piece promo.
  };

  template <PieceT Piece> struct PieceTypeForPieceT { static const PieceTypeT value; };
  template <> struct PieceTypeForPieceT<NoPiece> { static const PieceTypeT value = NoPieceType; };
  template <> struct PieceTypeForPieceT<SomePawns> { static const PieceTypeT value = Pawn; };
  template <> struct PieceTypeForPieceT<QueenKnight> { static const PieceTypeT value = Knight; };
  template <> struct PieceTypeForPieceT<KingKnight> { static const PieceTypeT value = Knight; };
  template <> struct PieceTypeForPieceT<BlackBishop> { static const PieceTypeT value = Bishop; };
  template <> struct PieceTypeForPieceT<WhiteBishop> { static const PieceTypeT value = Bishop; };
  template <> struct PieceTypeForPieceT<QueenRook> { static const PieceTypeT value = Rook; };
  template <> struct PieceTypeForPieceT<KingRook> { static const PieceTypeT value = Rook; };
  template <> struct PieceTypeForPieceT<TheQueen> { static const PieceTypeT value = Queen; };
  template <> struct PieceTypeForPieceT<PromoQueen> { static const PieceTypeT value = Queen; };
  template <> struct PieceTypeForPieceT<TheKing> { static const PieceTypeT value = King; };
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

  // TODO consts
  struct MoveInfoT {
    MoveTypeT moveType;
    // For castling this is the king 'from' square.
    SquareT from;
    // For castling this is the king 'to' square;
    SquareT to;
    // NoPiece if not a capture.
    PieceT capturedPiece;
    // Generally same as 'to' for a capture except for en-passent.
    // InvalidSquare is this is not a capture.
    SquareT captureSq;
    // True iff the moved piece delivers check from the 'to' square.
    bool isDirectCheck;
    // // The square containing the piece delivering discovered check.
    // // InvalidSquare if not in check.
    // // For castling giving check this is the rook 'to' square.
    // SquareT discoveredCheckerSq;
    // True iff the moved piece uncovers discovered check.
    bool isDiscoveredCheck;

    // // Generic ctor
    // MoveInfoT(MoveTypeT moveType, SquareT from, SquareT to, PieceT capturedPiece, SquareT captureSq, bool isDirectCheck, SquareT discoveredCheckerSq):
    //   moveType(moveType), from(from), to(to), capturedPiece(capturedPiece), captureSq(captureSq), isDirectCheck(isDirectCheck), discoveredCheckerSq(discoveredCheckerSq)
    // {}

    // // Push move ctor
    // MoveInfoT(SquareT from, SquareT to, bool isDirectCheck = false, SquareT discoveredCheckSq = InvalidSquare) :
    //   MoveInfoT(PushMove, from, to, NoPiece, InvalidSquare, isDirectCheck, discoveredCheckSq)
    // {}

    // // Capture move ctor
    // MoveInfoT(SquareT from, SquareT to, PieceT capturedPiece, bool isDirectCheck = false, SquareT discoveredCheckSq = InvalidSquare) :
    //   MoveInfoT(CaptureMove, from, to, capturedPiece, to, isDirectCheck, discoveredCheckSq)
    // {}

    // Temporary working ctor
    MoveInfoT(const double tag, const MoveTypeT moveType, const SquareT from, const SquareT to, const bool isDirectCheck, const bool isDiscoveredCheck):
      moveType(moveType), from(from), to(to), isDirectCheck(isDirectCheck), isDiscoveredCheck(isDiscoveredCheck) {}
  };

  enum SliderDirectionT {
    Diagonal,
    Orthogonal,
    NSliderDirections
  };
}

#endif //ndef TYPES_HPP
