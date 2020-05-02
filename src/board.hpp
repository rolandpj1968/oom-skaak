#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include <utility>
#include <vector>

#include "bits.hpp"
#include "types.hpp"

#undef USE_PROMOS

namespace Chess {

  namespace Board {

    struct SimpleColorStateImplT {
      // Pawns bitboard
      BitBoardT pawnsBb;

      // All pieces except pawns and strange promos.
      // MUST be InvalidSquare if a piece is not present - see emptyBoard()
      SquareT pieceSquares[NPieces];

      // InvalidSquare, or else en-passant square of the last move - i.e. the square behind a pawn two-square push.
      SquareT epSquare;

      // Castling rights
      CastlingRightsT castlingRights;
    };

    struct SimpleColorStateWithPromosImplT : SimpleColorStateImplT {
      // Bitmap of active promo pieces - index into promos array
      u8 activePromos;

      // Piece type and square of active promos - valid at indexes where activePromos bitmap is 1
      PromoPieceAndSquareT promos[NPawns];
    };

    // Don't use this directly with zero-initialisation or you'll be disappointed because some fields need InvalidSquare (!= 0) init.
    // Use emptyBoard() or startingPosition() or parseFen().
    template <typename ColorStateImplT>
    struct SimpleBoardImplT {
      typedef ColorStateImplT ColorStateT;
      
      ColorStateImplT state[NColors];
    };

    typedef SimpleBoardImplT<SimpleColorStateImplT> SimpleBoardT;
    typedef SimpleBoardImplT<SimpleColorStateWithPromosImplT> SimpleBoardWithPromosT;

    template <typename BoardT>
    struct BoardType {};

    template <> struct BoardType<SimpleBoardT> {
      typedef SimpleBoardWithPromosT WithPromosT;
      typedef SimpleBoardT WithoutPromosT;
    };

    template <> struct BoardType<SimpleBoardWithPromosT> {
      typedef SimpleBoardWithPromosT WithPromosT;
      typedef SimpleBoardT WithoutPromosT;
    };

    template <typename BoardOutputT, typename BoardInputT>
    inline BoardOutputT copyBoard(const BoardInputT& board);

    template <>
    inline SimpleBoardWithPromosT copyBoard<SimpleBoardWithPromosT, SimpleBoardT>(const SimpleBoardT& board) {
      SimpleBoardWithPromosT newBoard;
      // TODO - implement me
      return newBoard;
    }
    
    const bool DoesNotHavePromos = false;
    const bool DoesHavePromos = true;
    
    union PieceOrPromoIndexT {
      PieceT piece;
      u8 promoIndex;
    };
    
    // Piece map container
    // Maps a square to either a PieceT or a promo piece index, depending on the corresponding bit in allPromoPiecesBb
    struct ColorPieceMapT {
      BitBoardT allPromoPiecesBb;
      PieceOrPromoIndexT board[64+1]; // Allow board[InvalidSquare]
    };
    
    // Generate the (non-pawn) piece map for a color
    template <typename ColorStateT>
    inline ColorPieceMapT genColorPieceMap(const ColorStateT& colorState, const BitBoardT allPromoPiecesBb) {
      ColorPieceMapT pieceMap = {};

      pieceMap.board[colorState.pieceSquares[Knight1]].piece = Knight1;
      pieceMap.board[colorState.pieceSquares[Knight2]].piece = Knight2;
      pieceMap.board[colorState.pieceSquares[Bishop1]].piece = Bishop1;
      pieceMap.board[colorState.pieceSquares[Bishop2]].piece = Bishop2;
      pieceMap.board[colorState.pieceSquares[Rook1]].piece = Rook1;
      pieceMap.board[colorState.pieceSquares[Rook2]].piece = Rook2;
      pieceMap.board[colorState.pieceSquares[TheQueen]].piece = TheQueen;
      pieceMap.board[colorState.pieceSquares[TheKing]].piece = TheKing;

      // Promo pieces - ugh the bit stuff operates on BitBoardT type
#ifdef USE_PROMOS
      BitBoardT activePromos = (BitBoardT)colorState.activePromos;
      while(activePromos) {
	const int promoIndex = Bits::popLsb(activePromos);
	const PromoPieceAndSquareT promoPieceAndSquare = colorState.promos[promoIndex];
	const SquareT promoPieceSq = squareOf(promoPieceAndSquare);

	pieceMap.board[promoPieceSq].promoIndex = promoIndex;
      }

      pieceMap.allPromoPiecesBb = allPromoPiecesBb;
#endif //def USE_PROMOS
      
      return pieceMap;
    }

    template <typename BoardT, ColorT Color>
    inline PieceT removePiece(BoardT& board, const SquareT square, const PieceT piece) {
      typename BoardT::ColorStateT &colorState = board.state[(size_t)Color];

      colorState.pieceSquares[piece] = InvalidSquare;

      colorState.castlingRights = (CastlingRightsT) (colorState.castlingRights & ~CastlingRightsForPiece[piece]);
      
      return piece;
    }
    
