#ifndef MOVE_GEN_HPP
#define MOVE_GEN_HPP

#include "types.hpp"
#include "board.hpp"

namespace Chess {

  using namespace Board;
  
  namespace MoveGen {

    //
    // Pawn move rules are specialised for White and Black respectively.
    //
    
    template <ColorT> BitBoardT pawnsLeftAttacks(const BitBoardT pawns);
    template <ColorT> BitBoardT pawnsRightAttacks(const BitBoardT pawns);
    template <ColorT> BitBoardT pawnsPushOne(const BitBoardT pawns, const BitBoardT allPieces);
    template <ColorT> BitBoardT pawnsPushTwo(const BitBoardT pawnOneMoves, const BitBoardT allPieces);

    template <> inline BitBoardT pawnsLeftAttacks<White>(const BitBoardT pawns) {
      // Pawns on file A can't take left.
      return (pawns & ~FileA) << 7;
    }
    
    template <> inline BitBoardT pawnsRightAttacks<White>(const BitBoardT pawns) {
      // Pawns on file H can't take right.
      return (pawns & ~FileH) << 9;
    }
    
    template <> inline BitBoardT pawnsPushOne<White>(const BitBoardT pawns, const BitBoardT allPieces) {
      // White pieces move up the board but are blocked by pieces of either color.
      return (pawns << 8) & ~allPieces;
    }
    
    template <> inline BitBoardT pawnsPushTwo<White>(const BitBoardT pawnOneMoves, const BitBoardT allPieces) {
      // Pawns that can reach the 3rd rank after a single move can move to the 4th rank too,
      //   unless blocked by pieces of either color.
      return ((pawnOneMoves & Rank3) << 8) & ~allPieces;
    }

    template <> inline BitBoardT pawnsLeftAttacks<Black>(const BitBoardT pawns) {
      // Pawns on file A can't take left.
      return (pawns & ~FileA) >> 9;
    }
    
    template <> inline BitBoardT pawnsRightAttacks<Black>(const BitBoardT pawns) {
      // Pawns on file H can't take right.
      return (pawns & ~FileH) >> 7;
    }
    
    template <> inline BitBoardT pawnsPushOne<Black>(const BitBoardT pawns, const BitBoardT allPieces) {
      // Black pieces move downp the board but are blocked by pieces of any color.
      return (pawns >> 8) & ~allPieces;
    }
    
    template <> inline BitBoardT pawnsPushTwo<Black>(const BitBoardT pawnOneMoves, const BitBoardT allPieces) {
      // Pawns that can reach the 6rd rank after a single move can move to the 5th rank too,
      //   unless blocked by pieces of either color.
      return ((pawnOneMoves & Rank6) >> 8) & ~allPieces;
    }

    //
    // Static attack tables for knights and king
    //

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
    inline BitBoardT bishopAttacks(const SquareT square, const BitBoardT allPieces) {
      const BitBoardT blockers = allPieces & BishopBlockers[square];
      const auto magicBbKey = (blockers * BishopMagicBbMultipliers[square]) >> (64 - BishopMagicBbIndexBits[square]);
      return BishopMagicBbTable[square][magicBbKey];
    }

    // Magic BB rook attacks
    inline BitBoardT rookAttacks(const SquareT square, const BitBoardT allPieces) {
      const BitBoardT blockers = allPieces & RookBlockers[square];
      const auto magicBbKey = (blockers * RookMagicBbMultipliers[square]) >> (64 - RookMagicBbIndexBits[square]);
      return RookMagicBbTable[square][magicBbKey];
    }

    struct PieceAttacksT {
      // Pawn attacks (and moves) - single bit board for all pawns for each move type.
      BitBoardT pawnsLeftAttacks;
      BitBoardT pawnsRightAttacks;
      BitBoardT pawnsPushOne;
      BitBoardT pawnsPushTwo;

      // Piece moves
      BitBoardT pieceAttacks[NSpecificPieceTypes];
      
      // Uncommon promo piece moves - one for each pawn - one for each promo piece except 2nd queen.
      BitBoardT promoPieceMoves[NPawns];
      
      BitBoardT allAttacks;
    };

