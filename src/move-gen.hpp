#ifndef MOVE_GEN_HPP
#define MOVE_GEN_HPP

#include "types.hpp"
#include "bits.hpp"
#include "board.hpp"

namespace Chess {

  using namespace Board;
  
  namespace MoveGen {

    struct PieceAttacksT {
      // Pawn attacks (and moves) - single bit board for all pawns for each move type.
      BitBoardT pawnsLeftAttacks;
      BitBoardT pawnsRightAttacks;
      BitBoardT pawnsPushOne;
      BitBoardT pawnsPushTwo;

      // Piece moves
      BitBoardT pieceAttacks[NPieces];
      
      // Uncommon promo piece moves - one for each pawn - one for each promo piece except 2nd queen.
      BitBoardT promoPieceMoves[NPawns];
      
      BitBoardT allAttacks;
    };

    struct SquareAttackersT {
      // Attacks on a particular square for each piece type (of a particular color).
      BitBoardT pieceAttackers[NPieceTypes];
    };

    struct PiecePinMasksT {
      // Pawn pin masks - single bit board for all pawns for each move type.
      BitBoardT pawnsLeftPinMask;
      BitBoardT pawnsRightPinMask;
      BitBoardT pawnsPushOnePinMask;
      BitBoardT pawnsPushTwoPinMask;

      // Per-piece pin masks
      BitBoardT piecePinMasks[NPieces];
      
      // Uncommon promo piece moves - one for each pawn - one for each promo piece except 2nd queen.
      // BitBoardT promoPiecePinMasks[NPawns];
      
      // BitBoardT allPinMasks;
    };

    struct DirectCheckMasksT {
      // Pawn (direct) check squares
      BitBoardT pawnChecksBb;
      // Knight (direct) check squares
      BitBoardT knightChecksBb;
      // Bishop (direct) check squares
      BitBoardT bishopChecksBb;
      // Rook (direct) check squares
      BitBoardT rookChecksBb;

      DirectCheckMasksT():
	pawnChecksBb(BbNone), knightChecksBb(BbNone), bishopChecksBb(BbNone), rookChecksBb(BbNone) {}

      DirectCheckMasksT(const BitBoardT pawnChecksBb, const BitBoardT knightChecksBb, const BitBoardT bishopChecksBb, const BitBoardT rookChecksBb):
	pawnChecksBb(pawnChecksBb), knightChecksBb(knightChecksBb), bishopChecksBb(bishopChecksBb), rookChecksBb(rookChecksBb) {}
    };

    // Note that dicovered check is relatively easy to detect.
    // Queens can never move with discovered check, since they would have been checking the other king already, which is an invalid position - king can be captured.
    // Bishops can never be blocking a diagonal discovered check, and rooks can never be blocking an orthogonal discovered check for the same reason.
    // If a knight is blocking check then any move is a discovered check.
    // Pawns and kings are trickier.
    struct DiscoveredCheckMasksT {
      // From square discovery masks for knights and rooks.
      BitBoardT diagDiscoveryPiecesBb;
      // From square discovery masks for knights and bishops.
      BitBoardT orthogDiscoveryPiecesBb;

      // TODO pawns and king

      DiscoveredCheckMasksT():
	diagDiscoveryPiecesBb(BbNone), orthogDiscoveryPiecesBb(BbNone) {}
      
      DiscoveredCheckMasksT(const BitBoardT diagDiscoveryPiecesBb, const BitBoardT orthogDiscoveryPiecesBb):
	diagDiscoveryPiecesBb(diagDiscoveryPiecesBb), orthogDiscoveryPiecesBb(orthogDiscoveryPiecesBb) {}
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
      BitBoardT pieceMoves[NPieces];

      DirectCheckMasksT directChecks;
      DiscoveredCheckMasksT discoveredChecks;
      
      // TODO - construct properly
      LegalMovesT():
	isIllegalPos(false), nChecks(0), canCastleFlags(NoCastlingRights), pawnMoves()/*, PieceMoves???*/ {}
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
      static const PieceT TheRook = QueenRook;
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
      static const PieceT TheRook = KingRook;
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
      static const PieceT TheRook = QueenRook;
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
      static const PieceT TheRook = KingRook;
      static const SquareT RookFrom = H8;
      static const SquareT RookTo = F8;
    };

    //
    // Static attack tables for pawn, knight and king attacks
    //

