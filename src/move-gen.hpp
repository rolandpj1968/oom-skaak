#ifndef MOVE_GEN_HPP
#define MOVE_GEN_HPP

#include "types.hpp"
#include "bits.hpp"
#include "board.hpp"

namespace Chess {

  using namespace Board;
  
  namespace MoveGen {

    // Aggregated piece bitboards for one color.
    struct ColorPieceBbsT {
      // All pieces including promos.
      BitBoardT bbs[NPieceTypes];

      // Aggregated bitboards of diagonal/orthogonal sliders
      BitBoardT sliderBbs[NSliderDirections];
    };
    
    // Aggregated piece bitboards for both colors.
    struct PieceBbsT {
      ColorPieceBbsT colorPieceBbs[NColors];
    };

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

      // From square discovery masks for pawn pushes.
      BitBoardT pawnPushDiscoveryMasksBb;
      // From square discovery masks for pawn captures.
      BitBoardT pawnLeftDiscoveryMasksBb;
      BitBoardT pawnRightDiscoveryMasksBb;
      // Is EP move a discovery through the capture piece?
      bool isEpDiscovery;
      
      // TODO and king

      DiscoveredCheckMasksT():
	diagDiscoveryPiecesBb(BbNone), orthogDiscoveryPiecesBb(BbNone), pawnPushDiscoveryMasksBb(BbNone), pawnLeftDiscoveryMasksBb(BbNone), pawnRightDiscoveryMasksBb(BbNone), isEpDiscovery(false) {}
      
      DiscoveredCheckMasksT(const BitBoardT diagDiscoveryPiecesBb, const BitBoardT orthogDiscoveryPiecesBb, const BitBoardT pawnPushDiscoveryMasksBb, const BitBoardT pawnLeftDiscoveryMasksBb, const BitBoardT pawnRightDiscoveryMasksBb, const bool isEpDiscovery):
	diagDiscoveryPiecesBb(diagDiscoveryPiecesBb), orthogDiscoveryPiecesBb(orthogDiscoveryPiecesBb), pawnPushDiscoveryMasksBb(pawnPushDiscoveryMasksBb), pawnLeftDiscoveryMasksBb(pawnLeftDiscoveryMasksBb), pawnRightDiscoveryMasksBb(pawnRightDiscoveryMasksBb), isEpDiscovery(isEpDiscovery) {}
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
      PieceBbsT pieceBbs; // side-channel info
      
      CastlingRightsT canCastleFlags; // note not actually 'rights' per se but actually legal castling moves
      PawnPushesAndCapturesT pawnMoves;
      BitBoardT pieceMoves[NPieces];

