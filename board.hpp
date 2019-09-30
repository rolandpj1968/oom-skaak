#ifndef BOARD_H
#define BOARD_H

#include "types.hpp"

namespace Chess {
  
  namespace Board {

    struct PiecesForColorT {
      // All pieces including strange promos.
      BitBoardT bbs[NPieceTypes];

      // Bitmap of which pieces are still present on the board.
      PiecePresentFlagsT piecesPresent;
      
      // All pieces except pawns and strange promos
      SquareT pieceSquares[NSpecificPieceTypes];
    };

    struct BoardT {
      PiecesForColorT pieces[NColors];
    };

    extern BoardT startingPosition();
  }
  
}

#endif //ndef BOARD_H
