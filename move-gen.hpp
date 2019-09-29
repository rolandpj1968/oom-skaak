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

// Only activate one file, A-H (A=0, H=7)
//var onlyFile = [8]uint64{
  
    const BitBoardT Rank1 = 0x00000000000000FFULL;
    const BitBoardT Rank2 = 0X000000000000FF00ULL;
    const BitBoardT Rank3 = 0X0000000000FF0000ULL;
    const BitBoardT Rank4 = 0X00000000FF000000ULL;
    const BitBoardT Rank5 = 0X000000FF00000000ULL;
    const BitBoardT Rank6 = 0X0000FF0000000000ULL;
    const BitBoardT Rank7 = 0X00FF000000000000ULL;
    const BitBoardT Rank8 = 0XFF00000000000000ULL;

    //
    // Pawn move rules are specialised for White and Black respectively.
    //
    
    template <ColorT> BitBoardT pawnAttackLeftMoves(BitBoardT pawns);
    template <ColorT> BitBoardT pawnAttackRightMoves(BitBoardT pawns);
    template <ColorT> BitBoardT pawnMoveOneMoves(BitBoardT pawns, BitBoardT allPieces);
    template <ColorT> BitBoardT pawnMoveTwoMoves(BitBoardT pawnOneMoves, BitBoardT allPieces);

    template <> inline BitBoardT pawnAttackLeftMoves<White>(BitBoardT pawns) {
      // Pawns on rank A can't take left.
      return (pawns & ~FileA) << 7;
    }
    
    template <> inline BitBoardT pawnAttackRightMoves<White>(BitBoardT pawns) {
      // Pawns on rank H can't take right.
      return (pawns & ~FileH) << 9;
    }
    
    template <> inline BitBoardT pawnMoveOneMoves<White>(BitBoardT pawns, BitBoardT allPieces) {
      // White pieces move up the board but are blocked by pieces of either color.
      return (pawns << 8) & ~allPieces;
    }
    
    template <> inline BitBoardT pawnMoveTwoMoves<White>(BitBoardT pawnOneMoves, BitBoardT allPieces) {
      // Pawns that can reach the 3rd rank after a single move can move to the 4th rank too,
      //   unless blocked by pieces of either color.
      return ((pawnOneMoves & Rank3) << 8) & ~allPieces;
    }

    template <> inline BitBoardT pawnAttackLeftMoves<Black>(BitBoardT pawns) {
      // Pawns on rank A can't take left.
      return (pawns & ~FileA) >> 9;
    }
    
    template <> inline BitBoardT pawnAttackRightMoves<Black>(BitBoardT pawns) {
      // Pawns on rank H can't take right.
      return (pawns & ~FileH) >> 7;
    }
    
    template <> inline BitBoardT pawnMoveOneMoves<Black>(BitBoardT pawns, BitBoardT allPieces) {
      // Black pieces move downp the board but are blocked by pieces of any color.
      return (pawns >> 8) & ~allPieces;
    }
    
    template <> inline BitBoardT pawnMoveTwoMoves<Black>(BitBoardT pawnOneMoves, BitBoardT allPieces) {
      // Pawns that can reach the 6rd rank after a single move can move to the 5th rank too,
      //   unless blocked by pieces of either color.
      return ((pawnOneMoves & Rank6) >> 8) & ~allPieces;
    }

    const u64 aaaaa[2] = { 1, 2 };

// // Masks for attacks
// // In order: knight on A1, B1, C1, ... F8, G8, H8
// var knightMasks = [64]uint64{
// 	0x0000000000020400, 0x0000000000050800, 0x00000000000a1100, 0x0000000000142200,
// 	0x0000000000284400, 0x0000000000508800, 0x0000000000a01000, 0x0000000000402000,
// 	0x0000000002040004, 0x0000000005080008, 0x000000000a110011, 0x0000000014220022,
// 	0x0000000028440044, 0x0000000050880088, 0x00000000a0100010, 0x0000000040200020,
// 	0x0000000204000402, 0x0000000508000805, 0x0000000a1100110a, 0x0000001422002214,
// 	0x0000002844004428, 0x0000005088008850, 0x000000a0100010a0, 0x0000004020002040,
// 	0x0000020400040200, 0x0000050800080500, 0x00000a1100110a00, 0x0000142200221400,
// 	0x0000284400442800, 0x0000508800885000, 0x0000a0100010a000, 0x0000402000204000,
// 	0x0002040004020000, 0x0005080008050000, 0x000a1100110a0000, 0x0014220022140000,
// 	0x0028440044280000, 0x0050880088500000, 0x00a0100010a00000, 0x0040200020400000,
// 	0x0204000402000000, 0x0508000805000000, 0x0a1100110a000000, 0x1422002214000000,
// 	0x2844004428000000, 0x5088008850000000, 0xa0100010a0000000, 0x4020002040000000,
// 	0x0400040200000000, 0x0800080500000000, 0x1100110a00000000, 0x2200221400000000,
// 	0x4400442800000000, 0x8800885000000000, 0x100010a000000000, 0x2000204000000000,
// 	0x0004020000000000, 0x0008050000000000, 0x00110a0000000000, 0x0022140000000000,
// 	0x0044280000000000, 0x0088500000000000, 0x0010a00000000000, 0x0020400000000000}

