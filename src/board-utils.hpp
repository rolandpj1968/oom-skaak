#ifndef BOARD_UTILS_HPP
#define BOARD_UTILS_HPP

#include <cstdio>

#include <array>
#include <utility>
#include <vector>

#include "bits.hpp"
#include "board.hpp"
#include "move-gen.hpp"

namespace Chess {
  
  namespace BoardUtils {

    // // Note - MUST use this rather than zero-initialisation because piece squares need to be InvalidSquare
    // SimpleBoardT emptyBoard();
    
    // SimpleBoardT startingPosition();

    // template <typename BoardT>
    // bool isValid(const BoardT& board, const BitBoardT allYourKingAttackersBb);

    // // These are used for FEN output
    // char pieceChar(const std::vector<std::pair<ColorT, PieceTypeT>>& squarePieces);
    
    // template <typename BoardT>
    // std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64> genPieceMap(const BoardT& board);

    // template <typename BoardT>
    // void printBoard(const BoardT& board);
    
    // void printBb(BitBoardT bb);
    
    //using namespace MoveGen;

    // typedef SimpleBoardT BoardT;
    // typedef typename BoardT::ColorStateT ColorStateT;
    
    inline SimpleBoardT emptyBoard() {
      SimpleBoardT board = {};
      for(unsigned color = 0; color < NColors; color++) {
	SimpleColorStateImplT& colorState = board.state[color];
	for(int i = 0; i < NPieces; i++) {
	  colorState.pieceSquares[i] = InvalidSquare;
	}
	colorState.epSquare = InvalidSquare;
      }
      return board;
    }

    template <typename BoardT>
    inline void addPiece(BoardT& board, const ColorT color, const SquareT square, const PieceT piece) {
      typename BoardT::ColorStateT& c = board.state[(size_t)color];

      c.pieceSquares[piece] = square;
    }

    template <typename BoardT>
    inline void addPawn(BoardT& board, const ColorT color, const SquareT square) {
      typename BoardT::ColorStateT& c = board.state[(size_t)color];
      
      const BitBoardT pieceBb = bbForSquare(square);

      c.pawnsBb |= pieceBb;
    }
    
    template <typename BoardT>
    inline void addStartingPieces(BoardT& board, const ColorT color, const SquareT firstPieceSquare, const SquareT firstPawnSquare) {
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
      board.state[(size_t)color].castlingRights = (CastlingRightsT)(CanCastleQueenside | CanCastleKingside);
    }

    inline SimpleBoardT startingPosition() {
      SimpleBoardT board = emptyBoard();

      addStartingPieces(board, White, A1, A2);
      addStartingPieces(board, Black, A8, A7);
      
      return board;
    }

    inline void addPawnsForColor(std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64>& pieceMap, const ColorT color, BitBoardT pawnsBb) {
      while(pawnsBb) {
	const SquareT square = Bits::popLsb(pawnsBb);
	pieceMap[square].push_back(std::pair<ColorT, PieceTypeT>(color, Pawn));
      }
    }

    template <typename BoardT>
    inline void addPiecesForColor(std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64>& pieceMap, const ColorT color, const typename BoardT::ColorStateT& colorState) {
      addPawnsForColor(pieceMap, color, colorState.pawnsBb);

      for(PieceT piece = Knight1; piece < NPieces; piece = (PieceT)(piece+1)) {
	SquareT square = colorState.pieceSquares[piece];
	if(square != InvalidSquare) {
	  pieceMap[square].push_back(std::pair<ColorT, PieceTypeT>(color, PieceTypeForPiece[piece]));
	}
      }

      // Promo pieces - ugh the bit stuff operates on BitBoardT type
#ifdef USE_PROMOS
      BitBoardT activePromos = (BitBoardT)colorState.activePromos;
      while(activePromos) {
	const int promoIndex = Bits::popLsb(activePromos);
	const PromoPieceAndSquareT promoPieceAndSquare = colorState.promos[promoIndex];
	const PromoPieceT promoPiece = promoPieceOf(promoPieceAndSquare);
	const SquareT promoPieceSq = squareOf(promoPieceAndSquare);

	pieceMap[promoPieceSq].push_back(std::pair<ColorT, PieceTypeT>(color, PieceTypeForPromoPiece[promoPiece]));
      }
#endif //def USE_PROMOS
      
    }

