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

    template <typename BoardT>
    inline void addPiece(BoardT& board, const ColorT color, const SquareT square, const PieceT piece) {
      typename BoardT::ColorStateT& colorState = board.state[(size_t)color];
      NonPromosColorStateImplT& basicState = colorState.basic;

      basicState.pieceSquares[piece] = square;
    }

    template <typename BoardT>
    inline void addPawn(BoardT& board, const ColorT color, const SquareT square) {
      typename BoardT::ColorStateT& colorState = board.state[(size_t)color];
      NonPromosColorStateImplT& basicState = colorState.basic;
      
      const BitBoardT pieceBb = bbForSquare(square);

      basicState.pawnsBb |= pieceBb;
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
      board.state[(size_t)color].basic.castlingRights = (CastlingRightsT)(CanCastleQueenside | CanCastleKingside);
    }

    inline void addPieceToMap(std::pair<ColorT, PieceTypeT> pieceMap[64], const SquareT square, const std::pair<ColorT, PieceTypeT>& colorAndPiece) {
	pieceMap[square] = colorAndPiece;
    }

    inline void addPieceToMap(std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64>& pieceMap, const SquareT square, const std::pair<ColorT, PieceTypeT>& colorAndPiece) {
	pieceMap[square].push_back(colorAndPiece);
    }

    template <typename PieceMapT>
    inline void addPawnsForColor(PieceMapT & pieceMap, const ColorT color, BitBoardT pawnsBb) {
      while(pawnsBb) {
	const SquareT square = Bits::popLsb(pawnsBb);
	addPieceToMap(pieceMap, square, std::pair<ColorT, PieceTypeT>(color, Pawn));
      }
    }


    template <typename PieceMapT>
    inline void addPromoPiecesForColor(PieceMapT& pieceMap, const ColorT color, const typename BasicBoardT::ColorStateT& colorState) {
      // No promo pieces
    }
    
    template <typename PieceMapT>
    inline void addPromoPiecesForColor(PieceMapT& pieceMap, const ColorT color, const typename FullBoardT::ColorStateT& colorState) {
      // Ugh the bit stuff operates on BitBoardT type
      BitBoardT activePromos = (BitBoardT)colorState.promos.activePromos;
      while(activePromos) {
	const int promoIndex = Bits::popLsb(activePromos);
	const PromoPieceAndSquareT promoPieceAndSquare = colorState.promos.promos[promoIndex];
	const PromoPieceT promoPiece = promoPieceOf(promoPieceAndSquare);
	const SquareT promoPieceSq = squareOf(promoPieceAndSquare);

	addPieceToMap(pieceMap, promoPieceSq, std::pair<ColorT, PieceTypeT>(color, PieceTypeForPromoPiece[promoPiece]));
      }
    }
    
    template <typename BoardT, typename PieceMapT>
    inline void addPiecesForColor(PieceMapT& pieceMap, const ColorT color, const typename BoardT::ColorStateT& colorState) {
      const NonPromosColorStateImplT& basicState = colorState.basic;

      addPawnsForColor(pieceMap, color, basicState.pawnsBb);

      for(PieceT piece = Knight1; piece < NPieces; piece = (PieceT)(piece+1)) {
	SquareT square = basicState.pieceSquares[piece];
	if(square != InvalidSquare) {
	  addPieceToMap(pieceMap, square, std::pair<ColorT, PieceTypeT>(color, PieceTypeForPiece[piece]));
	}
      }

      // Promo pieces
      addPromoPiecesForColor(pieceMap, color, colorState);
    }

    template <typename BoardT, typename PieceMapT>
    inline void addPiecesToMap(PieceMapT& pieceMap, const BoardT& board) {
      addPiecesForColor<BoardT, PieceMapT>(pieceMap, White, board.state[(size_t)White]);
      addPiecesForColor<BoardT, PieceMapT>(pieceMap, Black, board.state[(size_t)Black]);
    }

    template <typename BoardT>
    std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64> genPieceMap(const BoardT& board) {
      typedef std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64> PieceMapT;
      PieceMapT pieceMap;

      addPiecesToMap(pieceMap, board);

      return pieceMap;
    }

    // Validate board
    template <typename BoardT>
    bool isValid(const BoardT& board, const BitBoardT allYourKingAttackersBb) {
      auto pieceMap = genPieceMap<BoardT>(board);

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
      
    inline char pieceChar(std::pair<ColorT, PieceTypeT> squarePiece) {
      return PieceChar[(size_t)squarePiece.first][squarePiece.second];
    }
    
    extern char pieceChar(const std::vector<std::pair<ColorT, PieceTypeT>>& squarePieces);
    
    extern void printRank(const std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64>& pieceMap, int rank);

    extern void printPieceClashes(const std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64>& pieceMap);
    
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
      
      const SquareT yourKingSq = yourState.basic.pieceSquares[TheKing];
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
      
      const SquareT myKingSq = myState.basic.pieceSquares[TheKing];
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

    extern void printBb(BitBoardT bb);
    
    extern BasicBoardT emptyBoard();
    extern BasicBoardT startingPosition();
    
  } // namespace BoardUtils
  
} // namespace Chess

#endif //def BOARD_UTILS_HPP
