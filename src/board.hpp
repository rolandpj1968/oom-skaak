#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include <utility>
#include <vector>

#include "bits.hpp"
#include "types.hpp"

namespace Chess {
  
  namespace Board {

    struct ColorStateT {
      // Pawns bitboard
      BitBoardT pawnsBb;

      // All pieces except pawns and strange promos.
      // MUST be InvalidSquare if a piece is not present - see emptyBoard()
      SquareT pieceSquares[NPieces];

      // Bitmap of active promo pieces - index into promos array
      u8 activePromos;

      // Piece type and square of active promos - valid at indexes where activePromos bitmap is 1
      PromoPieceAndSquareT promos[NPawns];

      // InvalidSquare, or else en-passant square of the last move - i.e. the square behind a pawn two-square push.
      SquareT epSquare;

      // Castling rights
      CastlingRightsT castlingRights;
    };

    // Don't use this directly with zero-initialisation or you'll be disappointed because some fields need InvalidSquare (!= 0) init.
    // Use emptyBoard() or startingPosition() or parseFen().
    struct BoardT {
      ColorStateT pieces[NColors];
    };

    const bool DoesNotHavePromos = false;
    const bool DoesHavePromos = true;
    
    // Board traits used for optimising move generation etc.
    // The compile-time traits can be false only if the run-time state is also false.
    //   On the other hand the code should handle compile-time traits being true even if run-time traits are false.
    template <ColorT ColorVal, bool HasPromosVal>
    struct ColorTraitsImplT {
      static const ColorT Color = ColorVal;
      static const bool HasPromos = HasPromosVal;

      typedef ColorTraitsImplT<Color, DoesHavePromos> WithPromosT;
      typedef ColorTraitsImplT<Color, DoesNotHavePromos> WithoutPromosT;
    };

    typedef ColorTraitsImplT<White, DoesNotHavePromos> WhiteStartingColorTraitsT;
    typedef ColorTraitsImplT<Black, DoesNotHavePromos> BlackStartingColorTraitsT;

    template <typename MyColorTraitsImplT, typename YourColorTraitsImplT>
    struct BoardTraitsImplT {
      typedef MyColorTraitsImplT MyColorTraitsT;
      typedef YourColorTraitsImplT YourColorTraitsT;

      static const ColorT Color = MyColorTraitsT::Color;
      static const ColorT OtherColor = YourColorTraitsT::Color;

      // Switch color to move
      typedef BoardTraitsImplT<YourColorTraitsT, MyColorTraitsT> ReverseT;

      // We have some promo pieces.
      typedef BoardTraitsImplT<typename MyColorTraitsT::WithPromosT, YourColorTraitsT> WithPromosT;
      // We have no promo pieces.
      typedef BoardTraitsImplT<typename MyColorTraitsT::WithoutPromosT, YourColorTraitsT> WithoutPromosT;
    };

    typedef BoardTraitsImplT<WhiteStartingColorTraitsT, BlackStartingColorTraitsT> StartingBoardTraitsT;

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
      BitBoardT activePromos = (BitBoardT)colorState.activePromos;
      while(activePromos) {
	const int promoIndex = Bits::popLsb(activePromos);
	const PromoPieceAndSquareT promoPieceAndSquare = colorState.promos[promoIndex];
	const SquareT promoPieceSq = squareOf(promoPieceAndSquare);

	pieceMap.board[promoPieceSq].promoIndex = promoIndex;
      }

      pieceMap.allPromoPiecesBb = allPromoPiecesBb;
      
      return pieceMap;
    }

    template <ColorT Color>
    inline PieceT removePiece(BoardT& board, const SquareT square, const PieceT piece) {
      ColorStateT &colorState = board.pieces[(size_t)Color];

      colorState.pieceSquares[piece] = InvalidSquare;

      colorState.castlingRights = (CastlingRightsT) (colorState.castlingRights & ~CastlingRightsForPiece[piece]);
      
      return piece;
    }
    
    template <ColorT Color>
    inline void removePawn(BoardT& board, const SquareT square) {
      ColorStateT &colorState = board.pieces[(size_t)Color];

      const BitBoardT squareBb = bbForSquare(square);

      colorState.pawnsBb &= ~squareBb;
    }
    
