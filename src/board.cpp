#include <cstdio>

#include <array>
#include <utility>
#include <vector>

#include "bits.hpp"
#include "board.hpp"
#include "move-gen.hpp"

namespace Chess {
  
  namespace Board {

    using namespace MoveGen;

    BoardT emptyBoard() {
      BoardT board = {};
      for(unsigned color = 0; color < NColors; color++) {
	ColorStateT& colorState = board.pieces[color];
	for(int i = 0; i < NPieces; i++) {
	  colorState.pieceSquares[i] = InvalidSquare;
	}
	colorState.epSquare = InvalidSquare;
      }
      return board;
    }

    // TODO - unusual promos
    static void addPiece(BoardT& board, const ColorT color, const SquareT square, const PieceT piece) {
      ColorStateT& c = board.pieces[(size_t)color];

      c.pieceSquares[piece] = square;
    }

    static void addPawn(BoardT& board, const ColorT color, const SquareT square) {
      ColorStateT& c = board.pieces[(size_t)color];
      
      const BitBoardT pieceBb = bbForSquare(square);

      c.pawnsBb |= pieceBb;
    }
    
    static void addStartingPieces(BoardT& board, const ColorT color, const SquareT firstPieceSquare, const SquareT firstPawnSquare) {
      // Pieces
      addPiece(board, color, firstPieceSquare,       Rook1);
      addPiece(board, color, firstPieceSquare+B1-A1, Knight1);
      addPiece(board, color, firstPieceSquare+C1-A1, Bishop1);
      addPiece(board, color, firstPieceSquare+D1-A1, TheQueen);
      addPiece(board, color, firstPieceSquare+E1-A1, TheKing);
      addPiece(board, color, firstPieceSquare+F1-A1, Bishop2);
      addPiece(board, color, firstPieceSquare+G1-A1, Knight2);
      addPiece(board, color, firstPieceSquare+H1-A1, Rook2);
      
      // Pawns
      for(SquareT square = firstPawnSquare; square <= firstPawnSquare+H2-A2; square += (B2-A2)) {
	addPawn(board, color, square);
      }

      // Castling rights
      board.pieces[(size_t)color].castlingRights = (CastlingRightsT)(CanCastleQueenside | CanCastleKingside);
    }

    BoardT startingPosition() {
      BoardT board = emptyBoard();

      addStartingPieces(board, White, A1, A2);
      addStartingPieces(board, Black, A8, A7);
      
      return board;
    }

    static void addPawnsForColor(std::array<std::vector<std::pair<ColorT, PieceT>>, 64>& pieceMap, const ColorT color, BitBoardT pawnsBb) {
      while(pawnsBb) {
	const SquareT square = Bits::popLsb(pawnsBb);
	pieceMap[square].push_back(std::pair<ColorT, PieceT>(color, SomePawns));
      }
    }
    
    static void addPiecesForColor(std::array<std::vector<std::pair<ColorT, PieceT>>, 64>& pieceMap, const ColorT color, const ColorStateT& colorState) {
      addPawnsForColor(pieceMap, color, colorState.pawnsBb);

      for(PieceT piece = Knight1; piece < NPieces; piece = (PieceT)(piece+1)) {
	SquareT square = colorState.pieceSquares[piece];
	if(square != InvalidSquare) {
	  pieceMap[square].push_back(std::pair<ColorT, PieceT>(color, piece));
	}
      }

      // Promo pieces - ugh the bit stuff operates on BitBoardT type
      BitBoardT activePromos = (BitBoardT)colorState.activePromos;
      while(activePromos) {
	const int promoIndex = Bits::popLsb(activePromos);
	const PromoPieceAndSquareT promoPieceAndSquare = colorState.promos[promoIndex];
	const PromoPieceT promoPiece = promoPieceOf(promoPieceAndSquare);
	const SquareT promoPieceSq = squareOf(promoPieceAndSquare);

	//pieceMap[promoPieceSq].push_back(std::pair<ColorT, PieceT>(color, piece)); // Grrr we want PieceTypeT in the map, not piece
      }
      
    }

    std::array<std::vector<std::pair<ColorT, PieceT>>, 64> genPieceMap(const BoardT& board) {
      std::array<std::vector<std::pair<ColorT, PieceT>>, 64> pieceMap;

      addPiecesForColor(pieceMap, White, board.pieces[(size_t)White]);
      addPiecesForColor(pieceMap, Black, board.pieces[(size_t)Black]);

      return pieceMap;
    }

    // Validate board
    bool isValid(const BoardT& board, const BitBoardT allYourKingAttackersBb) {
      std::array<std::vector<std::pair<ColorT, PieceT>>, 64> pieceMap = genPieceMap(board);

      // Are there any squares with multiple pieces on them?
      for(int i = 0; i < 64; i++) {
	if(pieceMap[i].size() > 1) {
	  return false;
	}
      }

      // Is the other king in check?
      if(allYourKingAttackersBb != BbNone) {
	return false;
      }

      // TODO castling rights and en-passant square validation

      return true;
    }

    static char PieceChar[NColors][NPieceTypes+1] = {
      // White
      { ".PNBRQK" },
      // Black
      { ".pnbrqk" }
    };

    char pieceChar(const std::vector<std::pair<ColorT, PieceT>>& squarePieces) {
      // Pieces clash on the square?
      if(squarePieces.size() > 1) {
	return 'X';
      }
      
      ColorT color = White;
      PieceTypeT pieceType = NoPieceType;

      if(squarePieces.size() == 1) {
	color = squarePieces[0].first;
	PieceT piece = squarePieces[0].second;
	pieceType = PieceTypeForPiece[piece];
      }
      
      return PieceChar[(size_t)color][pieceType];
    }
    
    static void printRank(const std::array<std::vector<std::pair<ColorT, PieceT>>, 64>& pieceMap, int rank) {
      printf("%d | ", rank+1);
      for(int file = 0; file < 8; file++) {
	SquareT square = (SquareT)((rank << 3) + file);
	printf("%c ", pieceChar(pieceMap[square]));
      }
      printf(" | %d\n", rank+1);
    }

    void printPieceClashes(const std::array<std::vector<std::pair<ColorT, PieceT>>, 64>& pieceMap) {
      for(int i = 0; i < 64; i++) {
	const std::vector<std::pair<ColorT, PieceT>>& squarePieceMap = pieceMap[i];
	if(squarePieceMap.size() > 1) {
	  printf("\nPiece clash on %s:", SquareStr[i]);
	  for(unsigned j = 0; j < squarePieceMap.size(); j++) {
	    ColorT color = squarePieceMap[j].first;
	    PieceT piece = squarePieceMap[j].second;
	    PieceTypeT pieceType = PieceTypeForPiece[piece];
	    printf(" %c", PieceChar[(size_t)color][pieceType]);
	  }
	  printf("\n");
	}
      }
    }
    
    void printBoard(const BoardT& board) {
      std::array<std::vector<std::pair<ColorT, PieceT>>, 64> pieceMap = genPieceMap(board);
      
      printf("    A B C D E F G H\n");
      printf("    ---------------\n");
      for(int rank = 7; rank >= 0; rank--) { 
	printRank(pieceMap, rank);
      }
      printf("    ---------------\n");
      printf("    A B C D E F G H\n");

      printPieceClashes(pieceMap);
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