    const BitBoardT PawnAttackers[NColors][64+1] = {
      // White pawn attackers
      {
	0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
	0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
	0x0000000000000002, 0x0000000000000005, 0x000000000000000a, 0x0000000000000014,
	0x0000000000000028, 0x0000000000000050, 0x00000000000000a0, 0x0000000000000040,
	0x0000000000000200, 0x0000000000000500, 0x0000000000000a00, 0x0000000000001400,
	0x0000000000002800, 0x0000000000005000, 0x000000000000a000, 0x0000000000004000,
	0x0000000000020000, 0x0000000000050000, 0x00000000000a0000, 0x0000000000140000,
	0x0000000000280000, 0x0000000000500000, 0x0000000000a00000, 0x0000000000400000,
	0x0000000002000000, 0x0000000005000000, 0x000000000a000000, 0x0000000014000000,
	0x0000000028000000, 0x0000000050000000, 0x00000000a0000000, 0x0000000040000000,
	0x0000000200000000, 0x0000000500000000, 0x0000000a00000000, 0x0000001400000000,
	0x0000002800000000, 0x0000005000000000, 0x000000a000000000, 0x0000004000000000,
	0x0000020000000000, 0x0000050000000000, 0x00000a0000000000, 0x0000140000000000,
	0x0000280000000000, 0x0000500000000000, 0x0000a00000000000, 0x0000400000000000,
	0x0002000000000000, 0x0005000000000000, 0x000a000000000000, 0x0014000000000000,
	0x0028000000000000, 0x0050000000000000, 0x00a0000000000000, 0x0040000000000000,
	BbNone, // InvalidSquare
      },
      // Black pawn attackers
      {
	0x0000000000000200, 0x0000000000000500, 0x0000000000000a00, 0x0000000000001400,
	0x0000000000002800, 0x0000000000005000, 0x000000000000a000, 0x0000000000004000,
	0x0000000000020000, 0x0000000000050000, 0x00000000000a0000, 0x0000000000140000,
	0x0000000000280000, 0x0000000000500000, 0x0000000000a00000, 0x0000000000400000,
	0x0000000002000000, 0x0000000005000000, 0x000000000a000000, 0x0000000014000000,
	0x0000000028000000, 0x0000000050000000, 0x00000000a0000000, 0x0000000040000000,
	0x0000000200000000, 0x0000000500000000, 0x0000000a00000000, 0x0000001400000000,
	0x0000002800000000, 0x0000005000000000, 0x000000a000000000, 0x0000004000000000,
	0x0000020000000000, 0x0000050000000000, 0x00000a0000000000, 0x0000140000000000,
	0x0000280000000000, 0x0000500000000000, 0x0000a00000000000, 0x0000400000000000,
	0x0002000000000000, 0x0005000000000000, 0x000a000000000000, 0x0014000000000000,
	0x0028000000000000, 0x0050000000000000, 0x00a0000000000000, 0x0040000000000000,
	0x0200000000000000, 0x0500000000000000, 0x0a00000000000000, 0x1400000000000000,
	0x2800000000000000, 0x5000000000000000, 0xa000000000000000, 0x4000000000000000,
	0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
	0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
	BbNone, // InvalidSquare
      },
    };

    const BitBoardT KnightAttacks[64+1] = { 
      0x0000000000020400ULL, 0x0000000000050800ULL, 0x00000000000a1100ULL, 0x0000000000142200ULL,
      0x0000000000284400ULL, 0x0000000000508800ULL, 0x0000000000a01000ULL, 0x0000000000402000ULL,
      0x0000000002040004ULL, 0x0000000005080008ULL, 0x000000000a110011ULL, 0x0000000014220022ULL,
      0x0000000028440044ULL, 0x0000000050880088ULL, 0x00000000a0100010ULL, 0x0000000040200020ULL,
      0x0000000204000402ULL, 0x0000000508000805ULL, 0x0000000a1100110aULL, 0x0000001422002214ULL,
      0x0000002844004428ULL, 0x0000005088008850ULL, 0x000000a0100010a0ULL, 0x0000004020002040ULL,
      0x0000020400040200ULL, 0x0000050800080500ULL, 0x00000a1100110a00ULL, 0x0000142200221400ULL,
      0x0000284400442800ULL, 0x0000508800885000ULL, 0x0000a0100010a000ULL, 0x0000402000204000ULL,
      0x0002040004020000ULL, 0x0005080008050000ULL, 0x000a1100110a0000ULL, 0x0014220022140000ULL,
      0x0028440044280000ULL, 0x0050880088500000ULL, 0x00a0100010a00000ULL, 0x0040200020400000ULL,
      0x0204000402000000ULL, 0x0508000805000000ULL, 0x0a1100110a000000ULL, 0x1422002214000000ULL,
      0x2844004428000000ULL, 0x5088008850000000ULL, 0xa0100010a0000000ULL, 0x4020002040000000ULL,
      0x0400040200000000ULL, 0x0800080500000000ULL, 0x1100110a00000000ULL, 0x2200221400000000ULL,
      0x4400442800000000ULL, 0x8800885000000000ULL, 0x100010a000000000ULL, 0x2000204000000000ULL,
      0x0004020000000000ULL, 0x0008050000000000ULL, 0x00110a0000000000ULL, 0x0022140000000000ULL,
      0x0044280000000000ULL, 0x0088500000000000ULL, 0x0010a00000000000ULL, 0x0020400000000000ULL,
      BbNone, // InvalidSquare
    };

    const BitBoardT KingAttacks[64+1] = {
      0x0000000000000302ULL, 0x0000000000000705ULL, 0x0000000000000e0aULL, 0x0000000000001c14ULL,
      0x0000000000003828ULL, 0x0000000000007050ULL, 0x000000000000e0a0ULL, 0x000000000000c040ULL,
      0x0000000000030203ULL, 0x0000000000070507ULL, 0x00000000000e0a0eULL, 0x00000000001c141cULL,
      0x0000000000382838ULL, 0x0000000000705070ULL, 0x0000000000e0a0e0ULL, 0x0000000000c040c0ULL,
      0x0000000003020300ULL, 0x0000000007050700ULL, 0x000000000e0a0e00ULL, 0x000000001c141c00ULL,
      0x0000000038283800ULL, 0x0000000070507000ULL, 0x00000000e0a0e000ULL, 0x00000000c040c000ULL,
      0x0000000302030000ULL, 0x0000000705070000ULL, 0x0000000e0a0e0000ULL, 0x0000001c141c0000ULL,
      0x0000003828380000ULL, 0x0000007050700000ULL, 0x000000e0a0e00000ULL, 0x000000c040c00000ULL,
      0x0000030203000000ULL, 0x0000070507000000ULL, 0x00000e0a0e000000ULL, 0x00001c141c000000ULL,
      0x0000382838000000ULL, 0x0000705070000000ULL, 0x0000e0a0e0000000ULL, 0x0000c040c0000000ULL,
      0x0003020300000000ULL, 0x0007050700000000ULL, 0x000e0a0e00000000ULL, 0x001c141c00000000ULL,
      0x0038283800000000ULL, 0x0070507000000000ULL, 0x00e0a0e000000000ULL, 0x00c040c000000000ULL,
      0x0302030000000000ULL, 0x0705070000000000ULL, 0x0e0a0e0000000000ULL, 0x1c141c0000000000ULL,
      0x3828380000000000ULL, 0x7050700000000000ULL, 0xe0a0e00000000000ULL, 0xc040c00000000000ULL,
      0x0203000000000000ULL, 0x0507000000000000ULL, 0x0a0e000000000000ULL, 0x141c000000000000ULL,
      0x2838000000000000ULL, 0x5070000000000000ULL, 0xa0e0000000000000ULL, 0x40c0000000000000ULL,
      BbNone, // InvalidSquare - there should always be a king, but...
    };

