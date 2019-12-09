#ifndef BOARD_HPP
#define BOARD_HPP

#include "types.hpp"

namespace Chess {
  
  namespace Board {

    typedef u8 SquarePieceT;

    const SquarePieceT EmptySquare = NoPiece;

    inline SquarePieceT makeSquarePiece(const ColorT color, const PieceT piece) {
      return ((int)color << 7) | piece;
    }

    inline ColorT squarePieceColor(const SquarePieceT squarePiece) {
      return (ColorT)(squarePiece >> 7);
    }

    inline PieceT squarePiecePiece(const SquarePieceT squarePiece) {
      return (PieceT) (squarePiece & ~((int)Black << 7));
    }

    struct ColorStateT {
      // All pieces including strange promos.
      BitBoardT bbsOld[NPieceTypes];

      // All pieces except pawns and strange promos.
      // MUST be InvalidSquare if a piece is not present
      SquareT pieceSquares[NPieces];

      // True iff there are promo pieces on the board.
      bool hasPromos;

      // InvalidSquare, or else en-passant square of the last move - i.e. the square behind a pawn two-square push.
      SquareT epSquare;

      // Castling rights
      CastlingRightsT castlingRights;
    };

    struct BoardT {
      ColorStateT pieces[NColors];

      SquarePieceT board[64];
    };

    const bool DoesNotHavePromos = false;
    const bool DoesHavePromos = true;
    
    // Board traits used for optimising move generation etc.
    // The compile-time traits can be false only if the run-time state is also false.
    //   On the other hand the code should handle compile-time traits being true even if run-time traits are false.
    template <ColorT ColorVal, bool HasPromosVal, bool CanCastleVal = true, bool HasQueenVal = true, bool HasRooksVal = true, bool HasBishopsVal = true>
    struct ColorTraitsImplT {
      static const ColorT Color = ColorVal;
      static const bool HasPromos = HasPromosVal;
      static const bool CanCastle = CanCastleVal;
      static const bool HasQueen = HasQueenVal;
      static const bool HasRooks = HasRooksVal;
      static const bool HasBishops = HasBishopsVal;

      typedef ColorTraitsImplT<Color, DoesHavePromos, CanCastle, HasQueen, HasRooks, HasBishops> WithPromosT;
      typedef ColorTraitsImplT<Color, DoesNotHavePromos, CanCastle, HasQueen, HasRooks, HasBishops> WithoutPromosT;
    };

    typedef ColorTraitsImplT<White, DoesNotHavePromos> WhiteStartingColorTraitsT;
    typedef ColorTraitsImplT<Black, DoesNotHavePromos> BlackStartingColorTraitsT;

    template <typename MyColorTraitsImplT, typename YourColorTraitsImplT>
    struct BoardTraitsImplT {
      typedef MyColorTraitsImplT MyColorTraitsT;
      typedef YourColorTraitsImplT YourColorTraitsT;

      static const ColorT Color = MyColorTraitsT::Color;
      static const ColorT OtherColor = YourColorTraitsT::Color;

      typedef BoardTraitsImplT<YourColorTraitsT, MyColorTraitsT> ReverseT;
    };

    typedef BoardTraitsImplT<WhiteStartingColorTraitsT, BlackStartingColorTraitsT> StartingBoardTraitsT;

    template <ColorT Color>
    inline PieceT removePiece(BoardT& board, const SquareT square, const PieceT piece) {
      board.board[square] = EmptySquare;
      
      // TODO handle non-standard promos
      ColorStateT &pieces = board.pieces[(size_t)Color];

      pieces.pieceSquares[piece] = InvalidSquare;

      const BitBoardT squareBb = bbForSquare(square);

      const PieceTypeT pieceType = PieceTypeForPiece[piece];
      pieces.bbsOld[pieceType] &= ~squareBb;
      pieces.bbsOld[AllPieceTypes] &= ~squareBb;

      if(piece == QueenRook) {
	pieces.castlingRights = (CastlingRightsT) (pieces.castlingRights & ~CanCastleQueenside);
      }
      if(piece == KingRook) {
	pieces.castlingRights = (CastlingRightsT) (pieces.castlingRights & ~CanCastleKingside);
      }
      if(piece == TheKing) {
	pieces.castlingRights = NoCastlingRights;
      }
      
      return piece;
    }
    