    template <ColorT Color>
    inline void removePromoPiece(BoardT& board, const int promoIndex) {
      ColorStateT &colorState = board.pieces[(size_t)Color];

      colorState.activePromos &= ~((u8)1 << promoIndex);
    }

    template <ColorT Color>
    inline PieceT removePiece(BoardT& board, const ColorPieceMapT& pieceMap, const SquareT square) {
      const PieceT piece = pieceMap.board[square].piece;

      return removePiece<Color>(board, square, piece);
    }

    template <ColorT Color>
    inline void removePromoPiece(BoardT& board, const ColorPieceMapT& pieceMap, const SquareT square) {
      const int promoIndex = pieceMap.board[square].promoIndex;

      removePromoPiece<Color>(board, promoIndex);
    }

    template <ColorT Color>
    inline void removePieceOrPawn(BoardT& board, const ColorPieceMapT& pieceMap, const SquareT square) {
      ColorStateT &pieces = board.pieces[(size_t)Color];
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

    template <ColorT Color, PieceT Piece>
    inline void removePiece(BoardT& board, const SquareT square) {
      removePiece<Color>(board, square, Piece);
    }

    inline void placePiece(BoardT& board, const ColorT color, const SquareT square, const PieceT piece) {
      ColorStateT &colorState = board.pieces[(size_t)color];

      colorState.pieceSquares[piece] = square;
    }
    
    inline void addPromoPiece(BoardT& board, const ColorT color, const int promoIndex, const PromoPieceT promoPiece, const SquareT square) {
      ColorStateT &colorState = board.pieces[(size_t)color];

      colorState.activePromos |= ((u8)1 << promoIndex);
      colorState.promos[promoIndex] = promoPieceAndSquareOf(promoPiece, square);
    }
    
    inline void movePromoPiece(BoardT& board, const ColorT color, const int promoIndex, const PromoPieceT promoPiece, const SquareT square) {
      ColorStateT &colorState = board.pieces[(size_t)color];

      colorState.promos[promoIndex] = promoPieceAndSquareOf(promoPiece, square);
    }
    
    template <ColorT Color>
    inline void placePiece(BoardT& board, const SquareT square, const PieceT piece) {
      placePiece(board, Color, square, piece);
    }

    inline void placePawn(BoardT& board, const ColorT color, const SquareT square) {
      ColorStateT &pieces = board.pieces[(size_t)color];

      const BitBoardT squareBb = bbForSquare(square);

      pieces.pawnsBb |= squareBb;
    }

    template <ColorT Color>
    inline void placePawn(BoardT& board, const SquareT square) {
      placePawn(board, Color, square);
    }

    template <ColorT Color, PieceT Piece>
    inline void placePiece(BoardT& board, const SquareT square) {
      placePiece<Color>(board, square, Piece);
    }
    
    template <ColorT Color, PieceT Piece>
    inline BoardT pushPiece(const BoardT& oldBoard, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePiece<Color, Piece>(board, from);

      placePiece<Color, Piece>(board, to);

      // Clear en-passant square
      board.pieces[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <ColorT Color>
    inline BoardT pushPromoPiece(const BoardT& oldBoard, const int promoIndex, const PromoPieceT promoPiece, const SquareT to) {
      BoardT board = oldBoard;

      movePromoPiece(board, Color, promoIndex, promoPiece, to);

      // Clear en-passant square
      board.pieces[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <ColorT Color, PieceT Piece>
    inline BoardT captureWithPiece(const BoardT& oldBoard, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePieceOrPawn<OtherColorT<Color>::value>(board, yourPieceMap, to);

      removePiece<Color, Piece>(board, from);

      placePiece<Color, Piece>(board, to);

      // Clear en-passant square
      board.pieces[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <ColorT Color, PieceT Piece>
    inline BoardT capturePromoPieceWithPiece(const BoardT& oldBoard, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePromoPiece<OtherColorT<Color>::value>(board, yourPieceMap, to);
      
      removePiece<Color, Piece>(board, from);

      placePiece<Color, Piece>(board, to);

      // Clear en-passant square
      board.pieces[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <ColorT Color>
    inline BoardT captureWithPromoPiece(const BoardT& oldBoard, const int promoIndex, const PromoPieceT promoPiece, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePieceOrPawn<OtherColorT<Color>::value>(board, yourPieceMap, to);

      movePromoPiece(board, Color, promoIndex, promoPiece, to);

      // Clear en-passant square
      board.pieces[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <ColorT Color>
    inline BoardT capturePromoPieceWithPromoPiece(const BoardT& oldBoard, const int promoIndex, const PromoPieceT promoPiece, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePromoPiece<OtherColorT<Color>::value>(board, yourPieceMap, to);

      movePromoPiece(board, Color, promoIndex, promoPiece, to);

      // Clear en-passant square
      board.pieces[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <ColorT Color>
    inline BoardT captureWithPawn(const BoardT& oldBoard, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePieceOrPawn<OtherColorT<Color>::value>(board, yourPieceMap, to);

      removePawn<Color>(board, from);

      placePawn<Color>(board, to);

      // Clear en-passant square
      board.pieces[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <ColorT Color>
    inline BoardT captureWithPawnToPromo(const BoardT& oldBoard, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
      BoardT board = oldBoard;

      removePieceOrPawn<OtherColorT<Color>::value>(board, yourPieceMap, to);

      removePawn<Color>(board, from);

      const int promoIndex = Bits::lsb(~board.pieces[(size_t)Color].activePromos);
      addPromoPiece(board, Color, promoIndex, promoPiece, to);

      // Clear en-passant square
      board.pieces[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <ColorT Color>
    inline BoardT capturePromoPieceWithPawn(const BoardT& oldBoard, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePromoPiece<OtherColorT<Color>::value>(board, yourPieceMap, to);

      removePawn<Color>(board, from);

      placePawn<Color>(board, to);
      
      // Clear en-passant square
      board.pieces[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <ColorT Color>
    inline BoardT capturePromoPieceWithPawnToPromo(const BoardT& oldBoard, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
      BoardT board = oldBoard;

      removePromoPiece<OtherColorT<Color>::value>(board, yourPieceMap, to);

      removePawn<Color>(board, from);

      const int promoIndex = Bits::lsb(~board.pieces[(size_t)Color].activePromos);
      addPromoPiece(board, Color, promoIndex, promoPiece, to);
      
      // Clear en-passant square
      board.pieces[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <ColorT Color, bool IsPawnPushTwo = false>
    inline BoardT pushPawn(const BoardT& oldBoard, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePawn<Color>(board, from);

      placePawn<Color>(board, to);

      // Set en-passant square
      if(IsPawnPushTwo) {
      	board.pieces[(size_t)Color].epSquare = (SquareT)((from+to)/2);
      } else {
      	board.pieces[(size_t)Color].epSquare = InvalidSquare;
      }	
      
      return board;
    }
    
    template <ColorT Color>
    inline BoardT pushPawnToPromo(const BoardT& oldBoard, const SquareT from, const SquareT to, PromoPieceT promoPiece) {
      BoardT board = oldBoard;

      removePawn<Color>(board, from);

      const int promoIndex = Bits::lsb(~board.pieces[(size_t)Color].activePromos);
      addPromoPiece(board, Color, promoIndex, promoPiece, to);

      // Clear en-passant square
      board.pieces[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <ColorT Color>
    inline BoardT captureEp(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      BoardT board = oldBoard;

      removePawn<OtherColorT<Color>::value>(board, captureSquare);

      removePawn<Color>(board, from);

      placePawn<Color>(board, to);

      // Clear en-passant square
      board.pieces[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }

    // Note - MUST use this rather than zero-initialisation because piece squares need to be InvalidSquare
    BoardT emptyBoard();
    
    BoardT startingPosition();

    bool isValid(const BoardT& board, const BitBoardT allYourKingAttackersBb);

    // These are used for FEN output
    char pieceChar(const std::vector<std::pair<ColorT, PieceTypeT>>& squarePieces);
    std::array<std::vector<std::pair<ColorT, PieceTypeT>>, 64> genPieceMap(const BoardT& board);

    void printBoard(const BoardT& board);
    void printBb(BitBoardT bb);
  }
    
}

#endif //ndef BOARD_HPP