    // Bishop rays
    const BitBoardT BishopRays[64+1] = {
      0x8040201008040200, 0x0080402010080500, 0x0000804020110a00, 0x0000008041221400,
      0x0000000182442800, 0x0000010204885000, 0x000102040810a000, 0x0102040810204000,
      0x4020100804020002, 0x8040201008050005, 0x00804020110a000a, 0x0000804122140014,
      0x0000018244280028, 0x0001020488500050, 0x0102040810a000a0, 0x0204081020400040,
      0x2010080402000204, 0x4020100805000508, 0x804020110a000a11, 0x0080412214001422,
      0x0001824428002844, 0x0102048850005088, 0x02040810a000a010, 0x0408102040004020,
      0x1008040200020408, 0x2010080500050810, 0x4020110a000a1120, 0x8041221400142241,
      0x0182442800284482, 0x0204885000508804, 0x040810a000a01008, 0x0810204000402010,
      0x0804020002040810, 0x1008050005081020, 0x20110a000a112040, 0x4122140014224180,
      0x8244280028448201, 0x0488500050880402, 0x0810a000a0100804, 0x1020400040201008,
      0x0402000204081020, 0x0805000508102040, 0x110a000a11204080, 0x2214001422418000,
      0x4428002844820100, 0x8850005088040201, 0x10a000a010080402, 0x2040004020100804,
      0x0200020408102040, 0x0500050810204080, 0x0a000a1120408000, 0x1400142241800000,
      0x2800284482010000, 0x5000508804020100, 0xa000a01008040201, 0x4000402010080402,
      0x0002040810204080, 0x0005081020408000, 0x000a112040800000, 0x0014224180000000,
      0x0028448201000000, 0x0050880402010000, 0x00a0100804020100, 0x0040201008040201,
    };

    // Rook rays
    const BitBoardT RookRays[64+1] = {
      0x01010101010101fe, 0x02020202020202fd, 0x04040404040404fb, 0x08080808080808f7,
      0x10101010101010ef, 0x20202020202020df, 0x40404040404040bf, 0x808080808080807f,
      0x010101010101fe01, 0x020202020202fd02, 0x040404040404fb04, 0x080808080808f708,
      0x101010101010ef10, 0x202020202020df20, 0x404040404040bf40, 0x8080808080807f80,
      0x0101010101fe0101, 0x0202020202fd0202, 0x0404040404fb0404, 0x0808080808f70808,
      0x1010101010ef1010, 0x2020202020df2020, 0x4040404040bf4040, 0x80808080807f8080,
      0x01010101fe010101, 0x02020202fd020202, 0x04040404fb040404, 0x08080808f7080808,
      0x10101010ef101010, 0x20202020df202020, 0x40404040bf404040, 0x808080807f808080,
      0x010101fe01010101, 0x020202fd02020202, 0x040404fb04040404, 0x080808f708080808,
      0x101010ef10101010, 0x202020df20202020, 0x404040bf40404040, 0x8080807f80808080,
      0x0101fe0101010101, 0x0202fd0202020202, 0x0404fb0404040404, 0x0808f70808080808,
      0x1010ef1010101010, 0x2020df2020202020, 0x4040bf4040404040, 0x80807f8080808080,
      0x01fe010101010101, 0x02fd020202020202, 0x04fb040404040404, 0x08f7080808080808,
      0x10ef101010101010, 0x20df202020202020, 0x40bf404040404040, 0x807f808080808080,
      0xfe01010101010101, 0xfd02020202020202, 0xfb04040404040404, 0xf708080808080808,
      0xef10101010101010, 0xdf20202020202020, 0xbf40404040404040, 0x7f80808080808080,
    };
    
    //
    // Rays - used to generate Magic Bitboard tables
    //
    extern BitBoardT Rays[8][64];

    //
    // Magic Bitboards for Rook and Bishop attacks
    //

