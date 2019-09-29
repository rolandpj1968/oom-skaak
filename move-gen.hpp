#ifndef MOVE_GEN_HPP
#define MOVE_GEN_HPP

#include "types.hpp"

#include "board.hpp"

namespace Chess {

  using namespace Board;
  
  namespace MoveGen {
  
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
    // Psuedo-moves only - in-check or moving into check are not included.
    // TODO excludes castling, en-passant and promo piece.
    template <ColorT Color, PiecePresentFlagsT PiecesPresent = AllPiecesPresentFlags, bool UseRuntimeChecks = true>
    inline PieceMovesT genPseudoMoves(const BoardForColorT& board, BitBoardT allPieces) {
      PieceMovesT moves = {0};

      // Pawns
      if((PiecesPresent & PawnsPresentFlag) || (UseRuntimeChecks && (board.piecesPresent & PawnsPresentFlag))) {
	BitBoardT pawns = board.bbs[Pawn];
	moves.pawnAttackLeftMoves = pawnAttackLeftMoves<Color>(pawns);
	moves.pawnAttackRightMoves = pawnAttackRightMoves<Color>(pawns);

	moves.allAttacks |= (moves.pawnAttackLeftMoves | moves.pawnAttackRightMoves);

	moves.pawnMoveOneMoves = pawnMoveOneMoves<Color>(pawns, allPieces);
	moves.pawnMoveTwoMoves = pawnMoveTwoMoves<Color>(moves.pawnMoveOneMoves, allPieces);
      }

      // Knights
      if((PiecesPresent & QueenKnightPresentFlag) || (UseRuntimeChecks && (board.piecesPresent & QueenKnightPresentFlag))) {
      
      return moves;
    }
    
  } // namespace MoveGen

  
} // namespace Chess

  
#endif //ndef MOVE_GEN
