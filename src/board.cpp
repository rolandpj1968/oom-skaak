#include <cstdio>

#include <array>
#include <utility>
#include <vector>

using namespace std;

#include "bits.hpp"
#include "board.hpp"
#include "move-gen.hpp"

namespace Chess {
  
  namespace Board {

    using namespace MoveGen;

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

    static void addPawnsForColor(array<vector<pair<ColorT, PieceT>>, 64>& pieceMap, const ColorT color, BitBoardT pawnsBb) {
      while(pawnsBb) {
	const SquareT square = Bits::popLsb(pawnsBb);
	pieceMap[square].push_back(pair<ColorT, PieceT>(color, SomePawns));
      }
    }
    
    static void addPiecesForColor(array<vector<pair<ColorT, PieceT>>, 64>& pieceMap, const ColorT color, const ColorStateT& colorState) {
      addPawnsForColor(pieceMap, color, colorState.pawnsBb);

      for(PieceT piece = QueenKnight; piece < NPieces; piece = (PieceT)(piece+1)) {
	SquareT square = colorState.pieceSquares[piece];
	if(square != InvalidSquare) {
	  pieceMap[square].push_back(pair<ColorT, PieceT>(color, piece));
	}
      }
    }

    static array<vector<pair<ColorT, PieceT>>, 64> genPieceMap(const BoardT& board) {
      array<vector<pair<ColorT, PieceT>>, 64> pieceMap;

      addPiecesForColor(pieceMap, White, board.pieces[(size_t)White]);
      addPiecesForColor(pieceMap, Black, board.pieces[(size_t)Black]);

      return pieceMap;
    }

    // Validate board
    template <typename BoardTraitsT>
    bool isValid(const BoardT& board) {
      array<vector<pair<ColorT, PieceT>>, 64> pieceMap = genPieceMap(board);

      // Are there any squares with multiple pieces on them?
      for(int i = 0; i < 64; i++) {
	if(pieceMap[i].size() > 1) {
	  return false;
	}
      }

      // Is the other king in check?
      typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;

      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
      
      const PieceBbsT& pieceBbs = genPieceBbs<BoardTraitsT>(board);
      const ColorPieceBbsT& myPieceBbs = pieceBbs.colorPieceBbs[(size_t)Color];
      const ColorPieceBbsT& yourPieceBbs = pieceBbs.colorPieceBbs[(size_t)OtherColor];
      
      const BitBoardT allMyPiecesBb = myPieceBbs.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;
      
      const SquareT yourKingSq = yourState.pieceSquares[TheKing];
      const SquareAttackersT yourKingAttackers = genSquareAttackers<YourColorTraitsT>(yourKingSq, myPieceBbs, allPiecesBb);
      const BitBoardT allYourKingAttackersBb = yourKingAttackers.pieceAttackers[AllPieceTypes];

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

    static char pieceChar(const vector<pair<ColorT, PieceT>>& squarePieces) {
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
    
    static void printRank(const array<vector<pair<ColorT, PieceT>>, 64>& pieceMap, int rank) {
      printf("%d | ", rank+1);
      for(int file = 0; file < 8; file++) {
	SquareT square = (SquareT)((rank << 3) + file);
	//SquarePieceT squarePiece = NoPiece; //board.board[square];
	// ColorT color = squarePieceColor(squarePiece);
	// PieceT piece = squarePiecePiece(squarePiece);
	//PieceTypeT pieceType = PieceTypeForPiece[piece];
	printf("%c ", pieceChar(pieceMap[square]));
      }
      printf(" | %d\n", rank+1);
    }

    void printBoard(const BoardT& board) {
      array<vector<pair<ColorT, PieceT>>, 64> pieceMap = genPieceMap(board);
      
      printf("    A B C D E F G H\n");
      printf("    ---------------\n");
      for(int rank = 7; rank >= 0; rank--) { 
	printRank(pieceMap, rank);
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