    template <typename BoardT, ColorT Color>
    inline void removePawn(BoardT& board, const SquareT square) {
      typename BoardT::ColorStateT &colorState = board.state[(size_t)Color];

      const BitBoardT squareBb = bbForSquare(square);

      colorState.pawnsBb &= ~squareBb;
    }

#ifdef USE_PROMOS
    template <typename BoardT, ColorT Color>
    inline void removePromoPiece(BoardT& board, const int promoIndex) {
      typename BoardT::ColorStateT &colorState = board.state[(size_t)Color];

      colorState.activePromos &= ~((u8)1 << promoIndex);
    }
#endif //def USE_PROMOS

    template <typename BoardT, ColorT Color>
    inline PieceT removePiece(BoardT& board, const ColorPieceMapT& pieceMap, const SquareT square) {
      const PieceT piece = pieceMap.board[square].piece;

      return removePiece<Color>(board, square, piece);
    }

    template <typename BoardT, ColorT Color>
    inline void removePromoPiece(BoardT& board, const ColorPieceMapT& pieceMap, const SquareT square) {
      const int promoIndex = pieceMap.board[square].promoIndex;

      removePromoPiece<BoardT, Color>(board, promoIndex);
    }

    template <typename BoardT, ColorT Color>
    inline void removePieceOrPawn(BoardT& board, const ColorPieceMapT& pieceMap, const SquareT square) {
      typename BoardT::ColorStateT &pieces = board.state[(size_t)Color];
      const BitBoardT yourPawnsBb = pieces.pawnsBb;

      // Remove pawn (or no-op if it's a piece)
      const BitBoardT squareBb = bbForSquare(square);
      const BitBoardT pawnBb = yourPawnsBb & squareBb; // BbNone for (non-pawn) pieces
      pieces.pawnsBb &= ~pawnBb;

      // Remove piece (or no-op it it's a pawn)
      const PieceT piece = pieceMap.board[square].piece; // NoPiece for pawns
      pieces.pieceSquares[piece] = InvalidSquare;

      pieces.castlingRights = (CastlingRightsT) (pieces.castlingRights & ~CastlingRightsForPiece[piece]);
    }

    template <typename BoardT, ColorT Color, PieceT Piece>
    inline void removePiece(BoardT& board, const SquareT square) {
      removePiece<BoardT, Color>(board, square, Piece);
    }

    template <typename BoardT>
    inline void placePiece(BoardT& board, const ColorT color, const SquareT square, const PieceT piece) {
      typename BoardT::ColorStateT &colorState = board.state[(size_t)color];

      colorState.pieceSquares[piece] = square;
    }

// #ifdef USE_PROMOS
    template <typename BoardT>
    inline void addPromoPiece(BoardT& board, const ColorT color, const int promoIndex, const PromoPieceT promoPiece, const SquareT square) {
      typename BoardT::ColorStateT &colorState = board.state[(size_t)color];

      colorState.activePromos |= ((u8)1 << promoIndex);
      colorState.promos[promoIndex] = promoPieceAndSquareOf(promoPiece, square);
    }
    
    template <typename BoardT>
    inline void movePromoPiece(BoardT& board, const ColorT color, const int promoIndex, const PromoPieceT promoPiece, const SquareT square) {
      typename BoardT::ColorStateT &colorState = board.state[(size_t)color];

      colorState.promos[promoIndex] = promoPieceAndSquareOf(promoPiece, square);
    }
// #endif //def USE_PROMOS
    
    template <typename BoardT, ColorT Color>
    inline void placePiece(BoardT& board, const SquareT square, const PieceT piece) {
      placePiece<BoardT>(board, Color, square, piece);
    }

    template <typename BoardT>
    inline void placePawn(BoardT& board, const ColorT color, const SquareT square) {
      typename BoardT::ColorStateT &pieces = board.state[(size_t)color];

      const BitBoardT squareBb = bbForSquare(square);

      pieces.pawnsBb |= squareBb;
    }

    template <typename BoardT, ColorT Color>
    inline void placePawn(BoardT& board, const SquareT square) {
      placePawn<BoardT>(board, Color, square);
    }

    template <typename BoardT, ColorT Color, PieceT Piece>
    inline void placePiece(BoardT& board, const SquareT square) {
      placePiece<BoardT, Color>(board, square, Piece);
    }
    
