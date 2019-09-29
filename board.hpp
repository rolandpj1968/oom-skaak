#ifndef BOARD_H
#define BOARD_H

#include "types.h"

namespace Chess {
  
  namespace Board {

    struct BoardForColorT {
      // All pieces including strange promos.
      BitBoardT bbs[NPieceTypes];

      // Bitmap of which pieces are still present on the board.
      PiecePresentBitmapT piecesPresent;
      
      // All pieces except pawns and strange promos
      SquareT pieceSquares[NSpecificPieceTypes];
    }
    
  }
  
}

#endif //ndef BOARD_H
