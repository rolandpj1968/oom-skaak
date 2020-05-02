#include <array>
#include <vector>

#include "board.hpp"
#include "board-utils.hpp"
#include "types.hpp"

namespace Chess {
  
  namespace BoardUtils {

    using namespace Board;

     void addPawnsForColor(std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64>& pieceMap, const ColorT color, BitBoardT pawnsBb) {
      while(pawnsBb) {
	const SquareT square = Bits::popLsb(pawnsBb);
	pieceMap[square].push_back(std::pair<ColorT, PieceTypeT>(color, Pawn));
      }
    }

    static char PieceChar[NColors][NPieceTypes+1] = {
      // White
      { ".PNBRQK" },
      // Black
      { ".pnbrqk" }
    };

    char pieceChar(const std::vector<std::pair<ColorT, PieceTypeT>>& squarePieces) {
      // Pieces clash on the square?
      if(squarePieces.size() > 1) {
	return 'X';
      }
      
      ColorT color = White;
      PieceTypeT pieceType = NoPieceType;

      if(squarePieces.size() == 1) {
	color = squarePieces[0].first;
	pieceType = squarePieces[0].second;
      }
      
      return PieceChar[(size_t)color][pieceType];
    }
    
    void printRank(const std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64>& pieceMap, int rank) {
      printf("%d | ", rank+1);
      for(int file = 0; file < 8; file++) {
	SquareT square = (SquareT)((rank << 3) + file);
	printf("%c ", pieceChar(pieceMap[square]));
      }
      printf(" | %d\n", rank+1);
    }

    void printPieceClashes(const std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64>& pieceMap) {
      for(int i = 0; i < 64; i++) {
	const std::vector<std::pair<ColorT, PieceTypeT>>& squarePieceMap = pieceMap[i];
	if(squarePieceMap.size() > 1) {
	  printf("\nPiece clash on %s:", SquareStr[i]);
	  for(unsigned j = 0; j < squarePieceMap.size(); j++) {
	    ColorT color = squarePieceMap[j].first;
	    PieceTypeT pieceType = squarePieceMap[j].second;
	    printf(" %c", PieceChar[(size_t)color][pieceType]);
	  }
	  printf("\n");
	}
      }
    }
    
    void printBbRank(BitBoardT bb, int rank) {
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
    
    BasicBoardT emptyBoard() {
      BasicBoardT board = {};
      for(unsigned color = 0; color < NColors; color++) {
	BasicColorStateImplT& colorState = board.state[color];
	NonPromosColorStateImplT& basicState = colorState.basic;
	for(int i = 0; i < NPieces; i++) {
	  basicState.pieceSquares[i] = InvalidSquare;
	}
	basicState.epSquare = InvalidSquare;
      }
      return board;
    }

    BasicBoardT startingPosition() {
      BasicBoardT board = emptyBoard();

      addStartingPieces(board, White, A1, A2);
      addStartingPieces(board, Black, A8, A7);
      
      return board;
    }

  } // namespace BoardUtils
  
} // namespace Chess
