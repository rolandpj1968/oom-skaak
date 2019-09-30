#ifndef BOARD_H
#define BOARD_H

#include "types.hpp"

namespace Chess {
  
  namespace Board {

    struct PiecesForColorT {
      // All pieces including strange promos.
      // TODO - I don't think I need this!
      BitBoardT bbs[NPieceTypes];

      // Bitmap of which pieces are still present on the board.
      PiecePresentFlagsT piecesPresent;
      
      // All pieces except pawns and strange promos
      SquareT pieceSquares[NSpecificPieceTypes];
    };

    typedef u8 SquarePieceT;

    inline SquarePieceT makeSquarePiece(const ColorT color, const SpecificPieceT specificPiece) {
      return (color << 7) | specificPiece;
    }

    inline ColorT squarePieceColor(const SquarePieceT squarePiece) {
      return (ColorT)(squarePiece >> 7);
    }

    inline SpecificPieceT squarePieceSpecificPiece(const SquarePieceT squarePiece) {
      return (SpecificPieceT) (squarePiece & ~(Black << 7));
    }

    struct BoardT {
      PiecesForColorT pieces[NColors];

      SquarePieceT board[64];
    };

    template <ColorT Color, bool isCapture, bool isEnPassant> BoardT inline move(const BoardT& board, const SpecificPieceT specificPiece, const SquareT from, const SquareT to) {
      const PieceT piece = PieceForSpecificPiece[specificPiece];

      const BitBoardT pieceBb = bbForSquare(square);

      #$%@#$%@#$% here
    }
    
    extern BoardT startingPosition();
  }
  
}

#endif //ndef BOARD_H