      DirectCheckMasksT directChecks;
      DiscoveredCheckMasksT discoveredChecks;
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
#define BOOST_PP_LOCAL_MACRO(n)			\
	NoCastlingRights,
#define BOOST_PP_LOCAL_LIMITS (0, 63)
#include BOOST_PP_LOCAL_ITERATE()
      },
      // castlingRights == CanCastleQueenside
      {
#define BOOST_PP_LOCAL_MACRO(n)						\
	((n) & QueensideCastleSpaceBits) == 0x0 ? CanCastleQueenside : NoCastlingRights,
#define BOOST_PP_LOCAL_LIMITS (0, 63)
#include BOOST_PP_LOCAL_ITERATE()
      },
      // castlingRights == CanCastleKingside
      {
#define BOOST_PP_LOCAL_MACRO(n)						\
	((n) & KingsideCastleSpaceBits) == 0x0 ? CanCastleKingside : NoCastlingRights,
#define BOOST_PP_LOCAL_LIMITS (0, 63)
#include BOOST_PP_LOCAL_ITERATE()
      },
      // castlingRights == CanCastleQueenside | CanCastleKingside
      {
#define BOOST_PP_LOCAL_MACRO(n)						\
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
	0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
	0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
	0x0000000000000002ull, 0x0000000000000005ull, 0x000000000000000aull, 0x0000000000000014ull,
	0x0000000000000028ull, 0x0000000000000050ull, 0x00000000000000a0ull, 0x0000000000000040ull,
	0x0000000000000200ull, 0x0000000000000500ull, 0x0000000000000a00ull, 0x0000000000001400ull,
	0x0000000000002800ull, 0x0000000000005000ull, 0x000000000000a000ull, 0x0000000000004000ull,
	0x0000000000020000ull, 0x0000000000050000ull, 0x00000000000a0000ull, 0x0000000000140000ull,
	0x0000000000280000ull, 0x0000000000500000ull, 0x0000000000a00000ull, 0x0000000000400000ull,
	0x0000000002000000ull, 0x0000000005000000ull, 0x000000000a000000ull, 0x0000000014000000ull,
	0x0000000028000000ull, 0x0000000050000000ull, 0x00000000a0000000ull, 0x0000000040000000ull,
	0x0000000200000000ull, 0x0000000500000000ull, 0x0000000a00000000ull, 0x0000001400000000ull,
	0x0000002800000000ull, 0x0000005000000000ull, 0x000000a000000000ull, 0x0000004000000000ull,
	0x0000020000000000ull, 0x0000050000000000ull, 0x00000a0000000000ull, 0x0000140000000000ull,
	0x0000280000000000ull, 0x0000500000000000ull, 0x0000a00000000000ull, 0x0000400000000000ull,
	0x0002000000000000ull, 0x0005000000000000ull, 0x000a000000000000ull, 0x0014000000000000ull,
	0x0028000000000000ull, 0x0050000000000000ull, 0x00a0000000000000ull, 0x0040000000000000ull,
	BbNone, // InvalidSquare
      },
      // Black pawn attackers
      {
	0x0000000000000200ull, 0x0000000000000500ull, 0x0000000000000a00ull, 0x0000000000001400ull,
	0x0000000000002800ull, 0x0000000000005000ull, 0x000000000000a000ull, 0x0000000000004000ull,
	0x0000000000020000ull, 0x0000000000050000ull, 0x00000000000a0000ull, 0x0000000000140000ull,
	0x0000000000280000ull, 0x0000000000500000ull, 0x0000000000a00000ull, 0x0000000000400000ull,
	0x0000000002000000ull, 0x0000000005000000ull, 0x000000000a000000ull, 0x0000000014000000ull,
	0x0000000028000000ull, 0x0000000050000000ull, 0x00000000a0000000ull, 0x0000000040000000ull,
	0x0000000200000000ull, 0x0000000500000000ull, 0x0000000a00000000ull, 0x0000001400000000ull,
	0x0000002800000000ull, 0x0000005000000000ull, 0x000000a000000000ull, 0x0000004000000000ull,
	0x0000020000000000ull, 0x0000050000000000ull, 0x00000a0000000000ull, 0x0000140000000000ull,
	0x0000280000000000ull, 0x0000500000000000ull, 0x0000a00000000000ull, 0x0000400000000000ull,
	0x0002000000000000ull, 0x0005000000000000ull, 0x000a000000000000ull, 0x0014000000000000ull,
	0x0028000000000000ull, 0x0050000000000000ull, 0x00a0000000000000ull, 0x0040000000000000ull,
	0x0200000000000000ull, 0x0500000000000000ull, 0x0a00000000000000ull, 0x1400000000000000ull,
	0x2800000000000000ull, 0x5000000000000000ull, 0xa000000000000000ull, 0x4000000000000000ull,
	0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
	0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
	BbNone, // InvalidSquare
      },
    };

    const BitBoardT KnightAttacks[64+1] = { 
      0x0000000000020400ull, 0x0000000000050800ull, 0x00000000000a1100ull, 0x0000000000142200ull,
      0x0000000000284400ull, 0x0000000000508800ull, 0x0000000000a01000ull, 0x0000000000402000ull,
      0x0000000002040004ull, 0x0000000005080008ull, 0x000000000a110011ull, 0x0000000014220022ull,
      0x0000000028440044ull, 0x0000000050880088ull, 0x00000000a0100010ull, 0x0000000040200020ull,
      0x0000000204000402ull, 0x0000000508000805ull, 0x0000000a1100110aull, 0x0000001422002214ull,
      0x0000002844004428ull, 0x0000005088008850ull, 0x000000a0100010a0ull, 0x0000004020002040ull,
      0x0000020400040200ull, 0x0000050800080500ull, 0x00000a1100110a00ull, 0x0000142200221400ull,
      0x0000284400442800ull, 0x0000508800885000ull, 0x0000a0100010a000ull, 0x0000402000204000ull,
      0x0002040004020000ull, 0x0005080008050000ull, 0x000a1100110a0000ull, 0x0014220022140000ull,
      0x0028440044280000ull, 0x0050880088500000ull, 0x00a0100010a00000ull, 0x0040200020400000ull,
      0x0204000402000000ull, 0x0508000805000000ull, 0x0a1100110a000000ull, 0x1422002214000000ull,
      0x2844004428000000ull, 0x5088008850000000ull, 0xa0100010a0000000ull, 0x4020002040000000ull,
      0x0400040200000000ull, 0x0800080500000000ull, 0x1100110a00000000ull, 0x2200221400000000ull,
      0x4400442800000000ull, 0x8800885000000000ull, 0x100010a000000000ull, 0x2000204000000000ull,
      0x0004020000000000ull, 0x0008050000000000ull, 0x00110a0000000000ull, 0x0022140000000000ull,
      0x0044280000000000ull, 0x0088500000000000ull, 0x0010a00000000000ull, 0x0020400000000000ull,
      BbNone, // InvalidSquare
    };

    const BitBoardT KingAttacks[64+1] = {
      0x0000000000000302ull, 0x0000000000000705ull, 0x0000000000000e0aull, 0x0000000000001c14ull,
      0x0000000000003828ull, 0x0000000000007050ull, 0x000000000000e0a0ull, 0x000000000000c040ull,
      0x0000000000030203ull, 0x0000000000070507ull, 0x00000000000e0a0eull, 0x00000000001c141cull,
      0x0000000000382838ull, 0x0000000000705070ull, 0x0000000000e0a0e0ull, 0x0000000000c040c0ull,
      0x0000000003020300ull, 0x0000000007050700ull, 0x000000000e0a0e00ull, 0x000000001c141c00ull,
      0x0000000038283800ull, 0x0000000070507000ull, 0x00000000e0a0e000ull, 0x00000000c040c000ull,
      0x0000000302030000ull, 0x0000000705070000ull, 0x0000000e0a0e0000ull, 0x0000001c141c0000ull,
      0x0000003828380000ull, 0x0000007050700000ull, 0x000000e0a0e00000ull, 0x000000c040c00000ull,
      0x0000030203000000ull, 0x0000070507000000ull, 0x00000e0a0e000000ull, 0x00001c141c000000ull,
      0x0000382838000000ull, 0x0000705070000000ull, 0x0000e0a0e0000000ull, 0x0000c040c0000000ull,
      0x0003020300000000ull, 0x0007050700000000ull, 0x000e0a0e00000000ull, 0x001c141c00000000ull,
      0x0038283800000000ull, 0x0070507000000000ull, 0x00e0a0e000000000ull, 0x00c040c000000000ull,
      0x0302030000000000ull, 0x0705070000000000ull, 0x0e0a0e0000000000ull, 0x1c141c0000000000ull,
      0x3828380000000000ull, 0x7050700000000000ull, 0xe0a0e00000000000ull, 0xc040c00000000000ull,
      0x0203000000000000ull, 0x0507000000000000ull, 0x0a0e000000000000ull, 0x141c000000000000ull,
      0x2838000000000000ull, 0x5070000000000000ull, 0xa0e0000000000000ull, 0x40c0000000000000ull,
      BbNone, // InvalidSquare - there should always be a king, but...
    };

    // Bishop rays
    const BitBoardT BishopRays[64+1] = {
      0x8040201008040200ull, 0x0080402010080500ull, 0x0000804020110a00ull, 0x0000008041221400ull,
      0x0000000182442800ull, 0x0000010204885000ull, 0x000102040810a000ull, 0x0102040810204000ull,
      0x4020100804020002ull, 0x8040201008050005ull, 0x00804020110a000aull, 0x0000804122140014ull,
      0x0000018244280028ull, 0x0001020488500050ull, 0x0102040810a000a0ull, 0x0204081020400040ull,
      0x2010080402000204ull, 0x4020100805000508ull, 0x804020110a000a11ull, 0x0080412214001422ull,
      0x0001824428002844ull, 0x0102048850005088ull, 0x02040810a000a010ull, 0x0408102040004020ull,
      0x1008040200020408ull, 0x2010080500050810ull, 0x4020110a000a1120ull, 0x8041221400142241ull,
      0x0182442800284482ull, 0x0204885000508804ull, 0x040810a000a01008ull, 0x0810204000402010ull,
      0x0804020002040810ull, 0x1008050005081020ull, 0x20110a000a112040ull, 0x4122140014224180ull,
      0x8244280028448201ull, 0x0488500050880402ull, 0x0810a000a0100804ull, 0x1020400040201008ull,
      0x0402000204081020ull, 0x0805000508102040ull, 0x110a000a11204080ull, 0x2214001422418000ull,
      0x4428002844820100ull, 0x8850005088040201ull, 0x10a000a010080402ull, 0x2040004020100804ull,
      0x0200020408102040ull, 0x0500050810204080ull, 0x0a000a1120408000ull, 0x1400142241800000ull,
      0x2800284482010000ull, 0x5000508804020100ull, 0xa000a01008040201ull, 0x4000402010080402ull,
      0x0002040810204080ull, 0x0005081020408000ull, 0x000a112040800000ull, 0x0014224180000000ull,
      0x0028448201000000ull, 0x0050880402010000ull, 0x00a0100804020100ull, 0x0040201008040201ull,
      BbNone, // InvalidSquare - there should always be a king, but...
    };

    // Rook rays
    const BitBoardT RookRays[64+1] = {
      0x01010101010101feull, 0x02020202020202fdull, 0x04040404040404fbull, 0x08080808080808f7ull,
      0x10101010101010efull, 0x20202020202020dfull, 0x40404040404040bfull, 0x808080808080807full,
      0x010101010101fe01ull, 0x020202020202fd02ull, 0x040404040404fb04ull, 0x080808080808f708ull,
      0x101010101010ef10ull, 0x202020202020df20ull, 0x404040404040bf40ull, 0x8080808080807f80ull,
      0x0101010101fe0101ull, 0x0202020202fd0202ull, 0x0404040404fb0404ull, 0x0808080808f70808ull,
      0x1010101010ef1010ull, 0x2020202020df2020ull, 0x4040404040bf4040ull, 0x80808080807f8080ull,
      0x01010101fe010101ull, 0x02020202fd020202ull, 0x04040404fb040404ull, 0x08080808f7080808ull,
      0x10101010ef101010ull, 0x20202020df202020ull, 0x40404040bf404040ull, 0x808080807f808080ull,
      0x010101fe01010101ull, 0x020202fd02020202ull, 0x040404fb04040404ull, 0x080808f708080808ull,
      0x101010ef10101010ull, 0x202020df20202020ull, 0x404040bf40404040ull, 0x8080807f80808080ull,
      0x0101fe0101010101ull, 0x0202fd0202020202ull, 0x0404fb0404040404ull, 0x0808f70808080808ull,
      0x1010ef1010101010ull, 0x2020df2020202020ull, 0x4040bf4040404040ull, 0x80807f8080808080ull,
      0x01fe010101010101ull, 0x02fd020202020202ull, 0x04fb040404040404ull, 0x08f7080808080808ull,
      0x10ef101010101010ull, 0x20df202020202020ull, 0x40bf404040404040ull, 0x807f808080808080ull,
      0xfe01010101010101ull, 0xfd02020202020202ull, 0xfb04040404040404ull, 0xf708080808080808ull,
      0xef10101010101010ull, 0xdf20202020202020ull, 0xbf40404040404040ull, 0x7f80808080808080ull,
      BbNone, // InvalidSquare
    };

    enum BishopUniRayDirT {
      Right,
      Left
    };
    
    const BitBoardT BishopUniRays[2][64+1] = {
      {
	// Bishop uni-rays right
	0x8040201008040201ull, 0x0080402010080402ull, 0x0000804020100804ull, 0x0000008040201008ull,
	0x0000000080402010ull, 0x0000000000804020ull, 0x0000000000008040ull, 0x0000000000000080ull,
	0x4020100804020100ull, 0x8040201008040201ull, 0x0080402010080402ull, 0x0000804020100804ull,
	0x0000008040201008ull, 0x0000000080402010ull, 0x0000000000804020ull, 0x0000000000008040ull,
	0x2010080402010000ull, 0x4020100804020100ull, 0x8040201008040201ull, 0x0080402010080402ull,
	0x0000804020100804ull, 0x0000008040201008ull, 0x0000000080402010ull, 0x0000000000804020ull,
	0x1008040201000000ull, 0x2010080402010000ull, 0x4020100804020100ull, 0x8040201008040201ull,
	0x0080402010080402ull, 0x0000804020100804ull, 0x0000008040201008ull, 0x0000000080402010ull,
	0x0804020100000000ull, 0x1008040201000000ull, 0x2010080402010000ull, 0x4020100804020100ull,
	0x8040201008040201ull, 0x0080402010080402ull, 0x0000804020100804ull, 0x0000008040201008ull,
	0x0402010000000000ull, 0x0804020100000000ull, 0x1008040201000000ull, 0x2010080402010000ull,
	0x4020100804020100ull, 0x8040201008040201ull, 0x0080402010080402ull, 0x0000804020100804ull,
	0x0201000000000000ull, 0x0402010000000000ull, 0x0804020100000000ull, 0x1008040201000000ull,
	0x2010080402010000ull, 0x4020100804020100ull, 0x8040201008040201ull, 0x0080402010080402ull,
	0x0100000000000000ull, 0x0201000000000000ull, 0x0402010000000000ull, 0x0804020100000000ull,
	0x1008040201000000ull, 0x2010080402010000ull, 0x4020100804020100ull, 0x8040201008040201ull,
	BbNone, // InvalidSquare
      },
      {
	// Bishop uni-rays left
	0x0000000000000001ull, 0x0000000000000102ull, 0x0000000000010204ull, 0x0000000001020408ull,
	0x0000000102040810ull, 0x0000010204081020ull, 0x0001020408102040ull, 0x0102040810204080ull,
	0x0000000000000102ull, 0x0000000000010204ull, 0x0000000001020408ull, 0x0000000102040810ull,
	0x0000010204081020ull, 0x0001020408102040ull, 0x0102040810204080ull, 0x0204081020408000ull,
	0x0000000000010204ull, 0x0000000001020408ull, 0x0000000102040810ull, 0x0000010204081020ull,
	0x0001020408102040ull, 0x0102040810204080ull, 0x0204081020408000ull, 0x0408102040800000ull,
	0x0000000001020408ull, 0x0000000102040810ull, 0x0000010204081020ull, 0x0001020408102040ull,
	0x0102040810204080ull, 0x0204081020408000ull, 0x0408102040800000ull, 0x0810204080000000ull,
	0x0000000102040810ull, 0x0000010204081020ull, 0x0001020408102040ull, 0x0102040810204080ull,
	0x0204081020408000ull, 0x0408102040800000ull, 0x0810204080000000ull, 0x1020408000000000ull,
	0x0000010204081020ull, 0x0001020408102040ull, 0x0102040810204080ull, 0x0204081020408000ull,
	0x0408102040800000ull, 0x0810204080000000ull, 0x1020408000000000ull, 0x2040800000000000ull,
	0x0001020408102040ull, 0x0102040810204080ull, 0x0204081020408000ull, 0x0408102040800000ull,
	0x0810204080000000ull, 0x1020408000000000ull, 0x2040800000000000ull, 0x4080000000000000ull,
	0x0102040810204080ull, 0x0204081020408000ull, 0x0408102040800000ull, 0x0810204080000000ull,
	0x1020408000000000ull, 0x2040800000000000ull, 0x4080000000000000ull, 0x8000000000000000ull,
	BbNone, // InvalidSquare
      }
    };

    template <ColorT Color, BishopUniRayDirT Dir>
    inline BitBoardT bishopUniRay(SquareT square);
    
    template <> inline BitBoardT bishopUniRay<White, Right>(SquareT square) { return BishopUniRays[Right][square]; }
    template <> inline BitBoardT bishopUniRay<White, Left>(SquareT square) { return BishopUniRays[Left][square]; }
    template <> inline BitBoardT bishopUniRay<Black, Right>(SquareT square) { return BishopUniRays[Left][square]; }
    template <> inline BitBoardT bishopUniRay<Black, Left>(SquareT square) { return BishopUniRays[Right][square]; }
    
    //
    // Rays - used to generate Magic Bitboard tables
    //
    extern BitBoardT Rays[8][64];

    //
    // Magic Bitboards for Rook and Bishop attacks
    //

    const BitBoardT RookBlockers[64+1] = {
      0x000101010101017Eull, 0x000202020202027Cull, 0x000404040404047Aull, 0x0008080808080876ull,
      0x001010101010106Eull, 0x002020202020205Eull, 0x004040404040403Eull, 0x008080808080807Eull,
      0x0001010101017E00ull, 0x0002020202027C00ull, 0x0004040404047A00ull, 0x0008080808087600ull,
      0x0010101010106E00ull, 0x0020202020205E00ull, 0x0040404040403E00ull, 0x0080808080807E00ull,
      0x00010101017E0100ull, 0x00020202027C0200ull, 0x00040404047A0400ull, 0x0008080808760800ull,
      0x00101010106E1000ull, 0x00202020205E2000ull, 0x00404040403E4000ull, 0x00808080807E8000ull,
      0x000101017E010100ull, 0x000202027C020200ull, 0x000404047A040400ull, 0x0008080876080800ull,
      0x001010106E101000ull, 0x002020205E202000ull, 0x004040403E404000ull, 0x008080807E808000ull,
      0x0001017E01010100ull, 0x0002027C02020200ull, 0x0004047A04040400ull, 0x0008087608080800ull,
      0x0010106E10101000ull, 0x0020205E20202000ull, 0x0040403E40404000ull, 0x0080807E80808000ull,
      0x00017E0101010100ull, 0x00027C0202020200ull, 0x00047A0404040400ull, 0x0008760808080800ull,
      0x00106E1010101000ull, 0x00205E2020202000ull, 0x00403E4040404000ull, 0x00807E8080808000ull,
      0x007E010101010100ull, 0x007C020202020200ull, 0x007A040404040400ull, 0x0076080808080800ull,
      0x006E101010101000ull, 0x005E202020202000ull, 0x003E404040404000ull, 0x007E808080808000ull,
      0x7E01010101010100ull, 0x7C02020202020200ull, 0x7A04040404040400ull, 0x7608080808080800ull,
      0x6E10101010101000ull, 0x5E20202020202000ull, 0x3E40404040404000ull, 0x7E80808080808000ull,
      0x0, // InvalidSquare
    };
    
    const BitBoardT BishopBlockers[64+1] = {
      0x0040201008040200ull, 0x0000402010080400ull, 0x0000004020100A00ull, 0x0000000040221400ull,
      0x0000000002442800ull, 0x0000000204085000ull, 0x0000020408102000ull, 0x0002040810204000ull,
      0x0020100804020000ull, 0x0040201008040000ull, 0x00004020100A0000ull, 0x0000004022140000ull,
      0x0000000244280000ull, 0x0000020408500000ull, 0x0002040810200000ull, 0x0004081020400000ull,
      0x0010080402000200ull, 0x0020100804000400ull, 0x004020100A000A00ull, 0x0000402214001400ull,
      0x0000024428002800ull, 0x0002040850005000ull, 0x0004081020002000ull, 0x0008102040004000ull,
      0x0008040200020400ull, 0x0010080400040800ull, 0x0020100A000A1000ull, 0x0040221400142200ull,
      0x0002442800284400ull, 0x0004085000500800ull, 0x0008102000201000ull, 0x0010204000402000ull,
      0x0004020002040800ull, 0x0008040004081000ull, 0x00100A000A102000ull, 0x0022140014224000ull,
      0x0044280028440200ull, 0x0008500050080400ull, 0x0010200020100800ull, 0x0020400040201000ull,
      0x0002000204081000ull, 0x0004000408102000ull, 0x000A000A10204000ull, 0x0014001422400000ull,
      0x0028002844020000ull, 0x0050005008040200ull, 0x0020002010080400ull, 0x0040004020100800ull,
      0x0000020408102000ull, 0x0000040810204000ull, 0x00000A1020400000ull, 0x0000142240000000ull,
      0x0000284402000000ull, 0x0000500804020000ull, 0x0000201008040200ull, 0x0000402010080400ull,
      0x0002040810204000ull, 0x0004081020400000ull, 0x000A102040000000ull, 0x0014224000000000ull,
      0x0028440200000000ull, 0x0050080402000000ull, 0x0020100804020000ull, 0x0040201008040200ull,
      0x0, // InvalidSquare
    };

    const BitBoardT RookMagicBbMultipliers[64+1] = {
      0xa8002c000108020ull, 0x6c00049b0002001ull, 0x100200010090040ull, 0x2480041000800801ull, 0x280028004000800ull,
      0x900410008040022ull, 0x280020001001080ull, 0x2880002041000080ull, 0xa000800080400034ull, 0x4808020004000ull,
      0x2290802004801000ull, 0x411000d00100020ull, 0x402800800040080ull, 0xb000401004208ull, 0x2409000100040200ull,
      0x1002100004082ull, 0x22878001e24000ull, 0x1090810021004010ull, 0x801030040200012ull, 0x500808008001000ull,
      0xa08018014000880ull, 0x8000808004000200ull, 0x201008080010200ull, 0x801020000441091ull, 0x800080204005ull,
      0x1040200040100048ull, 0x120200402082ull, 0xd14880480100080ull, 0x12040280080080ull, 0x100040080020080ull,
      0x9020010080800200ull, 0x813241200148449ull, 0x491604001800080ull, 0x100401000402001ull, 0x4820010021001040ull,
      0x400402202000812ull, 0x209009005000802ull, 0x810800601800400ull, 0x4301083214000150ull, 0x204026458e001401ull,
      0x40204000808000ull, 0x8001008040010020ull, 0x8410820820420010ull, 0x1003001000090020ull, 0x804040008008080ull,
      0x12000810020004ull, 0x1000100200040208ull, 0x430000a044020001ull, 0x280009023410300ull, 0xe0100040002240ull,
      0x200100401700ull, 0x2244100408008080ull, 0x8000400801980ull, 0x2000810040200ull, 0x8010100228810400ull,
      0x2000009044210200ull, 0x4080008040102101ull, 0x40002080411d01ull, 0x2005524060000901ull, 0x502001008400422ull,
      0x489a000810200402ull, 0x1004400080a13ull, 0x4000011008020084ull, 0x26002114058042ull,
      0x0, // InvalidSquare
    };

    const BitBoardT BishopMagicBbMultipliers[64+1] = {
      0x89a1121896040240ull, 0x2004844802002010ull, 0x2068080051921000ull, 0x62880a0220200808ull, 0x4042004000000ull,
      0x100822020200011ull, 0xc00444222012000aull, 0x28808801216001ull, 0x400492088408100ull, 0x201c401040c0084ull,
      0x840800910a0010ull, 0x82080240060ull, 0x2000840504006000ull, 0x30010c4108405004ull, 0x1008005410080802ull,
      0x8144042209100900ull, 0x208081020014400ull, 0x4800201208ca00ull, 0xf18140408012008ull, 0x1004002802102001ull,
      0x841000820080811ull, 0x40200200a42008ull, 0x800054042000ull, 0x88010400410c9000ull, 0x520040470104290ull,
      0x1004040051500081ull, 0x2002081833080021ull, 0x400c00c010142ull, 0x941408200c002000ull, 0x658810000806011ull,
      0x188071040440a00ull, 0x4800404002011c00ull, 0x104442040404200ull, 0x511080202091021ull, 0x4022401120400ull,
      0x80c0040400080120ull, 0x8040010040820802ull, 0x480810700020090ull, 0x102008e00040242ull, 0x809005202050100ull,
      0x8002024220104080ull, 0x431008804142000ull, 0x19001802081400ull, 0x200014208040080ull, 0x3308082008200100ull,
      0x41010500040c020ull, 0x4012020c04210308ull, 0x208220a202004080ull, 0x111040120082000ull, 0x6803040141280a00ull,
      0x2101004202410000ull, 0x8200000041108022ull, 0x21082088000ull, 0x2410204010040ull, 0x40100400809000ull,
      0x822088220820214ull, 0x40808090012004ull, 0x910224040218c9ull, 0x402814422015008ull, 0x90014004842410ull,
      0x1000042304105ull, 0x10008830412a00ull, 0x2520081090008908ull, 0x40102000a0a60140ull,
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
    template <typename BoardTraitsT>
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

	  typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
	  const ColorT OtherColor = BoardTraitsT::OtherColor;

	  const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
	  const ColorPieceMapT& yourPieceMap = genColorPieceMap<YourColorTraitsT>(yourState);
	  
	  const SquareT checkingPieceSq = Bits::lsb(allMyKingAttackersBb);
	  
	  // const PieceT checkingPiece1 = squarePiecePiece(board.board[checkingPieceSq]);

	  const BitBoardT pawnAttackerBb = allMyKingAttackersBb & yourState.pawnsBb;
	  const PieceT checkingPiece = pawnAttackerBb != BbNone ? SomePawns : yourPieceMap.board[checkingPieceSq];

	  // if(checkingPiece != checkingPiece1) {
	  //   printf("Booooo - bad checking piece - old %d vs new %d\n", checkingPiece1, checkingPiece);
	  //   //exit(1);
	  // }
	  
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

    template <SliderDirectionT SliderDirection> inline BitBoardT genPinnedPiecesBb(const SquareT myKingSq, const BitBoardT allPiecesBb, const BitBoardT allMyPiecesBb, const ColorPieceBbsT yourPieceBbs) {
      const BitBoardT myKingSliderAttackersBb = genSliderAttacksBb<SliderDirection>(myKingSq, allPiecesBb);
      // Potentially pinned pieces are my pieces that are on an open ray from my king
      const BitBoardT myCandidateSliderPinnedPiecesBb = myKingSliderAttackersBb & allMyPiecesBb;
      // Your pinning pieces are those that attack my king once my candidate pinned pieces are removed from the board
      const BitBoardT myKingSliderXrayAttackersBb = genSliderAttacksBb<SliderDirection>(myKingSq, (allPiecesBb & ~myCandidateSliderPinnedPiecesBb));
      // Your sliders of the required slider direction
      const BitBoardT yourSlidersBb = yourPieceBbs.sliderBbs[SliderDirection];
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
      
      PieceAttacksT attacks = {};

      // Pawns
      BitBoardT pawns = colorState.pawnsBb; //bbsOld[Pawn];
      attacks.pawnsLeftAttacks = pawnsLeftAttacks<Color>(pawns);
      attacks.pawnsRightAttacks = pawnsRightAttacks<Color>(pawns);
      
      attacks.allAttacks |= (attacks.pawnsLeftAttacks | attacks.pawnsRightAttacks);
      
      attacks.pawnsPushOne = pawnsPushOne<Color>(pawns, allPiecesBb);
      attacks.pawnsPushTwo = pawnsPushTwo<Color>(attacks.pawnsPushOne, allPiecesBb);

      // Knights
      
      SquareT knight1Square = colorState.pieceSquares[Knight1];
      
      attacks.pieceAttacks[Knight1] = KnightAttacks[knight1Square];
      
      attacks.allAttacks |= attacks.pieceAttacks[Knight1];
      
      SquareT knight2Square = colorState.pieceSquares[Knight2];
      
      attacks.pieceAttacks[Knight2] = KnightAttacks[knight2Square];
      
      attacks.allAttacks |= attacks.pieceAttacks[Knight2];

      // Bishops

      SquareT bishop1Square = colorState.pieceSquares[Bishop1];
      
      attacks.pieceAttacks[Bishop1] = bishopAttacks(bishop1Square, allPiecesBb);
      
      attacks.allAttacks |= attacks.pieceAttacks[Bishop1];
      
      SquareT bishop2Square = colorState.pieceSquares[Bishop2];
      
      attacks.pieceAttacks[Bishop2] = bishopAttacks(bishop2Square, allPiecesBb);
      
      attacks.allAttacks |= attacks.pieceAttacks[Bishop2];
      
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
      // if(ColorTraitsT::HasPromos) {
      // 	if(true/*piecesPresent & PromoQueenPresentFlag*/) {
      // 	  SquareT promoQueenSquare = colorState.pieceSquares[PromoQueen];
	  
      // 	  attacks.pieceAttacks[PromoQueen] = rookAttacks(promoQueenSquare, allPiecesBb) | bishopAttacks(promoQueenSquare, allPiecesBb);
	  
      // 	  attacks.allAttacks |= attacks.pieceAttacks[PromoQueen];
      // 	}
      // }
      
      return attacks;
    }

    // Generate attackers/defenders of a particular square.
    // Useful for check detection.
    template <typename ColorTraitsT>
    inline SquareAttackersT genSquareAttackers(const SquareT square, const ColorPieceBbsT& colorPieceBbs, const BitBoardT allPiecesBb) {
      const ColorT Color = ColorTraitsT::Color;
      
      SquareAttackersT attackers = {};

      // Pawns
      
      BitBoardT pawnAttackersBb = PawnAttackers[(size_t)Color][square] & colorPieceBbs.bbs[Pawn];
      attackers.pieceAttackers[Pawn] = pawnAttackersBb;
      attackers.pieceAttackers[AllPieceTypes] |= pawnAttackersBb;

      // Knights
      
      BitBoardT knightAttackersBb = KnightAttacks[square] & colorPieceBbs.bbs[Knight];
      attackers.pieceAttackers[Knight] = knightAttackersBb;
      attackers.pieceAttackers[AllPieceTypes] |= knightAttackersBb;

      // Diagonal sliders
      
      BitBoardT diagonalAttackerSquaresBb = bishopAttacks(square, allPiecesBb);

      BitBoardT bishopAttackers = diagonalAttackerSquaresBb & colorPieceBbs.bbs[Bishop];
      attackers.pieceAttackers[Bishop] = bishopAttackers;

      BitBoardT diagonalQueenAttackers = diagonalAttackerSquaresBb & colorPieceBbs.bbs[Queen];
      attackers.pieceAttackers[Queen] |= diagonalQueenAttackers;

      attackers.pieceAttackers[AllPieceTypes] |= bishopAttackers | diagonalQueenAttackers;

      // Orthogonal sliders

      BitBoardT orthogonalAttackerSquaresBb = rookAttacks(square, allPiecesBb);

      BitBoardT rookAttackers = orthogonalAttackerSquaresBb & colorPieceBbs.bbs[Rook];
      attackers.pieceAttackers[Rook] = rookAttackers;

      BitBoardT orthogonalQueenAttackers = orthogonalAttackerSquaresBb & colorPieceBbs.bbs[Queen];
      attackers.pieceAttackers[Queen] |= orthogonalQueenAttackers;

      attackers.pieceAttackers[AllPieceTypes] |= rookAttackers | orthogonalQueenAttackers;
	
      // King attacker

      BitBoardT kingAttackers = KingAttacks[square] & colorPieceBbs.bbs[King];
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
      pinMasks.piecePinMasks[Knight1] = BbAll;
      pinMasks.piecePinMasks[Knight2] = BbAll;
	
      // Bishops
      pinMasks.piecePinMasks[Bishop1] = BbAll;
      pinMasks.piecePinMasks[Bishop2] = BbAll;

      // Rooks
      pinMasks.piecePinMasks[QueenRook] = BbAll;
      pinMasks.piecePinMasks[KingRook] = BbAll;

      // Queens
      //   - diagonally pinned queens can only move along the king's bishop rays
      //   - orthogonally pinned queens can only move along the king's rook rays
      pinMasks.piecePinMasks[TheQueen] = BbAll;

      // TODO other promo pieces
      // if(MyColorTraitsT::HasPromos) {
      // 	if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
      // 	  pinMasks.piecePinMasks[PromoQueen] = BbAll;
      // 	}
      // }
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
      pinMasks.piecePinMasks[Knight1] = genPinnedMoveMask<Knight>(myState.pieceSquares[Knight1], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);
      pinMasks.piecePinMasks[Knight2] = genPinnedMoveMask<Knight>(myState.pieceSquares[Knight2], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);
	
      // Bishops
      pinMasks.piecePinMasks[Bishop1] = genPinnedMoveMask<Bishop>(myState.pieceSquares[Bishop1], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);
      pinMasks.piecePinMasks[Bishop2] = genPinnedMoveMask<Bishop>(myState.pieceSquares[Bishop2], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);

      // Rooks
      pinMasks.piecePinMasks[QueenRook] = genPinnedMoveMask<Rook>(myState.pieceSquares[QueenRook], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);
      pinMasks.piecePinMasks[KingRook] = genPinnedMoveMask<Rook>(myState.pieceSquares[KingRook], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);

      // Queens
      //   - diagonally pinned queens can only move along the king's bishop rays
      //   - orthogonally pinned queens can only move along the king's rook rays
      pinMasks.piecePinMasks[TheQueen] = genPinnedMoveMask<Queen>(myState.pieceSquares[TheQueen], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);

      // TODO other promo pieces
      // if(ColorTraitsT::HasPromos) {
      // 	if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
      // 	  pinMasks.piecePinMasks[PromoQueen] = genPinnedMoveMask<Queen>(myState.pieceSquares[PromoQueen], myKingSq, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);
      // 	}
      // }
    }
    
    template <typename BoardTraitsT>
    inline PiecePinMasksT genPinMasks(const BoardT& board, const PieceBbsT& pieceBbs) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& myState = board.pieces[(size_t)Color];
      
      const ColorPieceBbsT& myPieceBbs = pieceBbs.colorPieceBbs[(size_t)Color];
      const ColorPieceBbsT& yourPieceBbs = pieceBbs.colorPieceBbs[(size_t)OtherColor];
      
      const BitBoardT allMyPiecesBb = myPieceBbs.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];
      
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      const SquareT myKingSq = myState.pieceSquares[TheKing];

      // Find my pinned pieces - used to mask out invalid moves due to discovered check on my king
      const BitBoardT myDiagPinnedPiecesBb = genPinnedPiecesBb<Diagonal>(myKingSq, allPiecesBb, allMyPiecesBb, yourPieceBbs);
      const BitBoardT myOrthogPinnedPiecesBb = genPinnedPiecesBb<Orthogonal>(myKingSq, allPiecesBb, allMyPiecesBb, yourPieceBbs);
      
      // Generate pinned piece move masks for each piece
      PiecePinMasksT pinMasks = {};
      // Majority of positions have no pins
      if((myDiagPinnedPiecesBb | myOrthogPinnedPiecesBb) == BbNone) {
	genDefaultPiecePinMasks<MyColorTraitsT>(pinMasks);
      } else {
	genPiecePinMasks<MyColorTraitsT>(pinMasks, myState, myDiagPinnedPiecesBb, myOrthogPinnedPiecesBb);
      }
      
      return pinMasks;
    }

    template <typename BoardTraitsT>
    inline DiscoveredCheckMasksT genDiscoveryMasks(const BoardT& board, const PieceBbsT& pieceBbs, const BitBoardT legalEpCaptureLeftBb, const BitBoardT legalEpCaptureRightBb) {
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
      
      const BitBoardT allMyPiecesBb = pieceBbs.colorPieceBbs[(size_t)Color].bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = pieceBbs.colorPieceBbs[(size_t)OtherColor].bbs[AllPieceTypes];
      
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      const ColorPieceBbsT& myPieceBbs = pieceBbs.colorPieceBbs[(size_t)Color];

      const SquareT yourKingSq = yourState.pieceSquares[TheKing];

      // Find my pieces that are blocking check - used for discovered check detection.
      const BitBoardT myDiagDiscoveryPiecesBb = genPinnedPiecesBb<Diagonal>(yourKingSq, allPiecesBb, allMyPiecesBb, myPieceBbs);
      const BitBoardT myOrthogDiscoveryPiecesBb = genPinnedPiecesBb<Orthogonal>(yourKingSq, allPiecesBb, allMyPiecesBb, myPieceBbs);

      // Pawn push discovery pieces are all (pawn) diag discovery pieces AND all (pawn) orthog discovery pieces on the rank of the king.
      const BitBoardT pawnPushDiscoveryMasksBb = myDiagDiscoveryPiecesBb | (myOrthogDiscoveryPiecesBb & RankBbs[rankOf(yourKingSq)]);

      // Pawn capture discovery pieces are all (pawn) orthog discovery pieces AND all (pawn) diag discovery pieces not on the same bishop ray from the king as the capture direction
      const BitBoardT pawnLeftDiscoveryMasksBb = (myDiagDiscoveryPiecesBb & ~bishopUniRay<Color, Left>(yourKingSq)) | myOrthogDiscoveryPiecesBb;
      const BitBoardT pawnRightDiscoveryMasksBb = (myDiagDiscoveryPiecesBb & ~bishopUniRay<Color, Right>(yourKingSq)) | myOrthogDiscoveryPiecesBb;

      bool isEpDiscovery = false;
      // Only do the heavy lifting of detecting discovered check through the en-passant captured pawn if there really is an en-passant opportunity
      // En-passant is tricky because the captured pawn is not on the same square as the capturing piece, and might expose a discovered check itself.
      if((legalEpCaptureLeftBb | legalEpCaptureRightBb) != BbNone) {
	  
	const SquareT to = Bits::lsb(legalEpCaptureLeftBb | legalEpCaptureRightBb);
	const SquareT captureSq = pawnPushOneTo2From<BoardTraitsT::Color>(to);
	const BitBoardT captureSquareBb = bbForSquare(captureSq);

	// If your king is exposed to a diagonal slider when we remove the captured pawn, then this is a discovery through the ep-captured pawn
	if((bishopAttacks(yourKingSq, allPiecesBb & ~captureSquareBb) & myPieceBbs.sliderBbs[Diagonal]) != BbNone) {
	  isEpDiscovery = true;
	}
	//isEpDiscovery = (myDiagDiscoveryPiecesBb & captureSquareBb) != BbNone; // Gack - it's your damn piece, not mine :P
	// TODO horizontal discovery
      }
      
      return DiscoveredCheckMasksT(myDiagDiscoveryPiecesBb, myOrthogDiscoveryPiecesBb, pawnPushDiscoveryMasksBb, pawnLeftDiscoveryMasksBb, pawnRightDiscoveryMasksBb, isEpDiscovery);
    }

    template <PieceT Piece>
    inline BitBoardT genLegalPieceMoves(const PieceAttacksT myAttacks, const BitBoardT legalMoveMaskBb, const PiecePinMasksT& pinMasks, const BitBoardT allMyPiecesBb) {
      return myAttacks.pieceAttacks[Piece] & legalMoveMaskBb & pinMasks.piecePinMasks[Piece] & ~allMyPiecesBb;
    }

    template <typename BoardTraitsT>
    inline EpPawnCapturesT genLegalPawnEpCaptures(const BoardT& board, const ColorPieceBbsT& yourPieceBbs, const PieceAttacksT myAttacks, const SquareT epSquare, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT legalPawnsLeftBb, const BitBoardT legalPawnsRightBb) {
      const BitBoardT epSquareBb = bbForSquare(epSquare);

      const BitBoardT semiLegalEpCaptureLeftBb = legalPawnsLeftBb & epSquareBb;
      const BitBoardT semiLegalEpCaptureRightBb = legalPawnsRightBb & epSquareBb;

      BitBoardT legalEpCaptureLeftBb = BbNone;
      BitBoardT legalEpCaptureRightBb = BbNone;

      // Only do the heavy lifting of detecting discovered check through the captured pawn if there really is an en-passant opportunity
      // En-passant is tricky because the captured pawn is not on the same square as the capturing piece, and might expose a discovered check itself.
      if((semiLegalEpCaptureLeftBb | semiLegalEpCaptureRightBb) != BbNone) {
	const ColorStateT& myState = board.pieces[(size_t)BoardTraitsT::Color];
	const SquareT myKingSq = myState.pieceSquares[TheKing];
	  
	const SquareT to = Bits::lsb(semiLegalEpCaptureLeftBb | semiLegalEpCaptureRightBb);
	const SquareT captureSq = pawnPushOneTo2From<BoardTraitsT::Color>(to);
	const BitBoardT captureSquareBb = bbForSquare(captureSq);

	// Note that a discovered check can only be diagonal or horizontal, because the capturing pawn ends up on the same file as the captured pawn.
	const BitBoardT diagPinnedEpPawnBb = genPinnedPiecesBb<Diagonal>(myKingSq, allPiecesBb, captureSquareBb, yourPieceBbs);
	// Horizontal is really tricky because it involves both capturing and captured pawn.
	// We detect it by removing them both and looking for a king attack - could optimise this... TODO anyhow
	const BitBoardT orthogPinnedEpPawnBb = BbNone; //genPinnedPiecesBb<Orthogonal>(myKingSq, allPiecesBb, captureSquareBb, yourPieceBbs);

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
    inline PawnPushesAndCapturesT genLegalPawnMoves(const BoardT& board, const ColorPieceBbsT& yourPieceBbs, const PieceAttacksT myAttacks, const SquareT epSquare, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT legalMoveMaskBb, const PiecePinMasksT& pinMasks) {
      const BitBoardT legalPawnsPushOneBb = myAttacks.pawnsPushOne & legalMoveMaskBb & pinMasks.pawnsPushOnePinMask;
      const BitBoardT legalPawnsPushTwoBb = myAttacks.pawnsPushTwo & legalMoveMaskBb & pinMasks.pawnsPushTwoPinMask;
	
      // Pawn captures

      const BitBoardT legalPawnsLeftBb = myAttacks.pawnsLeftAttacks & legalMoveMaskBb & pinMasks.pawnsLeftPinMask;
      const BitBoardT legalPawnsRightBb = myAttacks.pawnsRightAttacks & legalMoveMaskBb & pinMasks.pawnsRightPinMask;

      // Pawn en-passant captures
      const EpPawnCapturesT legalEpPawnCaptures = (epSquare == InvalidSquare) ? EpPawnCapturesT() : genLegalPawnEpCaptures<BoardTraitsT>(board, yourPieceBbs, myAttacks, epSquare, allYourPiecesBb, allPiecesBb, legalPawnsLeftBb, legalPawnsRightBb);
      
      return PawnPushesAndCapturesT(legalPawnsPushOneBb, legalPawnsPushTwoBb, legalPawnsLeftBb & allYourPiecesBb, legalPawnsRightBb & allYourPiecesBb, legalEpPawnCaptures);
    }
    
    template <typename BoardTraitsT> 
    inline void genLegalNonKingMoves(LegalMovesT& legalMoves, const BoardT& board, const PieceBbsT& pieceBbs, const PieceAttacksT& myAttacks, const BitBoardT legalMoveMaskBb, const PiecePinMasksT& pinMasks) {
      // typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
      
      const ColorPieceBbsT& myPieceBbs = pieceBbs.colorPieceBbs[(size_t)Color];
      const ColorPieceBbsT& yourPieceBbs = pieceBbs.colorPieceBbs[(size_t)OtherColor];
      
      const BitBoardT allMyPiecesBb = myPieceBbs.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];
      
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      legalMoves.pawnMoves = genLegalPawnMoves<BoardTraitsT>(board, yourPieceBbs, myAttacks, yourState.epSquare, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinMasks);
      
      legalMoves.pieceMoves[Knight1] = genLegalPieceMoves<Knight1>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);
      legalMoves.pieceMoves[Knight2] = genLegalPieceMoves<Knight2>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);

      legalMoves.pieceMoves[Bishop1] = genLegalPieceMoves<Bishop1>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);
      legalMoves.pieceMoves[Bishop2] = genLegalPieceMoves<Bishop2>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);
      
      legalMoves.pieceMoves[QueenRook] = genLegalPieceMoves<QueenRook>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);
      legalMoves.pieceMoves[KingRook] = genLegalPieceMoves<KingRook>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);

      legalMoves.pieceMoves[TheQueen] = genLegalPieceMoves<TheQueen>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);
      
      // TODO other promo pieces
      // if(MyColorTraitsT::HasPromos) {
      // 	if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
      // 	  legalMoves.pieceMoves[PromoQueen] = genLegalPieceMoves<PromoQueen>(myAttacks, legalMoveMaskBb, pinMasks, allMyPiecesBb);
      // 	}
      // }
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
    inline BitBoardT genLegalKingMoves(const BoardT& board, const PieceBbsT& pieceBbs, const PieceAttacksT& yourAttacks, const BitBoardT allMyKingAttackersBb) {
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& myState = board.pieces[(size_t)Color];

      const ColorPieceBbsT& myPieceBbs = pieceBbs.colorPieceBbs[(size_t)Color];
      const ColorPieceBbsT& yourPieceBbs = pieceBbs.colorPieceBbs[(size_t)OtherColor];
      
      const BitBoardT allMyPiecesBb = myPieceBbs.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];
      
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;
      
      // King cannot move into check
      // King also cannot move away from a checking slider cos it's still in check.
      BitBoardT illegalKingSquaresBb = BbNone;
      const BitBoardT myKingBb = myPieceBbs.bbs[King];
      BitBoardT diagSliderCheckersBb = allMyKingAttackersBb & yourPieceBbs.sliderBbs[Diagonal];
      while(diagSliderCheckersBb) {
	const SquareT sliderSq = Bits::popLsb(diagSliderCheckersBb);
	illegalKingSquaresBb |= bishopAttacks(sliderSq, allPiecesBb & ~myKingBb);
      }
      BitBoardT orthogSliderCheckersBb = allMyKingAttackersBb & yourPieceBbs.sliderBbs[Orthogonal];
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
    
    template <typename ColorTraitsT>
    inline ColorPieceBbsT genColorPieceBbs(const ColorStateT& state) {
      ColorPieceBbsT pieceBbs = {};

      pieceBbs.bbs[Pawn] = state.pawnsBb; //bbsOld[Pawn];

      // TODO promos
      pieceBbs.bbs[Knight] = bbForSquare(state.pieceSquares[Knight1]) | bbForSquare(state.pieceSquares[Knight2]);
      pieceBbs.bbs[Bishop] = bbForSquare(state.pieceSquares[Bishop1]) | bbForSquare(state.pieceSquares[Bishop2]);
      pieceBbs.bbs[Rook] = bbForSquare(state.pieceSquares[QueenRook]) | bbForSquare(state.pieceSquares[KingRook]);
      pieceBbs.bbs[Queen] = bbForSquare(state.pieceSquares[TheQueen]);
      pieceBbs.bbs[King] = bbForSquare(state.pieceSquares[TheKing]);

      pieceBbs.sliderBbs[Diagonal] = pieceBbs.bbs[Bishop] | pieceBbs.bbs[Queen];
      pieceBbs.sliderBbs[Orthogonal] = pieceBbs.bbs[Rook] | pieceBbs.bbs[Queen];

      pieceBbs.bbs[AllPieceTypes] = pieceBbs.bbs[Pawn] | pieceBbs.bbs[Knight] | pieceBbs.bbs[Bishop] | pieceBbs.bbs[Rook] | pieceBbs.bbs[Queen] | pieceBbs.bbs[King];

      return pieceBbs;
    }
    
    template <typename BoardTraitsT>
    inline PieceBbsT genPieceBbs(const BoardT& board) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& myState = board.pieces[(size_t)Color];
      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
      
      PieceBbsT pieceBbs = {};

      pieceBbs.colorPieceBbs[(size_t)Color] = genColorPieceBbs<MyColorTraitsT>(myState);
      pieceBbs.colorPieceBbs[(size_t)OtherColor] = genColorPieceBbs<YourColorTraitsT>(yourState);

      return pieceBbs;
    }
    
    template <typename BoardTraitsT>
    inline LegalMovesT genLegalMoves(const BoardT& board) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;
      
      const ColorStateT& myState = board.pieces[(size_t)Color];
      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];

      LegalMovesT legalMoves = {};
      
      legalMoves.pieceBbs = genPieceBbs<BoardTraitsT>(board);
      const PieceBbsT& pieceBbs = legalMoves.pieceBbs;

      const ColorPieceBbsT& myPieceBbs = pieceBbs.colorPieceBbs[(size_t)Color];
      const ColorPieceBbsT& yourPieceBbs = pieceBbs.colorPieceBbs[(size_t)OtherColor];
      
      const BitBoardT allMyPiecesBb = myPieceBbs.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;
      
      // Generate moves
      const PieceAttacksT myAttacks = genPieceAttacks<MyColorTraitsT>(myState, allPiecesBb);

      // Is your king in check? If so we got here via an illegal move of the move-generator
      if((myAttacks.allAttacks & yourPieceBbs.bbs[King]) != 0) {
	// Illegal position - doesn't count
	legalMoves.isIllegalPos = true;
      }

      // This is now a legal position.

      // Evaluate check - eventually do this in the parent

      const SquareT myKingSq = myState.pieceSquares[TheKing];
      const SquareAttackersT myKingAttackers = genSquareAttackers<YourColorTraitsT>(myKingSq, yourPieceBbs, allPiecesBb);
      const BitBoardT allMyKingAttackersBb = myKingAttackers.pieceAttackers[AllPieceTypes];

      // Needed for castling and for king moves so evaluate this here.
      const PieceAttacksT yourAttacks = genPieceAttacks<YourColorTraitsT>(yourState, allPiecesBb);

      legalMoves.nChecks = Bits::count(allMyKingAttackersBb);

      // Double check can only be evaded by moving the king so only bother with other pieces if nChecks < 2
      if(legalMoves.nChecks < 2) {

	// If we're in check then the only legal moves are capture or blocking of the checking piece.
	const BitBoardT legalMoveMaskBb = genLegalMoveMaskBb<BoardTraitsT>(board, legalMoves.nChecks, allMyKingAttackersBb, myKingSq, allPiecesBb, yourAttacks);
	  
	// Calculate pinned piece move restrictions.
	const PiecePinMasksT pinMasks = genPinMasks<BoardTraitsT>(board, pieceBbs);

	// Filter legal non-king moves
	genLegalNonKingMoves<BoardTraitsT>(legalMoves, board, pieceBbs, myAttacks, legalMoveMaskBb, pinMasks);

	// Castling
	legalMoves.canCastleFlags = genLegalCastlingFlags<BoardTraitsT>(board, yourAttacks, allPiecesBb);
      }

      legalMoves.pieceMoves[TheKing] = genLegalKingMoves<BoardTraitsT>(board, pieceBbs, yourAttacks, allMyKingAttackersBb);

      legalMoves.directChecks = genDirectCheckMasks<Color>(yourState, allPiecesBb);
      legalMoves.discoveredChecks = genDiscoveryMasks<BoardTraitsT>(board, pieceBbs, legalMoves.pawnMoves.epCaptures.epLeftCaptureBb, legalMoves.pawnMoves.epCaptures.epRightCaptureBb);
      
      return legalMoves;
    }

    extern int countAttacks(const PieceAttacksT& pieceAttacks, const BitBoardT filterOut = BbNone, const BitBoardT filterInPawnTakes = BbAll);

    // TODO - this shouldn't be in this header file but it has awkward dependencies
    template <typename BoardTraitsT>
    extern bool isValid(const BoardT& board) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      //typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;

      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
      
      const PieceBbsT& pieceBbs = genPieceBbs<BoardTraitsT>(board);
      const ColorPieceBbsT& myPieceBbs = pieceBbs.colorPieceBbs[(size_t)Color];
      const ColorPieceBbsT& yourPieceBbs = pieceBbs.colorPieceBbs[(size_t)OtherColor];
      
      const BitBoardT allMyPiecesBb = myPieceBbs.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;
      
      const SquareT yourKingSq = yourState.pieceSquares[TheKing];
      const SquareAttackersT yourKingAttackers = genSquareAttackers<MyColorTraitsT>(yourKingSq, myPieceBbs, allPiecesBb);
      const BitBoardT allYourKingAttackersBb = yourKingAttackers.pieceAttackers[AllPieceTypes];

      return Board::isValid(board, allYourKingAttackersBb);
    }
    
  } // namespace MoveGen

  
} // namespace Chess

  
#endif //ndef MOVE_GEN_HPP
