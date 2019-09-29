#ifndef MOVE_GEN_HPP
#define MOVE_GEN_HPP

#include "types.h"

#include "board.hpp"

namespace Chess {
  
  namespace MoveGen {
  
    struct PieceMovesT {
      // Pawn moves
      BitBoardT pawnAttackLeftMoves;
      BitBoardT pawnAttackRightMoves;
      BitBoardT pawnMoveOneMoves;
      BitBoardT pawnMoveTwoMoves;
      
      // Knight moves
      BitBoardT queenKnightMoves;
      BitBoardT kingKnightMoves;
      
      // Bishop moves
      BitBoardT blackBishopMoves;
      BitBoardT whiteBishopMoves;
      
      // Rook moves
      BitBoardT queenRookMoves;
      BitBoardT kingRookMoves;
      
      // Queen moves
      BitBoardT queenMoves;
      BitBoardT promoQueenMoves;
      
      // Uncommon promo piece moves - one for each pawn.
      BitBoardT promoPieceMoves[NPawns];
      
      BitBoardT allAttacks;
    };

    // Generate moves - or more generally generate attacks/defenses - the moveMask param will limit to valid moves.
    // Psuedo-moves only - in-check or moving into check are not included.
    // TODO excludes castling, en-passant and promo piece.
    template <ColorT, PiecePresentBitmapT PiecesPresent = AllPiecesPresentBits, boolean UseRuntimeChecks = true>
    inline PieceMovesT genPseudoMoves(const BoardT& board, BitBoardT moveMask) {
      PieceMovesT moves = {0};
      
      return moves;
    }
    
  } // namespace MoveGen

  
} // namespace Chess

  
#endif //ndef MOVE_GEN
