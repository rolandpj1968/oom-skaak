#include "board.hpp"

namespace Chess {
  
  namespace Board {

    // TODO - PieceT should be inferred from SpecificPieceT
    // TODO - unusual promos
    static void addPiece(BoardT& board, const ColorT color, const SquareT square, const SpecificPieceT specificPiece) {
      ColorStateT& c = board.pieces[color];
      
      const BitBoardT pieceBb = bbForSquare(square);

      c.bbs[PieceForSpecificPiece[specificPiece]] |= pieceBb;
      c.bbs[AllPieces] |= pieceBb;

      c.pieceSquares[specificPiece] = square;

      board.board[square] = makeSquarePiece(color, specificPiece);
    }

    static void addStartingPieces(BoardT& board, const ColorT color, const SquareT firstPieceSquare, const SquareT firstPawnSquare) {
      // Pieces
      addPiece(board, color, firstPieceSquare,       QueenRook);
      addPiece(board, color, firstPieceSquare+B1-A1, QueenKnight);
      addPiece(board, color, firstPieceSquare+C1-A1, BlackBishop);
      addPiece(board, color, firstPieceSquare+D1-A1, SpecificQueen);
      addPiece(board, color, firstPieceSquare+E1-A1, SpecificKing);
      addPiece(board, color, firstPieceSquare+F1-A1, WhiteBishop);
      addPiece(board, color, firstPieceSquare+G1-A1, KingKnight);
      addPiece(board, color, firstPieceSquare+H1-A1, KingRook);
      
      // Pawns
      for(SquareT square = firstPawnSquare; square <= firstPawnSquare+H2-A2; square += (B2-A2)) {
	addPiece(board, color, square, SpecificPawn);
      }

      // Castling rights
      board.pieces[color].castlingRights = (CastlingRightsT)(CanCastleQueenside | CanCastleKingside);
    }

    BoardT startingPosition() {
      BoardT board = {0};

      addStartingPieces(board, White, A1, A2);
      addStartingPieces(board, Black, A8, A7);
      
      return board;
    }
  }
}