    template <typename BoardT, ColorT Color, PieceT Piece>
    inline BoardT pushPiece(const BoardT& oldBoard, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePiece<BoardT, Color, Piece>(board, from);

      placePiece<BoardT, Color, Piece>(board, to);

      // Clear en-passant square
      board.state[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }

#ifdef USE_PROMOS
    template <typename BoardT, ColorT Color>
    inline BoardT pushPromoPiece(const BoardT& oldBoard, const int promoIndex, const PromoPieceT promoPiece, const SquareT to) {
      BoardT board = oldBoard;

      movePromoPiece<BoardT>(board, Color, promoIndex, promoPiece, to);

      // Clear en-passant square
      board.state[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
#endif // USE_PROMOS
    
    template <typename BoardT, ColorT Color, PieceT Piece>
    inline BoardT captureWithPiece(const BoardT& oldBoard, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePieceOrPawn<BoardT, OtherColorT<Color>::value>(board, yourPieceMap, to);

      removePiece<BoardT, Color, Piece>(board, from);

      placePiece<BoardT, Color, Piece>(board, to);

      // Clear en-passant square
      board.state[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }

#ifdef USE_PROMOS
    template <typename BoardT, ColorT Color, PieceT Piece>
    inline BoardT capturePromoPieceWithPiece(const BoardT& oldBoard, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePromoPiece<BoardT, OtherColorT<Color>::value>(board, yourPieceMap, to);
      
      removePiece<BoardT, Color, Piece>(board, from);

      placePiece<BoardT, Color, Piece>(board, to);

      // Clear en-passant square
      board.state[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <typename BoardT, ColorT Color>
    inline BoardT captureWithPromoPiece(const BoardT& oldBoard, const int promoIndex, const PromoPieceT promoPiece, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePieceOrPawn<BoardT, OtherColorT<Color>::value>(board, yourPieceMap, to);

      movePromoPiece<BoardT>(board, Color, promoIndex, promoPiece, to);

      // Clear en-passant square
      board.state[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <typename BoardT, ColorT Color>
    inline BoardT capturePromoPieceWithPromoPiece(const BoardT& oldBoard, const int promoIndex, const PromoPieceT promoPiece, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePromoPiece<BoardT, OtherColorT<Color>::value>(board, yourPieceMap, to);

      movePromoPiece<BoardT>(board, Color, promoIndex, promoPiece, to);

      // Clear en-passant square
      board.state[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
#endif //def USE_PROMOS
    
    template <typename BoardT, ColorT Color>
    inline BoardT captureWithPawn(const BoardT& oldBoard, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePieceOrPawn<BoardT, OtherColorT<Color>::value>(board, yourPieceMap, to);

      removePawn<BoardT, Color>(board, from);

      placePawn<BoardT, Color>(board, to);

      // Clear en-passant square
      board.state[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }

#ifdef USE_PROMOS
    template <typename BoardT, ColorT Color>
    inline BoardT captureWithPawnToPromo(const BoardT& oldBoard, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
      BoardT board = oldBoard;

      removePieceOrPawn<BoardT, OtherColorT<Color>::value>(board, yourPieceMap, to);

      removePawn<BoardT, Color>(board, from);

      const int promoIndex = Bits::lsb(~board.state[(size_t)Color].activePromos);
      addPromoPiece<BoardT>(board, Color, promoIndex, promoPiece, to);

      // Clear en-passant square
      board.state[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <typename BoardT, ColorT Color>
    inline BoardT capturePromoPieceWithPawn(const BoardT& oldBoard, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePromoPiece<BoardT, OtherColorT<Color>::value>(board, yourPieceMap, to);

      removePawn<BoardT, Color>(board, from);

      placePawn<BoardT, Color>(board, to);
      
      // Clear en-passant square
      board.state[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <typename BoardT, ColorT Color>
    inline BoardT capturePromoPieceWithPawnToPromo(const BoardT& oldBoard, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
      BoardT board = oldBoard;

      removePromoPiece<BoardT, OtherColorT<Color>::value>(board, yourPieceMap, to);

      removePawn<BoardT, Color>(board, from);

      const int promoIndex = Bits::lsb(~board.state[(size_t)Color].activePromos);
      addPromoPiece<BoardT>(board, Color, promoIndex, promoPiece, to);
      
      // Clear en-passant square
      board.state[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
#endif //def USE_PROMOS

    template <typename BoardT, ColorT Color, bool IsPawnPushTwo = false>
    inline BoardT pushPawn(const BoardT& oldBoard, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePawn<BoardT, Color>(board, from);

      placePawn<BoardT, Color>(board, to);

      // Set en-passant square
      if(IsPawnPushTwo) {
      	board.state[(size_t)Color].epSquare = (SquareT)((from+to)/2);
      } else {
      	board.state[(size_t)Color].epSquare = InvalidSquare;
      }	
      
      return board;
    }

// #ifdef USE_PROMOS
    template <typename BoardT, ColorT Color>
    inline BoardT pushPawnToPromo(const BoardT& oldBoard, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
      BoardT board = oldBoard;

      removePawn<BoardT, Color>(board, from);

      const int promoIndex = Bits::lsb(~board.state[(size_t)Color].activePromos);
      addPromoPiece<BoardT>(board, Color, promoIndex, promoPiece, to);

      // Clear en-passant square
      board.state[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
// #endif //def USE_PROMOS
    
    template <typename BoardT, ColorT Color>
    inline BoardT captureEp(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      BoardT board = oldBoard;

      removePawn<BoardT, OtherColorT<Color>::value>(board, captureSquare);

      removePawn<BoardT, Color>(board, from);

      placePawn<BoardT, Color>(board, to);

      // Clear en-passant square
      board.state[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }

  }
    
}

#endif //ndef BOARD_HPP