    const BitBoardT RookBlockers[64+1] = {
      0x000101010101017EULL, 0x000202020202027CULL, 0x000404040404047AULL, 0x0008080808080876ULL,
      0x001010101010106EULL, 0x002020202020205EULL, 0x004040404040403EULL, 0x008080808080807EULL,
      0x0001010101017E00ULL, 0x0002020202027C00ULL, 0x0004040404047A00ULL, 0x0008080808087600ULL,
      0x0010101010106E00ULL, 0x0020202020205E00ULL, 0x0040404040403E00ULL, 0x0080808080807E00ULL,
      0x00010101017E0100ULL, 0x00020202027C0200ULL, 0x00040404047A0400ULL, 0x0008080808760800ULL,
      0x00101010106E1000ULL, 0x00202020205E2000ULL, 0x00404040403E4000ULL, 0x00808080807E8000ULL,
      0x000101017E010100ULL, 0x000202027C020200ULL, 0x000404047A040400ULL, 0x0008080876080800ULL,
      0x001010106E101000ULL, 0x002020205E202000ULL, 0x004040403E404000ULL, 0x008080807E808000ULL,
      0x0001017E01010100ULL, 0x0002027C02020200ULL, 0x0004047A04040400ULL, 0x0008087608080800ULL,
      0x0010106E10101000ULL, 0x0020205E20202000ULL, 0x0040403E40404000ULL, 0x0080807E80808000ULL,
      0x00017E0101010100ULL, 0x00027C0202020200ULL, 0x00047A0404040400ULL, 0x0008760808080800ULL,
      0x00106E1010101000ULL, 0x00205E2020202000ULL, 0x00403E4040404000ULL, 0x00807E8080808000ULL,
      0x007E010101010100ULL, 0x007C020202020200ULL, 0x007A040404040400ULL, 0x0076080808080800ULL,
      0x006E101010101000ULL, 0x005E202020202000ULL, 0x003E404040404000ULL, 0x007E808080808000ULL,
      0x7E01010101010100ULL, 0x7C02020202020200ULL, 0x7A04040404040400ULL, 0x7608080808080800ULL,
      0x6E10101010101000ULL, 0x5E20202020202000ULL, 0x3E40404040404000ULL, 0x7E80808080808000ULL,
      0x0, // InvalidSquare
    };
    
    const BitBoardT BishopBlockers[64+1] = {
      0x0040201008040200ULL, 0x0000402010080400ULL, 0x0000004020100A00ULL, 0x0000000040221400ULL,
      0x0000000002442800ULL, 0x0000000204085000ULL, 0x0000020408102000ULL, 0x0002040810204000ULL,
      0x0020100804020000ULL, 0x0040201008040000ULL, 0x00004020100A0000ULL, 0x0000004022140000ULL,
      0x0000000244280000ULL, 0x0000020408500000ULL, 0x0002040810200000ULL, 0x0004081020400000ULL,
      0x0010080402000200ULL, 0x0020100804000400ULL, 0x004020100A000A00ULL, 0x0000402214001400ULL,
      0x0000024428002800ULL, 0x0002040850005000ULL, 0x0004081020002000ULL, 0x0008102040004000ULL,
      0x0008040200020400ULL, 0x0010080400040800ULL, 0x0020100A000A1000ULL, 0x0040221400142200ULL,
      0x0002442800284400ULL, 0x0004085000500800ULL, 0x0008102000201000ULL, 0x0010204000402000ULL,
      0x0004020002040800ULL, 0x0008040004081000ULL, 0x00100A000A102000ULL, 0x0022140014224000ULL,
      0x0044280028440200ULL, 0x0008500050080400ULL, 0x0010200020100800ULL, 0x0020400040201000ULL,
      0x0002000204081000ULL, 0x0004000408102000ULL, 0x000A000A10204000ULL, 0x0014001422400000ULL,
      0x0028002844020000ULL, 0x0050005008040200ULL, 0x0020002010080400ULL, 0x0040004020100800ULL,
      0x0000020408102000ULL, 0x0000040810204000ULL, 0x00000A1020400000ULL, 0x0000142240000000ULL,
      0x0000284402000000ULL, 0x0000500804020000ULL, 0x0000201008040200ULL, 0x0000402010080400ULL,
      0x0002040810204000ULL, 0x0004081020400000ULL, 0x000A102040000000ULL, 0x0014224000000000ULL,
      0x0028440200000000ULL, 0x0050080402000000ULL, 0x0020100804020000ULL, 0x0040201008040200ULL,
      0x0, // InvalidSquare
    };

    const BitBoardT RookMagicBbMultipliers[64+1] = {
      0xa8002c000108020ULL, 0x6c00049b0002001ULL, 0x100200010090040ULL, 0x2480041000800801ULL, 0x280028004000800ULL,
      0x900410008040022ULL, 0x280020001001080ULL, 0x2880002041000080ULL, 0xa000800080400034ULL, 0x4808020004000ULL,
      0x2290802004801000ULL, 0x411000d00100020ULL, 0x402800800040080ULL, 0xb000401004208ULL, 0x2409000100040200ULL,
      0x1002100004082ULL, 0x22878001e24000ULL, 0x1090810021004010ULL, 0x801030040200012ULL, 0x500808008001000ULL,
      0xa08018014000880ULL, 0x8000808004000200ULL, 0x201008080010200ULL, 0x801020000441091ULL, 0x800080204005ULL,
      0x1040200040100048ULL, 0x120200402082ULL, 0xd14880480100080ULL, 0x12040280080080ULL, 0x100040080020080ULL,
      0x9020010080800200ULL, 0x813241200148449ULL, 0x491604001800080ULL, 0x100401000402001ULL, 0x4820010021001040ULL,
      0x400402202000812ULL, 0x209009005000802ULL, 0x810800601800400ULL, 0x4301083214000150ULL, 0x204026458e001401ULL,
      0x40204000808000ULL, 0x8001008040010020ULL, 0x8410820820420010ULL, 0x1003001000090020ULL, 0x804040008008080ULL,
      0x12000810020004ULL, 0x1000100200040208ULL, 0x430000a044020001ULL, 0x280009023410300ULL, 0xe0100040002240ULL,
      0x200100401700ULL, 0x2244100408008080ULL, 0x8000400801980ULL, 0x2000810040200ULL, 0x8010100228810400ULL,
      0x2000009044210200ULL, 0x4080008040102101ULL, 0x40002080411d01ULL, 0x2005524060000901ULL, 0x502001008400422ULL,
      0x489a000810200402ULL, 0x1004400080a13ULL, 0x4000011008020084ULL, 0x26002114058042ULL,
      0x0, // InvalidSquare
    };

