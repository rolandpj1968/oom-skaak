#ifndef BOARD_HPP
#define BOARD_HPP

#include "types.hpp"

namespace Chess {
  
  namespace Board {

    typedef u8 SquarePieceT;

    const SquarePieceT EmptySquare = SpecificNoPiece;

    inline SquarePieceT makeSquarePiece(const ColorT color, const SpecificPieceT specificPiece) {
      return (color << 7) | specificPiece;
    }

    inline ColorT squarePieceColor(const SquarePieceT squarePiece) {
      return (ColorT)(squarePiece >> 7);
    }

    inline SpecificPieceT squarePieceSpecificPiece(const SquarePieceT squarePiece) {
      return (SpecificPieceT) (squarePiece & ~(Black << 7));
    }

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

      SquarePieceT board[64];
    };

    template <ColorT Color> inline SpecificPieceT removePiece(BoardT& board, const SquareT square) {
      const SquarePieceT squarePiece = board.board[square];
      const SpecificPieceT specificPiece = squarePieceSpecificPiece(squarePiece);

      board.board[square] = EmptySquare;
      
      // TODO handle non-standard promos
      PiecesForColorT &pieces = board.pieces[Color];

      pieces.pieceSquares[specificPiece] = InvalidSquare;

      const BitBoardT squareBb = bbForSquare(square);

      const PieceT piece = PieceForSpecificPiece[specificPiece];
      pieces.bbs[piece] &= ~squareBb;
      pieces.bbs[AllPieces] &= ~squareBb;

      // TODO brokken for non-standard promos
      const PiecePresentFlagsT piecePresentFlag = PresentFlagForSpecificPiece[specificPiece];
      pieces.piecesPresent &= ~piecePresentFlag;
      
      return specificPiece;
    }

    // TODO - non-standard promos
    template <ColorT Color> inline void placePiece(BoardT& board, const SpecificPieceT specificPiece, const SquareT square) {
      board.board[square] = makeSquarePiece(Color, specificPiece);

      // TODO handle non-standard promos
      PiecesForColorT &pieces = board.pieces[Color];

      pieces.pieceSquares[specificPiece] = square;

      const BitBoardT squareBb = bbForSquare(square);

      const PieceT piece = PieceForSpecificPiece[specificPiece];
      pieces.bbs[piece] |= squareBb;
      pieces.bbs[AllPieces] |= squareBb;

      // TODO brokken for non-standard promos
      const PiecePresentFlagsT piecePresentFlag = PresentFlagForSpecificPiece[specificPiece];
      pieces.piecesPresent |= piecePresentFlag;
    }

    template <ColorT Color, bool isCapture> inline BoardT move(const BoardT& oldBoard, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      // TODO en-passant has different take square
      if(isCapture) {
	removePiece<otherColor<Color>::value>(board, to);
      }

      SpecificPieceT specificPiece = removePiece<Color>(board, from);

      placePiece<Color>(board, specificPiece, to);
      
      // TODO - castling rights?

      return board;
    }
    
    extern BoardT startingPosition();
  }
  
}

#endif //ndef BOARD_HPP
