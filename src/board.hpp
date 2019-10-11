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

    struct ColorStateT {
      // All pieces including strange promos.
      BitBoardT bbs[NPieceTypes];

      // All pieces except pawns and strange promos.
      // MUST be InvalidSquare if a piece is not present
      SquareT pieceSquares[NSpecificPieceTypes];

      // True iff there are promo pieces on the board.
      bool hasPromos;

      // If non-zero, then en-passant square of the last move - i.e. the square behind a pawn two-square push.
      SquareT epSquare;

      // Castling rights
      CastlingRightsT castlingRights;
    };

    struct BoardT {
      ColorStateT pieces[NColors];

      SquarePieceT board[64];
    };

    template <ColorT Color> inline SpecificPieceT removePiece(BoardT& board, const SquareT square) {
      const SquarePieceT squarePiece = board.board[square];
      const SpecificPieceT specificPiece = squarePieceSpecificPiece(squarePiece);

      board.board[square] = EmptySquare;
      
      // TODO handle non-standard promos
      ColorStateT &pieces = board.pieces[Color];

      pieces.pieceSquares[specificPiece] = InvalidSquare;

      const BitBoardT squareBb = bbForSquare(square);

      const PieceT piece = PieceForSpecificPiece[specificPiece];
      pieces.bbs[piece] &= ~squareBb;
      pieces.bbs[AllPieces] &= ~squareBb;
      
      return specificPiece;
    }

    template <ColorT Color, SpecificPieceT SpecificPiece> inline void removeSpecificPiece(BoardT& board, const SquareT square) {
      board.board[square] = EmptySquare;
      
      // TODO handle non-standard promos
      ColorStateT &pieces = board.pieces[Color];

      pieces.pieceSquares[SpecificPiece] = InvalidSquare;

      const BitBoardT squareBb = bbForSquare(square);

      const PieceT piece = PieceForSpecificPieceT<SpecificPiece>::value;
      pieces.bbs[piece] &= ~squareBb;
      pieces.bbs[AllPieces] &= ~squareBb;
    }

    template <ColorT Color> inline void placePiece(BoardT& board, const SpecificPieceT specificPiece, const SquareT square) {
      board.board[square] = makeSquarePiece(Color, specificPiece);

      // TODO handle non-standard promos
      ColorStateT &pieces = board.pieces[Color];

      pieces.pieceSquares[specificPiece] = square;

      const BitBoardT squareBb = bbForSquare(square);

      const PieceT piece = PieceForSpecificPiece[specificPiece];
      pieces.bbs[piece] |= squareBb;
      pieces.bbs[AllPieces] |= squareBb;
    }

    // TODO - non-standard promos
    template <ColorT Color, SpecificPieceT SpecificPiece> inline void placeSpecificPiece(BoardT& board, const SquareT square) {
      board.board[square] = makeSquarePiece(Color, SpecificPiece);

      // TODO handle non-standard promos
      ColorStateT &pieces = board.pieces[Color];

      pieces.pieceSquares[SpecificPiece] = square;

      const BitBoardT squareBb = bbForSquare(square);

      const PieceT piece = PieceForSpecificPieceT<SpecificPiece>::value;
      pieces.bbs[piece] |= squareBb;
      pieces.bbs[AllPieces] |= squareBb;
    }

    inline BoardT copyForMove(const BoardT& oldBoard) {
      BoardT board = oldBoard;
      // En-passant squares are always lost after a move.
      // Only actually need to clear one side's square, so could template this on Color...
      board.pieces[White].epSquare = 0;
      board.pieces[Black].epSquare = 0;

      return board;
    }

    template <ColorT Color, PushOrCaptureT PushOrCapture, bool IsPawnPushTwo = false> inline BoardT move(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      BoardT board = copyForMove(oldBoard);

      if(PushOrCapture == Capture) {
    	removePiece<OtherColorT<Color>::value>(board, captureSquare);
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
    
    template <ColorT Color, SpecificPieceT SpecificPiece, PushOrCaptureT PushOrCapture, bool IsPawnPushTwo = false> inline BoardT moveSpecificPiece(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      BoardT board = copyForMove(oldBoard);

      if(PushOrCapture == Capture) {
	removePiece<OtherColorT<Color>::value>(board, captureSquare);
      }

      removeSpecificPiece<Color, SpecificPiece>(board, from);

      placeSpecificPiece<Color, SpecificPiece>(board, to);

      // Set en-passant square
      if(IsPawnPushTwo) {
	board.pieces[Color].epSquare = (SquareT)((from+to)/2);
      }
      
      // TODO - castling rights?

      return board;
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece, SpecificPieceT SpecificPieceCaptured> inline BoardT captureSpecificPiece(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      BoardT board = copyForMove(oldBoard);

      removeSpecificPiece<OtherColorT<Color>::value, SpecificPieceCaptured>(board, captureSquare);

      removeSpecificPiece<Color, SpecificPiece>(board, from);

      placeSpecificPiece<Color, SpecificPiece>(board, to);

      return board;
    }
    
    template <ColorT Color, PushOrCaptureT PushOrCapture, bool IsPawnPushTwo = false> inline BoardT move(const BoardT& oldBoard, const SquareT from, const SquareT to) {
      return move<Color, PushOrCapture, IsPawnPushTwo>(oldBoard, from, to, /*captureSquare = */to);
    }

    template <ColorT Color, SpecificPieceT SpecificPiece, PushOrCaptureT PushOrCapture, bool IsPawnPushTwo = false> inline BoardT moveSpecificPiece(const BoardT& oldBoard, const SquareT from, const SquareT to) {
      return moveSpecificPiece<Color, SpecificPiece, PushOrCapture, IsPawnPushTwo>(oldBoard, from, to, /*captureSquare = */to);
    }

    template <ColorT Color> inline BoardT captureEp(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      return captureSpecificPiece<Color, SpecificPawn, SpecificPawn>(oldBoard, from, to, captureSquare);
    }
    
    extern BoardT startingPosition();
  }
    
}

#endif //ndef BOARD_HPP