    const BitBoardT BishopMagicBbMultipliers[64+1] = {
      0x89a1121896040240ULL, 0x2004844802002010ULL, 0x2068080051921000ULL, 0x62880a0220200808ULL, 0x4042004000000ULL,
      0x100822020200011ULL, 0xc00444222012000aULL, 0x28808801216001ULL, 0x400492088408100ULL, 0x201c401040c0084ULL,
      0x840800910a0010ULL, 0x82080240060ULL, 0x2000840504006000ULL, 0x30010c4108405004ULL, 0x1008005410080802ULL,
      0x8144042209100900ULL, 0x208081020014400ULL, 0x4800201208ca00ULL, 0xf18140408012008ULL, 0x1004002802102001ULL,
      0x841000820080811ULL, 0x40200200a42008ULL, 0x800054042000ULL, 0x88010400410c9000ULL, 0x520040470104290ULL,
      0x1004040051500081ULL, 0x2002081833080021ULL, 0x400c00c010142ULL, 0x941408200c002000ULL, 0x658810000806011ULL,
      0x188071040440a00ULL, 0x4800404002011c00ULL, 0x104442040404200ULL, 0x511080202091021ULL, 0x4022401120400ULL,
      0x80c0040400080120ULL, 0x8040010040820802ULL, 0x480810700020090ULL, 0x102008e00040242ULL, 0x809005202050100ULL,
      0x8002024220104080ULL, 0x431008804142000ULL, 0x19001802081400ULL, 0x200014208040080ULL, 0x3308082008200100ULL,
      0x41010500040c020ULL, 0x4012020c04210308ULL, 0x208220a202004080ULL, 0x111040120082000ULL, 0x6803040141280a00ULL,
      0x2101004202410000ULL, 0x8200000041108022ULL, 0x21082088000ULL, 0x2410204010040ULL, 0x40100400809000ULL,
      0x822088220820214ULL, 0x40808090012004ULL, 0x910224040218c9ULL, 0x402814422015008ULL, 0x90014004842410ULL,
      0x1000042304105ULL, 0x10008830412a00ULL, 0x2520081090008908ULL, 0x40102000a0a60140ULL,
      0x0, // InvalidSquare
    };
    
    const u8 RookMagicBbIndexBits[64+1] = {
      12, 11, 11, 11, 11, 11, 11, 12,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11,
      12, 11, 11, 11, 11, 11, 11, 12,
      0 // InvalidSquare
    };

    const u8 BishopMagicBbIndexBits[64+1] = {
      6, 5, 5, 5, 5, 5, 5, 6,
      5, 5, 5, 5, 5, 5, 5, 5,
      5, 5, 7, 7, 7, 7, 5, 5,
      5, 5, 7, 9, 9, 7, 5, 5,
      5, 5, 7, 9, 9, 7, 5, 5,
      5, 5, 7, 7, 7, 7, 5, 5,
      5, 5, 5, 5, 5, 5, 5, 5,
      6, 5, 5, 5, 5, 5, 5, 6,
      0 // InvalidSquare
    };
  
    extern BitBoardT RookMagicBbTable[64+1][4096];
    extern BitBoardT BishopMagicBbTable[64+1][1024];

    // Magic BB bishop attacks
    inline BitBoardT bishopAttacks(const SquareT square, const BitBoardT allPiecesBb) {
      const BitBoardT blockers = allPiecesBb & BishopBlockers[square];
      const auto magicBbKey = (blockers * BishopMagicBbMultipliers[square]) >> (64 - BishopMagicBbIndexBits[square]);
      return BishopMagicBbTable[square][magicBbKey];
    }

    // Magic BB rook attacks
    inline BitBoardT rookAttacks(const SquareT square, const BitBoardT allPiecesBb) {
      const BitBoardT blockers = allPiecesBb & RookBlockers[square];
      const auto magicBbKey = (blockers * RookMagicBbMultipliers[square]) >> (64 - RookMagicBbIndexBits[square]);
      return RookMagicBbTable[square][magicBbKey];
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
	    const PieceT checkingPiece = squarePiecePiece(board.board[checkingPieceSq]);
	    const BitBoardT diagAttacksFromMyKingBb = bishopAttacks(myKingSq, allPiecesBb);
	    if(allMyKingAttackersBb & diagAttacksFromMyKingBb) {
	      legalMoveMaskBb |= diagAttacksFromMyKingBb & yourAttacks.pieceAttacks[checkingPiece] & BishopRays[checkingPieceSq];
	    }
	    const BitBoardT orthogAttacksFromMyKingBb = rookAttacks(myKingSq, allPiecesBb);
	    if(allMyKingAttackersBb & orthogAttacksFromMyKingBb) {
	      legalMoveMaskBb |= orthogAttacksFromMyKingBb & yourAttacks.pieceAttacks[checkingPiece] & RookRays[checkingPieceSq];
	    }
	  }
	}

	return legalMoveMaskBb;
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

