#include "board.hpp"

namespace Chess {
  
  namespace Board {

    // TODO - PieceT should be inferred from SpecificPieceT
    // TODO - unusual promos
    static void addPiece(BoardT& board, const ColorT color, const SquareT square, const SpecificPieceT specificPiece, const PiecePresentFlagsT piecePresentFlag) {
      PiecesForColorT& c = board.pieces[color];
      
      const BitBoardT pieceBb = bbForSquare(square);

      c.bbs[PieceForSpecificPiece[specificPiece]] |= pieceBb;
      c.bbs[AllPieces] |= pieceBb;

      c.piecesPresent |= piecePresentFlag;
      c.pieceSquares[specificPiece] = square;

      board.board[square] = makeSquarePiece(color, specificPiece);
    }

    static void addStartingPieces(BoardT& board, const ColorT color, const SquareT firstPieceSquare, const SquareT firstPawnSquare) {
      // Pieces
      addPiece(board, color, firstPieceSquare,       QueenRook, QueenRookPresentFlag);
      addPiece(board, color, firstPieceSquare+B1-A1, QueenKnight, QueenKnightPresentFlag);
      addPiece(board, color, firstPieceSquare+C1-A1, BlackBishop, BlackBishopPresentFlag);
      addPiece(board, color, firstPieceSquare+D1-A1, SpecificQueen, QueenPresentFlag);
      addPiece(board, color, firstPieceSquare+E1-A1, SpecificKing, 0/*king always present*/);
      addPiece(board, color, firstPieceSquare+F1-A1, WhiteBishop, WhiteBishopPresentFlag);
      addPiece(board, color, firstPieceSquare+G1-A1, KingKnight, KingKnightPresentFlag);
      addPiece(board, color, firstPieceSquare+H1-A1, KingRook, KingRookPresentFlag);
      
      // Pawns
      for(SquareT square = firstPawnSquare; square <= firstPawnSquare+H2-A2; square += (B2-A2)) {
	addPiece(board, color, square, SpecificPawn, PawnsPresentFlag);
      }
    }

    BoardT startingPosition() {
      BoardT board = {0};

      addStartingPieces(board, White, A1, A2);
      addStartingPieces(board, Black, A8, A7);
      
      return board;
    }
  }
}