// var kingMasks = [64]uint64{
// 	0x0000000000000302, 0x0000000000000705, 0x0000000000000e0a, 0x0000000000001c14,
// 	0x0000000000003828, 0x0000000000007050, 0x000000000000e0a0, 0x000000000000c040,
// 	0x0000000000030203, 0x0000000000070507, 0x00000000000e0a0e, 0x00000000001c141c,
// 	0x0000000000382838, 0x0000000000705070, 0x0000000000e0a0e0, 0x0000000000c040c0,
// 	0x0000000003020300, 0x0000000007050700, 0x000000000e0a0e00, 0x000000001c141c00,
// 	0x0000000038283800, 0x0000000070507000, 0x00000000e0a0e000, 0x00000000c040c000,
// 	0x0000000302030000, 0x0000000705070000, 0x0000000e0a0e0000, 0x0000001c141c0000,
// 	0x0000003828380000, 0x0000007050700000, 0x000000e0a0e00000, 0x000000c040c00000,
// 	0x0000030203000000, 0x0000070507000000, 0x00000e0a0e000000, 0x00001c141c000000,
// 	0x0000382838000000, 0x0000705070000000, 0x0000e0a0e0000000, 0x0000c040c0000000,
// 	0x0003020300000000, 0x0007050700000000, 0x000e0a0e00000000, 0x001c141c00000000,
// 	0x0038283800000000, 0x0070507000000000, 0x00e0a0e000000000, 0x00c040c000000000,
// 	0x0302030000000000, 0x0705070000000000, 0x0e0a0e0000000000, 0x1c141c0000000000,
// 	0x3828380000000000, 0x7050700000000000, 0xe0a0e00000000000, 0xc040c00000000000,
// 	0x0203000000000000, 0x0507000000000000, 0x0a0e000000000000, 0x141c000000000000,
// 	0x2838000000000000, 0x5070000000000000, 0xa0e0000000000000, 0x40c0000000000000}
  
    struct PieceMovesT {
      // Pawn moves
      BitBoardT pawnAttackLeftMoves;
      BitBoardT pawnAttackRightMoves;
      BitBoardT pawnMoveOneMoves;
      BitBoardT pawnMoveTwoMoves;

      // Piece moves
      BitBoardT pieceMoves[NSpecificPieceTypes];
      
      // Uncommon promo piece moves - one for each pawn - one for each promo piece except 2nd queen.
      BitBoardT promoPieceMoves[NPawns];
      
      BitBoardT allAttacks;
    };

    // Generate moves - or more generally generate attacks/defenses - the moveMask param will limit to valid moves.
    // Psuedo-moves only - in-check or moving-into-check are not considered.
    // TODO excludes castling, en-passant and promo piece.
    template <ColorT Color, PiecePresentFlagsT PiecesPresent = AllPiecesPresentFlags, bool UseRuntimeChecks = true>
    inline PieceMovesT genPseudoMoves(const BoardForColorT& board, BitBoardT allPieces) {
      PieceMovesT moves = {0};

      // Pawns
      if((PiecesPresent & PawnsPresentFlag) || (UseRuntimeChecks && (board.piecesPresent & PawnsPresentFlag))) {
	BitBoardT pawns = board.bbs[Pawn];
	moves.pawnAttackLeftMoves = pawnAttackLeftMoves<Color>(pawns);
	moves.pawnAttackRightMoves = pawnAttackRightMoves<Color>(pawns);

	// moves.allAttacks |= (moves.pawnAttackLeftMoves | moves.pawnAttackRightMoves);

	moves.pawnMoveOneMoves = pawnMoveOneMoves<Color>(pawns, allPieces);
	moves.pawnMoveTwoMoves = pawnMoveTwoMoves<Color>(moves.pawnMoveOneMoves, allPieces);
      }

      // Knights
      if((PiecesPresent & QueenKnightPresentFlag) || (UseRuntimeChecks && (board.piecesPresent & QueenKnightPresentFlag))) {
      }
      
      return moves;
    }
    
  } // namespace MoveGen

  
} // namespace Chess

  
#endif //ndef MOVE_GEN
