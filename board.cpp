#include "board.hpp"

namespace Chess {
  
  namespace Board {

    // TODO - PieceT should be inferred from SpecificPieceT
    // TODO - unusual promos
    static void addPiece(PiecesForColorT& c, const SquareT square, const PieceT piece, const SpecificPieceT specificPiece, const PiecePresentFlagsT piecePresentFlag) {
      BitBoardT pieceBb = bbForSquare(square);

      c.bbs[piece] |= pieceBb;
      c.bbs[AllPieces] |= pieceBb;

      c.piecesPresent |= piecePresentFlag;
      c.pieceSquares[specificPiece] = square;
    }

    static void addStartingPieces(PiecesForColorT& c, const SquareT firstPieceSquare, const SquareT firstPawnSquare) {
      // Pieces
      addPiece(c, firstPieceSquare, Rook, QueenRook, QueenRookPresentFlag);
      addPiece(c, firstPieceSquare+B1-A1, Knight, QueenKnight, QueenKnightPresentFlag);
      addPiece(c, firstPieceSquare+C1-A1, Bishop, BlackBishop, BlackBishopPresentFlag);
      addPiece(c, firstPieceSquare+D1-A1, Queen, SpecificQueen, QueenPresentFlag);
      addPiece(c, firstPieceSquare+E1-A1, King, SpecificKing, 0/*king always present*/);
      addPiece(c, firstPieceSquare+F1-A1, Bishop, WhiteBishop, WhiteBishopPresentFlag);
      addPiece(c, firstPieceSquare+G1-A1, Knight, KingKnight, KingKnightPresentFlag);
      addPiece(c, firstPieceSquare+H1-A1, Rook, KingRook, KingRookPresentFlag);
      
      // Pawns
      for(SquareT square = firstPawnSquare; square <= firstPawnSquare+H2-A2; square += (B2-A2)) {
	addPiece(c, square, Pawn, SpecificPawn/*dummy*/, PawnsPresentFlag);
      }
    }

    static void addWhiteStartingPieces(PiecesForColorT& w) {
      addStartingPieces(w, A1, A2);
    }
    
    static void addBlackStartingPieces(PiecesForColorT& b) {
      addStartingPieces(b, A1, A2);
    }
    
    BoardT startingPosition() {
      BoardT board = {0};

      addWhiteStartingPieces(board.pieces[White]);
      addBlackStartingPieces(board.pieces[Black]);
      
      return board;
    }
  }
}