    // Generate attacks/defenses for all pieces.
    // Note that move gen for a piece on InvalidSquare MUST always generate BbNone (and not SIGSEGV :P).
    // TODO - missing support for unusual promos.
    template <ColorT Color, typename BoardTraitsT>
    inline PieceAttacksT genPieceAttacks(const ColorStateT& colorState, const BitBoardT allPieces) {
      PieceAttacksT attacks = {0};

      // Pawns
      BitBoardT pawns = colorState.pawnsBb;
      attacks.pawnsLeftAttacks = pawnsLeftAttacks<Color>(pawns);
      attacks.pawnsRightAttacks = pawnsRightAttacks<Color>(pawns);
      
      attacks.allAttacks |= (attacks.pawnsLeftAttacks | attacks.pawnsRightAttacks);
      
      attacks.pawnsPushOne = pawnsPushOne<Color>(pawns, allPieces);
      attacks.pawnsPushTwo = pawnsPushTwo<Color>(attacks.pawnsPushOne, allPieces);

      // Knights
      
      SquareT queenKnightSquare = colorState.pieceSquares[QueenKnight];
      
      attacks.pieceAttacks[QueenKnight] = KnightAttacks[queenKnightSquare];
      
      attacks.allAttacks |= attacks.pieceAttacks[QueenKnight];
      
      SquareT kingKnightSquare = colorState.pieceSquares[KingKnight];
      
      attacks.pieceAttacks[KingKnight] = KnightAttacks[kingKnightSquare];
      
      attacks.allAttacks |= attacks.pieceAttacks[KingKnight];

      // Bishops

      SquareT blackBishopSquare = colorState.pieceSquares[BlackBishop];
      
      attacks.pieceAttacks[BlackBishop] = bishopAttacks(blackBishopSquare, allPieces);
      
      attacks.allAttacks |= attacks.pieceAttacks[BlackBishop];
      
      SquareT whiteBishopSquare = colorState.pieceSquares[WhiteBishop];
      
      attacks.pieceAttacks[WhiteBishop] = bishopAttacks(whiteBishopSquare, allPieces);
      
      attacks.allAttacks |= attacks.pieceAttacks[WhiteBishop];
      
      // Rooks

      SquareT queenRookSquare = colorState.pieceSquares[QueenRook];
      
      attacks.pieceAttacks[QueenRook] = rookAttacks(queenRookSquare, allPieces);
      
      attacks.allAttacks |= attacks.pieceAttacks[QueenRook];
      
      SquareT kingRookSquare = colorState.pieceSquares[KingRook];
      
      attacks.pieceAttacks[KingRook] = rookAttacks(kingRookSquare, allPieces);
      
      attacks.allAttacks |= attacks.pieceAttacks[KingRook];
      
      // Queens

      SquareT queenSquare = colorState.pieceSquares[SpecificQueen];
      
      attacks.pieceAttacks[SpecificQueen] = rookAttacks(queenSquare, allPieces) | bishopAttacks(queenSquare, allPieces);
      
      attacks.allAttacks |= attacks.pieceAttacks[SpecificQueen];

      // King - always 1 king and always present
      SquareT kingSquare = colorState.pieceSquares[SpecificKing];

      attacks.pieceAttacks[SpecificKing] = KingAttacks[kingSquare];

      attacks.allAttacks |= attacks.pieceAttacks[SpecificKing];

      // TODO - unusual promos
      if(BoardTraitsT::hasPromos) {
	if(true/*piecesPresent & PromoQueenPresentFlag*/) {
	  SquareT promoQueenSquare = colorState.pieceSquares[PromoQueen];
	  
	  attacks.pieceAttacks[PromoQueen] = rookAttacks(promoQueenSquare, allPieces) | bishopAttacks(promoQueenSquare, allPieces);
	  
	  attacks.allAttacks |= attacks.pieceAttacks[PromoQueen];
	}
      }
      
      return attacks;
    }

    extern int countAttacks(const PieceAttacksT& pieceAttacks, const BitBoardT filterOut = BbNone, const BitBoardT filterInPawnTakes = BbAll);
    
  } // namespace MoveGen

  
} // namespace Chess

  
#endif //ndef MOVE_GEN_HPP