    template <ColorT Color>
    inline PieceT removePiece(BoardT& board, const SquareT square) {
      const SquarePieceT squarePiece = board.board[square];
      const PieceT piece = squarePiecePiece(squarePiece);

      return removePiece<Color>(board, square, piece);
    }

    template <ColorT Color, PieceT Piece>
    inline void removePiece(BoardT& board, const SquareT square) {
      removePiece<Color>(board, square, Piece);
    }

    // TODO -  promos
    template <ColorT Color>
    inline void placePiece(BoardT& board, const SquareT square, const PieceT piece) {
      board.board[square] = makeSquarePiece(Color, piece);

      // TODO handle non-standard promos
      ColorStateT &pieces = board.pieces[(size_t)Color];

      pieces.pieceSquares[piece] = square;

      const BitBoardT squareBb = bbForSquare(square);

      const PieceTypeT pieceType = PieceTypeForPiece[piece];
      pieces.bbsOld[pieceType] |= squareBb;
      pieces.bbsOld[AllPieceTypes] |= squareBb;
    }

    // TODO - non-standard promos
    template <ColorT Color, PieceT Piece>
    inline void placePiece(BoardT& board, const SquareT square) {
      placePiece<Color>(board, square, Piece);
    }
    
    template <ColorT Color, PushOrCaptureT PushOrCapture, bool IsPawnPushTwo = false>
    inline BoardT move(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      BoardT board = oldBoard;

      if(PushOrCapture == Capture) {
    	removePiece<OtherColorT<Color>::value>(board, captureSquare);
      }

      PieceT piece = removePiece<Color>(board, from);

      placePiece<Color>(board, to, piece);

      // Set en-passant square
      if(IsPawnPushTwo) {
    	board.pieces[(size_t)Color].epSquare = (SquareT)((from+to)/2);
      } else {
	board.pieces[(size_t)Color].epSquare = InvalidSquare;
      }
      
      return board;
    }
    
    template <ColorT Color, PieceT Piece, PushOrCaptureT PushOrCapture, bool IsPawnPushTwo = false>
    inline BoardT movePiece(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      BoardT board = oldBoard;

      if(PushOrCapture == Capture) {
      	removePiece<OtherColorT<Color>::value>(board, captureSquare);
      }

      removePiece<Color, Piece>(board, from);

      placePiece<Color, Piece>(board, to);

      // Set en-passant square
      if(IsPawnPushTwo) {
      	board.pieces[(size_t)Color].epSquare = (SquareT)((from+to)/2);
      } else {
      	board.pieces[(size_t)Color].epSquare = InvalidSquare;
      }	
      
      return board;
    }
    
    template <ColorT Color, PieceT Piece, PieceT PieceCaptured>
    inline BoardT capturePiece(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      BoardT board = oldBoard;

      removePiece<OtherColorT<Color>::value, PieceCaptured>(board, captureSquare);

      removePiece<Color, Piece>(board, from);

      placePiece<Color, Piece>(board, to);

      // Clear en-passant square
      board.pieces[(size_t)Color].epSquare = InvalidSquare;
      
      return board;
    }
    
    template <ColorT Color, PushOrCaptureT PushOrCapture, bool IsPawnPushTwo = false>
    inline BoardT move(const BoardT& oldBoard, const SquareT from, const SquareT to) {
      return move<Color, PushOrCapture, IsPawnPushTwo>(oldBoard, from, to, /*captureSquare = */to);
    }

    template <ColorT Color, PieceT Piece, PushOrCaptureT PushOrCapture, bool IsPawnPushTwo = false>
    inline BoardT movePiece(const BoardT& oldBoard, const SquareT from, const SquareT to) {
      return movePiece<Color, Piece, PushOrCapture, IsPawnPushTwo>(oldBoard, from, to, /*captureSquare = */to);
    }

    template <ColorT Color>
    inline BoardT captureEp(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      return capturePiece<Color, SomePawns, SomePawns>(oldBoard, from, to, captureSquare);
    }
    
    extern BoardT startingPosition();

    extern void printBoard(const BoardT& board);
    extern void printBb(BitBoardT bb);
  }
    
}

#endif //ndef BOARD_HPP
