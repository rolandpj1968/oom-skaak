#include <cstdio>
#include "board.hpp"

namespace Chess {
  
  namespace Board {

    // TODO - unusual promos
    static void addPiece(BoardT& board, const ColorT color, const SquareT square, const PieceT piece) {
      ColorStateT& c = board.pieces[(size_t)color];
      
      // const BitBoardT pieceBb = bbForSquare(square);

      // c.bbsOld[PieceTypeForPiece[piece]] |= pieceBb;
      // c.bbsOld[AllPieceTypes] |= pieceBb;

      c.pieceSquares[piece] = square;

      //board.board[square] = makeSquarePiece(color, piece);
    }

    static void addPawn(BoardT& board, const ColorT color, const SquareT square) {
      ColorStateT& c = board.pieces[(size_t)color];
      
      const BitBoardT pieceBb = bbForSquare(square);

      c.pawnsBb/*bbsOld[PieceTypeForPiece[piece]]*/ |= pieceBb;
      // c.bbsOld[AllPieceTypes] |= pieceBb;

      // c.pieceSquares[piece] = square;

      //board.board[square] = makeSquarePiece(color, SomePawns);
    }
    
    static void addStartingPieces(BoardT& board, const ColorT color, const SquareT firstPieceSquare, const SquareT firstPawnSquare) {
      // Pieces
      addPiece(board, color, firstPieceSquare,       QueenRook);
      addPiece(board, color, firstPieceSquare+B1-A1, QueenKnight);
      addPiece(board, color, firstPieceSquare+C1-A1, BlackBishop);
      addPiece(board, color, firstPieceSquare+D1-A1, TheQueen);
      addPiece(board, color, firstPieceSquare+E1-A1, TheKing);
      addPiece(board, color, firstPieceSquare+F1-A1, WhiteBishop);
      addPiece(board, color, firstPieceSquare+G1-A1, KingKnight);
      addPiece(board, color, firstPieceSquare+H1-A1, KingRook);
      
      // Pawns
      for(SquareT square = firstPawnSquare; square <= firstPawnSquare+H2-A2; square += (B2-A2)) {
	addPawn(board, color, square);
      }

      // Castling rights
      board.pieces[(size_t)color].castlingRights = (CastlingRightsT)(CanCastleQueenside | CanCastleKingside);
    }

    BoardT startingPosition() {
      BoardT board = {0};

      addStartingPieces(board, White, A1, A2);
      addStartingPieces(board, Black, A8, A7);
      
      return board;
    }

    static char PieceChar[NColors][NPieceTypes+1] = {
      // White
      { ".PNBRQK" },
      // Black
      { ".pnbrqk" }
    };

    static void printRank(const BoardT& board, int rank) {
      printf("%d | ", rank+1);
      for(int file = 0; file < 8; file++) {
	//SquareT square = (SquareT)((rank << 3) + file);
	SquarePieceT squarePiece = NoPiece; //board.board[square];
	ColorT color = squarePieceColor(squarePiece);
	PieceT piece = squarePiecePiece(squarePiece);
	PieceTypeT pieceType = PieceTypeForPiece[piece];
	printf("%c ", PieceChar[(size_t)color][pieceType]);
      }
      printf(" | %d\n", rank+1);
    }

    void printBoard(const BoardT& board) {
      printf("    A B C D E F G H\n");
      printf("    ---------------\n");
      for(int rank = 7; rank >= 0; rank--) { 
	printRank(board, rank);
      }
      printf("    ---------------\n");
      printf("    A B C D E F G H\n");
    }

    static void printBbRank(BitBoardT bb, int rank) {
      printf("%d | ", rank+1);
      for(int file = 0; file < 8; file++) {
	SquareT square = (SquareT)((rank << 3) + file);
	printf("%c ", "-*"[(bb >> square) & 1]);
      }
      printf(" | %d\n", rank+1);
    }

    void printBb(BitBoardT bb) {
      printf("    A B C D E F G H\n");
      printf("    ---------------\n");
      for(int rank = 7; rank >= 0; rank--) { 
	printBbRank(bb, rank);
      }
      printf("    ---------------\n");
      printf("    A B C D E F G H\n");
    }
  }
}