    template <ColorT Color>
    inline SquareT pawnPushOneTo2From(const SquareT square);
    template <> inline SquareT pawnPushOneTo2From<White>(const SquareT square) { return square - 8; }
    template <> inline SquareT pawnPushOneTo2From<Black>(const SquareT square) { return square + 8; }

    template <ColorT Color>
    struct PawnPushOneTo2FromFn {
      static SquareT fn(const SquareT from) { return pawnPushOneTo2From<Color>(from); }
    };

    template <ColorT Color>
    inline SquareT pawnPushTwoTo2From(const SquareT square);
    template <> inline SquareT pawnPushTwoTo2From<White>(const SquareT square) { return square - 16; }
    template <> inline SquareT pawnPushTwoTo2From<Black>(const SquareT square) { return square + 16; }

    template <ColorT Color>
    struct PawnPushTwoTo2FromFn {
      static SquareT fn(const SquareT from) { return pawnPushTwoTo2From<Color>(from); }
    };

    //
    // Pawn move rules are specialised for White and Black respectively.
    //
    
    template <ColorT> BitBoardT pawnsLeftAttacks(const BitBoardT pawns);
    template <ColorT> BitBoardT pawnsRightAttacks(const BitBoardT pawns);
    template <ColorT> BitBoardT pawnsPushOne(const BitBoardT pawns, const BitBoardT allPiecesBb);
    template <ColorT> BitBoardT pawnsPushTwo(const BitBoardT pawnOneMoves, const BitBoardT allPiecesBb);

    template <> inline BitBoardT pawnsLeftAttacks<White>(const BitBoardT pawns) {
      // Pawns on file A can't take left.
      return (pawns & ~FileA) << 7;
    }
    
    template <> inline BitBoardT pawnsRightAttacks<White>(const BitBoardT pawns) {
      // Pawns on file H can't take right.
      return (pawns & ~FileH) << 9;
    }
    
    template <> inline BitBoardT pawnsPushOne<White>(const BitBoardT pawns, const BitBoardT allPiecesBb) {
      // White pieces move up the board but are blocked by pieces of either color.
      return (pawns << 8) & ~allPiecesBb;
    }
    
    template <> inline BitBoardT pawnsPushTwo<White>(const BitBoardT pawnOneMoves, const BitBoardT allPiecesBb) {
      // Pawns that can reach the 3rd rank after a single move can move to the 4th rank too,
      //   unless blocked by pieces of either color.
      return ((pawnOneMoves & Rank3) << 8) & ~allPiecesBb;
    }

    template <> inline BitBoardT pawnsLeftAttacks<Black>(const BitBoardT pawns) {
      // Pawns on file A can't take left.
      return (pawns & ~FileA) >> 9;
    }
    
    template <> inline BitBoardT pawnsRightAttacks<Black>(const BitBoardT pawns) {
      // Pawns on file H can't take right.
      return (pawns & ~FileH) >> 7;
    }
    
    template <> inline BitBoardT pawnsPushOne<Black>(const BitBoardT pawns, const BitBoardT allPiecesBb) {
      // Black pieces move downp the board but are blocked by pieces of any color.
      return (pawns >> 8) & ~allPiecesBb;
    }
    
    template <> inline BitBoardT pawnsPushTwo<Black>(const BitBoardT pawnOneMoves, const BitBoardT allPiecesBb) {
      // Pawns that can reach the 6rd rank after a single move can move to the 5th rank too,
      //   unless blocked by pieces of either color.
      return ((pawnOneMoves & Rank6) >> 8) & ~allPiecesBb;
    }

    // Attack generation

    // Generate attacks/defenses for all pieces.
    // Note that move gen for a piece on InvalidSquare MUST always generate BbNone (and not SIGSEGV :P).
    // TODO - missing support for unusual promos.
    template <typename ColorTraitsT>
    inline PieceAttacksT genPieceAttacks(const ColorStateT& colorState, const BitBoardT allPiecesBb) {
      const ColorT Color = ColorTraitsT::Color;
      
      PieceAttacksT attacks = {0};

      // Pawns
      BitBoardT pawns = colorState.bbs[Pawn];
      attacks.pawnsLeftAttacks = pawnsLeftAttacks<Color>(pawns);
      attacks.pawnsRightAttacks = pawnsRightAttacks<Color>(pawns);
      
      attacks.allAttacks |= (attacks.pawnsLeftAttacks | attacks.pawnsRightAttacks);
      
      attacks.pawnsPushOne = pawnsPushOne<Color>(pawns, allPiecesBb);
      attacks.pawnsPushTwo = pawnsPushTwo<Color>(attacks.pawnsPushOne, allPiecesBb);

      // Knights
      
      SquareT queenKnightSquare = colorState.pieceSquares[QueenKnight];
      
      attacks.pieceAttacks[QueenKnight] = KnightAttacks[queenKnightSquare];
      
      attacks.allAttacks |= attacks.pieceAttacks[QueenKnight];
      
      SquareT kingKnightSquare = colorState.pieceSquares[KingKnight];
      
      attacks.pieceAttacks[KingKnight] = KnightAttacks[kingKnightSquare];
      
      attacks.allAttacks |= attacks.pieceAttacks[KingKnight];

      // Bishops

      SquareT blackBishopSquare = colorState.pieceSquares[BlackBishop];
      
      attacks.pieceAttacks[BlackBishop] = bishopAttacks(blackBishopSquare, allPiecesBb);
      
      attacks.allAttacks |= attacks.pieceAttacks[BlackBishop];
      
      SquareT whiteBishopSquare = colorState.pieceSquares[WhiteBishop];
      
      attacks.pieceAttacks[WhiteBishop] = bishopAttacks(whiteBishopSquare, allPiecesBb);
      
      attacks.allAttacks |= attacks.pieceAttacks[WhiteBishop];
      
      // Rooks

      SquareT queenRookSquare = colorState.pieceSquares[QueenRook];
      
      attacks.pieceAttacks[QueenRook] = rookAttacks(queenRookSquare, allPiecesBb);
      
      attacks.allAttacks |= attacks.pieceAttacks[QueenRook];
      
      SquareT kingRookSquare = colorState.pieceSquares[KingRook];
      
      attacks.pieceAttacks[KingRook] = rookAttacks(kingRookSquare, allPiecesBb);
      
      attacks.allAttacks |= attacks.pieceAttacks[KingRook];
      
      // Queen

      SquareT queenSquare = colorState.pieceSquares[TheQueen];
      
      attacks.pieceAttacks[TheQueen] = rookAttacks(queenSquare, allPiecesBb) | bishopAttacks(queenSquare, allPiecesBb);
      
      attacks.allAttacks |= attacks.pieceAttacks[TheQueen];

      // King - always 1 king and always present
      SquareT kingSquare = colorState.pieceSquares[TheKing];

      attacks.pieceAttacks[TheKing] = KingAttacks[kingSquare];

      attacks.allAttacks |= attacks.pieceAttacks[TheKing];

      // TODO - unusual promos
      if(ColorTraitsT::HasPromos) {
	if(true/*piecesPresent & PromoQueenPresentFlag*/) {
	  SquareT promoQueenSquare = colorState.pieceSquares[PromoQueen];
	  
	  attacks.pieceAttacks[PromoQueen] = rookAttacks(promoQueenSquare, allPiecesBb) | bishopAttacks(promoQueenSquare, allPiecesBb);
	  
	  attacks.allAttacks |= attacks.pieceAttacks[PromoQueen];
	}
      }
      
      return attacks;
    }

