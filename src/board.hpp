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

      // All pieces except pawns and strange promos
      SquareT pieceSquares[NSpecificPieceTypes];

      // Bitmap of which pieces are still present on the board.
      PiecePresentFlagsT piecesPresent;

      // If non-zero, then en-passant square of the last move - i.e. the square behind a pawn two-square push.
      SquareT epSquare;
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
      // Only clear pawns-present flag when all pawns are gone
      if(specificPiece != SpecificPawn || pieces.bbs[Pawn] == BbNone) {
	pieces.piecesPresent &= ~piecePresentFlag;
      }
      
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

    inline BoardT copyForMove(const BoardT& oldBoard) {
      BoardT board = oldBoard;
      // En-passant squares are always lost after a move.
      board.pieces[White].epSquare = 0;
      board.pieces[Black].epSquare = 0;

      return board;
    }

    template <ColorT Color, PushOrCaptureT PushOrCapture, bool IsPawnPushTwo = false> inline BoardT move(const BoardT& oldBoard, const SquareT from, const SquareT to) {
      BoardT board = copyForMove(oldBoard);

      if(PushOrCapture == Capture) {
	removePiece<otherColor<Color>::value>(board, to);
      }

      SpecificPieceT specificPiece = removePiece<Color>(board, from);

      placePiece<Color>(board, specificPiece, to);

      // Set en-passant square
      if(IsPawnPushTwo) {
	board.pieces[Color].epSquare = (SquareT)((from+to)/2);
      }
      
      // TODO - castling rights?

      return board;
    }

    template <ColorT Color> inline BoardT captureEp(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      BoardT board = copyForMove(oldBoard);

      removePiece<otherColor<Color>::value>(board, captureSquare);

      SpecificPieceT specificPiece = removePiece<Color>(board, from);

      placePiece<Color>(board, specificPiece, to);
      
      return board;
    }
    
    extern BoardT startingPosition();
  }
    
}

#endif //ndef BOARD_HPP