    template <typename BoardT>
    std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64> genPieceMap(const BoardT& board) {
      std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64> pieceMap;

      addPiecesForColor<BoardT>(pieceMap, White, board.state[(size_t)White]);
      addPiecesForColor<BoardT>(pieceMap, Black, board.state[(size_t)Black]);

      return pieceMap;
    }

    // Validate board
    template <typename BoardT>
    bool isValid(const BoardT& board, const BitBoardT allYourKingAttackersBb) {
      std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64> pieceMap = genPieceMap<BoardT>(board);

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

    extern char PieceChar[NColors][NPieceTypes+1];

    inline char pieceChar(const std::vector<std::pair<ColorT, PieceTypeT>>& squarePieces) {
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
    
    inline void printRank(const std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64>& pieceMap, int rank) {
      printf("%d | ", rank+1);
      for(int file = 0; file < 8; file++) {
	SquareT square = (SquareT)((rank << 3) + file);
	printf("%c ", pieceChar(pieceMap[square]));
      }
      printf(" | %d\n", rank+1);
    }

    inline void printPieceClashes(const std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64>& pieceMap) {
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
    
    template <typename BoardT>
    void printBoard(const BoardT& board) {
      std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64> pieceMap = genPieceMap<BoardT>(board);
      
      printf("    A B C D E F G H\n");
      printf("    ---------------\n");
      for(int rank = 7; rank >= 0; rank--) { 
	printRank(pieceMap, rank);
      }
      printf("    ---------------\n");
      printf("    A B C D E F G H\n");

      printPieceClashes(pieceMap);
    }

    inline void printBbRank(BitBoardT bb, int rank) {
      printf("%d | ", rank+1);
      for(int file = 0; file < 8; file++) {
	SquareT square = (SquareT)((rank << 3) + file);
	printf("%c ", "-*"[(bb >> square) & 1]);
      }
      printf(" | %d\n", rank+1);
    }

    inline void printBb(BitBoardT bb) {
      printf("    A B C D E F G H\n");
      printf("    ---------------\n");
      for(int rank = 7; rank >= 0; rank--) { 
	printBbRank(bb, rank);
      }
      printf("    ---------------\n");
      printf("    A B C D E F G H\n");
    }
    
    // template <typename BoardT>
    // int countAttacks(const typename PieceAttackBbsImplType<BoardT>::PieceAttackBbsT& pieceAttackBbs, const BitBoardT filterOut = BbNone, const BitBoardT filterInPawnTakes = BbAll);

    // TODO - this shouldn't be in this header file but it has awkward dependencies
    template <typename BoardT, ColorT Color>
    bool isValid(const BoardT& board) {
      typedef typename BoardT::ColorStateT ColorStateT;

      typedef typename MoveGen::PieceBbsImplType<BoardT>::PieceBbsT PieceBbsT;
      typedef typename MoveGen::ColorPieceBbsImplType<BoardT>::ColorPieceBbsT ColorPieceBbsT;
      
      const ColorT OtherColor = OtherColorT<Color>::value;

      const ColorStateT& yourState = board.state[(size_t)OtherColor];
      
      const PieceBbsT& pieceBbs = MoveGen::genPieceBbs<BoardT, Color>(board);
      const ColorPieceBbsT& myPieceBbs = pieceBbs.colorPieceBbs[(size_t)Color];
      const ColorPieceBbsT& yourPieceBbs = pieceBbs.colorPieceBbs[(size_t)OtherColor];
      
      const BitBoardT allMyPiecesBb = myPieceBbs.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;
      
      const SquareT yourKingSq = yourState.pieceSquares[TheKing];
      const MoveGen::SquareAttackerBbsT yourKingAttackerBbs = MoveGen::genSquareAttackerBbs<BoardT, Color>(yourKingSq, myPieceBbs, allPiecesBb);
      const BitBoardT allYourKingAttackersBb = yourKingAttackerBbs.pieceAttackerBbs[AllPieceTypes];

      return isValid(board, allYourKingAttackersBb);
    }

    // TODO - this shouldn't be in this header file but it has awkward dependencies
    template <typename BoardT, ColorT Color>
    int getNChecks(const BoardT& board) {
      typedef typename BoardT::ColorStateT ColorStateT;
      
      typedef typename MoveGen::PieceBbsImplType<BoardT>::PieceBbsT PieceBbsT;
      typedef typename MoveGen::ColorPieceBbsImplType<BoardT>::ColorPieceBbsT ColorPieceBbsT;
      
      const ColorT OtherColor = OtherColorT<Color>::value;
      
      const ColorStateT& myState = board.state[(size_t)Color];

      const PieceBbsT pieceBbs = MoveGen::genPieceBbs<BoardT, Color>(board);

      const ColorPieceBbsT& myPieceBbs = pieceBbs.colorPieceBbs[(size_t)Color];
      const ColorPieceBbsT& yourPieceBbs = pieceBbs.colorPieceBbs[(size_t)OtherColor];
      
      const BitBoardT allMyPiecesBb = myPieceBbs.bbs[AllPieceTypes];
      const BitBoardT allYourPiecesBb = yourPieceBbs.bbs[AllPieceTypes];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;
      
      const SquareT myKingSq = myState.pieceSquares[TheKing];
      const MoveGen::SquareAttackerBbsT myKingAttackerBbs = MoveGen::genSquareAttackerBbs<BoardT, OtherColor>(myKingSq, yourPieceBbs, allPiecesBb);
      const BitBoardT allMyKingAttackersBb = myKingAttackerBbs.pieceAttackerBbs[AllPieceTypes];

      return Bits::count(allMyKingAttackersBb);
    }
    
    template <typename BoardT>
    static int countFilteredOut(const BitBoardT attacks, const BitBoardT filterOut) {
      printf("             %016lx filtered out %016lx -> %016lx count %d\n", attacks, filterOut, attacks & ~filterOut, Bits::count(attacks & ~filterOut));
      return Bits::count(attacks & ~filterOut);
    }

    template <typename BoardT>
    static int countFilteredIn(const BitBoardT attacks, const BitBoardT filterIn) {
      printf("             %016lx filtered in %016lx -> %016lx count %d\n", attacks, filterIn, attacks & filterIn, Bits::count(attacks & filterIn));
      return Bits::count(attacks & filterIn);
    }

    // Hrm, this ignores in-check
    template <typename BoardT>
    int countAttacks(const typename MoveGen::PieceAttackBbsImplType<BoardT>::PieceAttackBbsT& pieceAttacks, const BitBoardT filterOut = BbNone, const BitBoardT filterInPawnTakes = BbAll) {
      int nAttacks =
    	countFilteredIn<BoardT>(pieceAttacks.pawnsLeftAttacksBb, filterInPawnTakes) +
    	countFilteredIn<BoardT>(pieceAttacks.pawnsRightAttacksBb, filterInPawnTakes) +
    	countFilteredOut<BoardT>(pieceAttacks.pawnsPushOneBb, filterOut) +
    	countFilteredOut<BoardT>(pieceAttacks.pawnsPushTwoBb, filterOut);
      
      for(PieceT piece = Knight1; piece <= TheKing; piece = PieceT(piece + 1)) {
    	nAttacks += countFilteredOut<BoardT>(pieceAttacks.pieceAttackBbs[piece], filterOut);
      }

      return nAttacks;
    }
    
  } // namespace BoardUtils
  
} // namespace Chess

#endif //def BOARD_UTILS_HPP
