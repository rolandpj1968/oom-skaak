#ifndef MOVE_GEN_HPP
#define MOVE_GEN_HPP

#include "types.hpp"

#include "board.hpp"

namespace Chess {

  using namespace Board;
  
  namespace MoveGen {

    const BitBoardT FileA = 0x0101010101010101ULL;
    const BitBoardT FileB = 0x0202020202020202ULL;
    const BitBoardT FileC = 0x0404040404040404ULL;
    const BitBoardT FileD = 0x0808080808080808ULL;
    const BitBoardT FileE = 0x1010101010101010ULL;
    const BitBoardT FileF = 0x2020202020202020ULL;
    const BitBoardT FileG = 0x4040404040404040ULL;
    const BitBoardT FileH = 0x8080808080808080ULL;

    const BitBoardT Rank1 = 0x00000000000000ffULL;
    const BitBoardT Rank2 = 0X000000000000ff00ULL;
    const BitBoardT Rank3 = 0X0000000000ff0000ULL;
    const BitBoardT Rank4 = 0X00000000ff000000ULL;
    const BitBoardT Rank5 = 0X000000ff00000000ULL;
    const BitBoardT Rank6 = 0X0000ff0000000000ULL;
    const BitBoardT Rank7 = 0X00ff000000000000ULL;
    const BitBoardT Rank8 = 0Xff00000000000000ULL;

    //
    // Pawn move rules are specialised for White and Black respectively.
    //
    
    template <ColorT> BitBoardT pawnsLeftAttacks(BitBoardT pawns);
    template <ColorT> BitBoardT pawnsRightAttacks(BitBoardT pawns);
    template <ColorT> BitBoardT pawnsPushOne(BitBoardT pawns, BitBoardT allPieces);
    template <ColorT> BitBoardT pawnsPushTwo(BitBoardT pawnOneMoves, BitBoardT allPieces);

    template <> inline BitBoardT pawnsLeftAttacks<White>(BitBoardT pawns) {
      // Pawns on rank A can't take left.
      return (pawns & ~FileA) << 7;
    }
    
    template <> inline BitBoardT pawnsRightAttacks<White>(BitBoardT pawns) {
      // Pawns on rank H can't take right.
      return (pawns & ~FileH) << 9;
    }
    
    template <> inline BitBoardT pawnsPushOne<White>(BitBoardT pawns, BitBoardT allPieces) {
      // White pieces move up the board but are blocked by pieces of either color.
      return (pawns << 8) & ~allPieces;
    }
    
    template <> inline BitBoardT pawnsPushTwo<White>(BitBoardT pawnOneMoves, BitBoardT allPieces) {
      // Pawns that can reach the 3rd rank after a single move can move to the 4th rank too,
      //   unless blocked by pieces of either color.
      return ((pawnOneMoves & Rank3) << 8) & ~allPieces;
    }

    template <> inline BitBoardT pawnsLeftAttacks<Black>(BitBoardT pawns) {
      // Pawns on rank A can't take left.
      return (pawns & ~FileA) >> 9;
    }
    
    template <> inline BitBoardT pawnsRightAttacks<Black>(BitBoardT pawns) {
      // Pawns on rank H can't take right.
      return (pawns & ~FileH) >> 7;
    }
    
    template <> inline BitBoardT pawnsPushOne<Black>(BitBoardT pawns, BitBoardT allPieces) {
      // Black pieces move downp the board but are blocked by pieces of any color.
      return (pawns >> 8) & ~allPieces;
    }
    
    template <> inline BitBoardT pawnsPushTwo<Black>(BitBoardT pawnOneMoves, BitBoardT allPieces) {
      // Pawns that can reach the 6rd rank after a single move can move to the 5th rank too,
      //   unless blocked by pieces of either color.
      return ((pawnOneMoves & Rank6) >> 8) & ~allPieces;
    }

    const u64 aaaaa[2] = { 1, 2 };

    const BitBoardT KnightAttacks[64] = { 
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
    };

    const BitBoardT KingAttacks[64] = {
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
    };
  
    struct PieceAttacksT {
      // Pawn moves
      BitBoardT pawnsLeftAttacks;
      BitBoardT pawnsRightAttacks;
      BitBoardT pawnsPushOne;     // Not actually attacks - possibly remove
      BitBoardT pawnsPushTwo;     // Not actually attacks - possible remove

      // Piece moves
      BitBoardT pieceAttacks[NSpecificPieceTypes];
      
      // Uncommon promo piece moves - one for each pawn - one for each promo piece except 2nd queen.
      BitBoardT promoPieceMoves[NPawns];
      
      BitBoardT allAttacks;
    };

    // Generate attacks/defenses for all pieces.
    template <ColorT Color, PiecePresentFlagsT PiecesPresent = AllPiecesPresentFlags, bool UseRuntimeChecks = true>
    inline PieceAttacksT genPieceAttacks(const PiecesForColorT& board, BitBoardT allPieces) {
      PieceAttacksT attacks = {0};

      // Pawns
      if((PiecesPresent & PawnsPresentFlag) || (UseRuntimeChecks && (board.piecesPresent & PawnsPresentFlag))) {
	BitBoardT pawns = board.bbs[Pawn];
	attacks.pawnsLeftAttacks = pawnsLeftAttacks<Color>(pawns);
	attacks.pawnsRightAttacks = pawnsRightAttacks<Color>(pawns);

	attacks.allAttacks |= (attacks.pawnsLeftAttacks | attacks.pawnsRightAttacks);

	attacks.pawnsPushOne = pawnsPushOne<Color>(pawns, allPieces);
	attacks.pawnsPushTwo = pawnsPushTwo<Color>(attacks.pawnsPushOne, allPieces);
      }

      // Knights
      
      if((PiecesPresent & QueenKnightPresentFlag) || (UseRuntimeChecks && (board.piecesPresent & QueenKnightPresentFlag))) {
	SquareT queenKnightSquare = board.pieceSquares[QueenKnight];

	attacks.pieceAttacks[QueenKnight] = KnightAttacks[queenKnightSquare];

	attacks.allAttacks |= attacks.pieceAttacks[QueenKnight];
      }
      
      if((PiecesPresent & KingKnightPresentFlag) || (UseRuntimeChecks && (board.piecesPresent & KingKnightPresentFlag))) {
	SquareT kingKnightSquare = board.pieceSquares[KingKnight];

	attacks.pieceAttacks[KingKnight] = KnightAttacks[kingKnightSquare];

	attacks.allAttacks |= attacks.pieceAttacks[KingKnight];
      }

      // King - always 1 king and always present
      SquareT kingSquare = board.pieceSquares[SpecificKing];

      attacks.pieceAttacks[SpecificKing] = KingAttacks[kingSquare];

      attacks.allAttacks |= attacks.pieceAttacks[SpecificKing];
      
      return attacks;
    }
    
  } // namespace MoveGen

  
} // namespace Chess

  
#endif //ndef MOVE_GEN