    // Generate attackers/defenders of a particular square.
    // Useful for check detection.
    template <typename ColorTraitsT>
    inline SquareAttackersT genSquareAttackers(const SquareT square, const ColorStateT& colorState, const BitBoardT allPiecesBb) {
      const ColorT Color = ColorTraitsT::Color;
      
      SquareAttackersT attackers = {0};

      // Pawns
      
      BitBoardT pawnAttackersBb = PawnAttackers[(size_t)Color][square] & colorState.bbs[Pawn];
      attackers.pieceAttackers[Pawn] = pawnAttackersBb;
      attackers.pieceAttackers[AllPieceTypes] |= pawnAttackersBb;

      // Knights
      
      BitBoardT knightAttackersBb = KnightAttacks[square] & colorState.bbs[Knight];
      attackers.pieceAttackers[Knight] = knightAttackersBb;
      attackers.pieceAttackers[AllPieceTypes] |= knightAttackersBb;

      // Diagonal sliders
      
      BitBoardT diagonalAttackerSquaresBb = bishopAttacks(square, allPiecesBb);

      BitBoardT bishopAttackers = diagonalAttackerSquaresBb & colorState.bbs[Bishop];
      attackers.pieceAttackers[Bishop] = bishopAttackers;

      BitBoardT diagonalQueenAttackers = diagonalAttackerSquaresBb & colorState.bbs[Queen];
      attackers.pieceAttackers[Queen] |= diagonalQueenAttackers;

      attackers.pieceAttackers[AllPieceTypes] |= bishopAttackers | diagonalQueenAttackers;

      // Orthogonal sliders

      BitBoardT orthogonalAttackerSquaresBb = rookAttacks(square, allPiecesBb);

      BitBoardT rookAttackers = orthogonalAttackerSquaresBb & colorState.bbs[Rook];
      attackers.pieceAttackers[Rook] = rookAttackers;

      BitBoardT orthogonalQueenAttackers = orthogonalAttackerSquaresBb & colorState.bbs[Queen];
      attackers.pieceAttackers[Queen] |= orthogonalQueenAttackers;

      attackers.pieceAttackers[AllPieceTypes] |= rookAttackers | orthogonalQueenAttackers;
	
      // King attacker

      BitBoardT kingAttackers = KingAttacks[square] & colorState.bbs[King];
      attackers.pieceAttackers[King] = kingAttackers;

      attackers.pieceAttackers[AllPieceTypes] |= kingAttackers;

      return attackers;
    }

    // Pinned move mask generation
    
    template <PieceTypeT PieceType>
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
      pinMasks.piecePinMasks[TheQueen] = BbAll;

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
      
      const SquareT myKingSq = myState.pieceSquares[TheKing];
      
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
      pinMasks.piecePinMasks[TheQueen] = genPinnedMoveMask<Queen>(myState.pieceSquares[TheQueen], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);

      // TODO other promo pieces
      if(ColorTraitsT::HasPromos) {
	if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
	  pinMasks.piecePinMasks[PromoQueen] = genPinnedMoveMask<Queen>(myState.pieceSquares[PromoQueen], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);
	}
      }
    }
    
    template <typename BoardTraitsT>
    inline PiecePinMasksT genPinMasks(const BoardT& board) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& myState = board.pieces[(size_t)Color];
      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieceTypes];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      const SquareT myKingSq = myState.pieceSquares[TheKing];

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

    template <typename BoardTraitsT>
    inline DiscoveredCheckMasksT genDiscoveryMasks(const BoardT& board) {
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& myState = board.pieces[(size_t)Color];
      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieceTypes];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      const SquareT yourKingSq = yourState.pieceSquares[TheKing];

      // Find my pieces that are blocking check - used for discovered check detection.
      const BitBoardT myDiagDiscoveryPiecesBb = genPinnedPiecesBb<Diagonal>(yourKingSq, allPiecesBb, allMyPiecesBb, myState);
      const BitBoardT myOrthogDiscoveryPiecesBb = genPinnedPiecesBb<Orthogonal>(yourKingSq, allPiecesBb, allMyPiecesBb, myState);

      return DiscoveredCheckMasksT(myDiagDiscoveryPiecesBb, myOrthogDiscoveryPiecesBb);
    }

    template <PieceT Piece>
    inline BitBoardT genLegalPieceMoves(const PieceAttacksT myAttacks, const BitBoardT legalMoveMaskBb, const PiecePinMasksT pinMasks, BitBoardT allMyPiecesBb) {
      return myAttacks.pieceAttacks[Piece] & legalMoveMaskBb & pinMasks.piecePinMasks[Piece] & ~allMyPiecesBb;
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
	const ColorStateT& yourState = board.pieces[(size_t)BoardTraitsT::OtherColor];
	const ColorStateT& myState = board.pieces[(size_t)BoardTraitsT::Color];
	const SquareT myKingSq = myState.pieceSquares[TheKing];
	  
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
      
      const ColorStateT& myState = board.pieces[(size_t)Color];
      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieceTypes];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      legalMoves.pawnMoves = genLegalPawnMoves<BoardTraitsT>(board, myAttacks, yourState.epSquare, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinMasks);
      
      legalMoves.pieceMoves[QueenKnight] = genLegalPieceMoves<QueenKnight>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);
      legalMoves.pieceMoves[KingKnight] = genLegalPieceMoves<KingKnight>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);

      legalMoves.pieceMoves[BlackBishop] = genLegalPieceMoves<BlackBishop>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);
      legalMoves.pieceMoves[WhiteBishop] = genLegalPieceMoves<WhiteBishop>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);
      
      legalMoves.pieceMoves[QueenRook] = genLegalPieceMoves<QueenRook>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);
      legalMoves.pieceMoves[KingRook] = genLegalPieceMoves<KingRook>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);

      legalMoves.pieceMoves[TheQueen] = genLegalPieceMoves<TheQueen>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);
      
      // TODO other promo pieces
      if(MyColorTraitsT::HasPromos) {
	if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
	  legalMoves.pieceMoves[PromoQueen] = genLegalPieceMoves<PromoQueen>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);
	}
      }
    }

    template <typename BoardTraitsT>
    inline CastlingRightsT genLegalCastlingFlags(const BoardT& board, const PieceAttacksT& yourAttacks, const BitBoardT allPiecesBb) {
      const ColorT Color = BoardTraitsT::Color;
      const ColorStateT& myState = board.pieces[(size_t)Color];

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
    inline BitBoardT genLegalKingMoves(const BoardT& board, const PieceAttacksT& yourAttacks, const BitBoardT allMyKingAttackersBb) {
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& myState = board.pieces[(size_t)Color];
      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieceTypes];
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

      const SquareT kingSq = myState.pieceSquares[TheKing];
      const BitBoardT legalKingMovesBb = KingAttacks[kingSq] & ~yourAttacks.allAttacks & ~illegalKingSquaresBb;

      return legalKingMovesBb & ~allMyPiecesBb;
    }

    template <ColorT Color>
    inline DirectCheckMasksT genDirectCheckMasks(const ColorStateT& yourState, const BitBoardT allPiecesBb) {
      const SquareT yourKingSq = yourState.pieceSquares[TheKing];

      const BitBoardT pawnChecksBb = PawnAttackers[(size_t)Color][yourKingSq];
      const BitBoardT knightChecksBb = KnightAttacks[yourKingSq];
      const BitBoardT bishopChecksBb = bishopAttacks(yourKingSq, allPiecesBb);
      const BitBoardT rookChecksBb = rookAttacks(yourKingSq, allPiecesBb);

      return DirectCheckMasksT(pawnChecksBb, knightChecksBb, bishopChecksBb, rookChecksBb);
    }
    
    template <typename BoardTraitsT>
    inline LegalMovesT genLegalMoves(const BoardT& board) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& myState = board.pieces[(size_t)Color];
      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieceTypes];
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

      const SquareT myKingSq = myState.pieceSquares[TheKing];
      const SquareAttackersT myKingAttackers = genSquareAttackers<YourColorTraitsT>(myKingSq, yourState, allPiecesBb);
      const BitBoardT allMyKingAttackersBb = myKingAttackers.pieceAttackers[AllPieceTypes];

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
	legalMoves.canCastleFlags = genLegalCastlingFlags<BoardTraitsT>(board, yourAttacks, allPiecesBb);
      }

      legalMoves.pieceMoves[TheKing] = genLegalKingMoves<BoardTraitsT>(board, yourAttacks, allMyKingAttackersBb);

      legalMoves.directChecks = genDirectCheckMasks<Color>(yourState, allPiecesBb);
      legalMoves.discoveredChecks = genDiscoveryMasks<BoardTraitsT>(board);
      
      return legalMoves;
    }

    extern int countAttacks(const PieceAttacksT& pieceAttacks, const BitBoardT filterOut = BbNone, const BitBoardT filterInPawnTakes = BbAll);
    
  } // namespace MoveGen

  
} // namespace Chess

  
#endif //ndef MOVE_GEN_HPP
