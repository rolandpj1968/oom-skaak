#ifndef BOARD_HPP
#define BOARD_HPP

#include "types.hpp"

namespace Chess {
  
  namespace Board {

    typedef u8 SquarePieceT;

    const SquarePieceT EmptySquare = SpecificNoPiece;

    inline SquarePieceT makeSquarePiece(const ColorT color, const SpecificPieceT specificPiece) {
      return (color << 7) | specificPiece;
    }

    inline ColorT squarePieceColor(const SquarePieceT squarePiece) {
      return (ColorT)(squarePiece >> 7);
    }

    inline SpecificPieceT squarePieceSpecificPiece(const SquarePieceT squarePiece) {
      return (SpecificPieceT) (squarePiece & ~(Black << 7));
    }

    struct ColorStateT {
      // All pieces including strange promos.
      BitBoardT bbs[NPieceTypes];

      // All pieces except pawns and strange promos.
      // MUST be InvalidSquare if a piece is not present
      SquareT pieceSquares[NSpecificPieceTypes];

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
    template <bool HasPromos, bool CanCastle = true, bool HasQueen = true, bool HasRooks = true, bool HasBishops = true>
    struct BoardTraitsImplT {
      static const bool hasPromos = HasPromos;
      static const bool canCastle = CanCastle;
      static const bool hasQueen = HasQueen;
      static const bool hasRooks = HasRooks;
      static const bool hasBishops = HasBishops;

      typedef BoardTraitsImplT<DoesHavePromos, CanCastle, HasQueen, HasRooks, HasBishops> WithPromosT;
      typedef BoardTraitsImplT<DoesNotHavePromos, CanCastle, HasQueen, HasRooks, HasBishops> WithoutPromosT;
    };

    typedef BoardTraitsImplT<DoesNotHavePromos> StartingBoardTraitsT;

    template <ColorT Color> inline SpecificPieceT removePiece(BoardT& board, const SquareT square) {
      const SquarePieceT squarePiece = board.board[square];
      const SpecificPieceT specificPiece = squarePieceSpecificPiece(squarePiece);

      board.board[square] = EmptySquare;
      
      // TODO handle non-standard promos
      ColorStateT &pieces = board.pieces[Color];

      pieces.pieceSquares[specificPiece] = InvalidSquare;

      const BitBoardT squareBb = bbForSquare(square);

      const PieceT piece = PieceForSpecificPiece[specificPiece];
      pieces.bbs[piece] &= ~squareBb;
      pieces.bbs[AllPieces] &= ~squareBb;

      if(specificPiece == QueenRook) {
	pieces.castlingRights = (CastlingRightsT) (pieces.castlingRights & ~CanCastleQueenside);
      }
      if(specificPiece == KingRook) {
	pieces.castlingRights = (CastlingRightsT) (pieces.castlingRights & ~CanCastleKingside);
      }
      if(specificPiece == SpecificKing) {
	pieces.castlingRights = NoCastlingRights;
      }
      
      return specificPiece;
    }

    template <ColorT Color, SpecificPieceT SpecificPiece> inline void removeSpecificPiece(BoardT& board, const SquareT square) {
      board.board[square] = EmptySquare;
      
      // TODO handle non-standard promos
      ColorStateT &pieces = board.pieces[Color];

      pieces.pieceSquares[SpecificPiece] = InvalidSquare;

      const BitBoardT squareBb = bbForSquare(square);

      const PieceT piece = PieceForSpecificPieceT<SpecificPiece>::value;
      pieces.bbs[piece] &= ~squareBb;
      pieces.bbs[AllPieces] &= ~squareBb;

      if(SpecificPiece == QueenRook) {
	pieces.castlingRights = (CastlingRightsT) (pieces.castlingRights & ~CanCastleQueenside);
      }
      if(SpecificPiece == KingRook) {
	pieces.castlingRights = (CastlingRightsT) (pieces.castlingRights & ~CanCastleKingside);
      }
      if(SpecificPiece == SpecificKing) {
	pieces.castlingRights = NoCastlingRights;
      }
    }

    template <ColorT Color> inline void placePiece(BoardT& board, const SpecificPieceT specificPiece, const SquareT square) {
      board.board[square] = makeSquarePiece(Color, specificPiece);

      // TODO handle non-standard promos
      ColorStateT &pieces = board.pieces[Color];

      pieces.pieceSquares[specificPiece] = square;

      const BitBoardT squareBb = bbForSquare(square);

      const PieceT piece = PieceForSpecificPiece[specificPiece];
      pieces.bbs[piece] |= squareBb;
      pieces.bbs[AllPieces] |= squareBb;
    }

    // TODO - non-standard promos
    template <ColorT Color, SpecificPieceT SpecificPiece> inline void placeSpecificPiece(BoardT& board, const SquareT square) {
      board.board[square] = makeSquarePiece(Color, SpecificPiece);

      // TODO handle non-standard promos
      ColorStateT &pieces = board.pieces[Color];

      pieces.pieceSquares[SpecificPiece] = square;

      const BitBoardT squareBb = bbForSquare(square);

      const PieceT piece = PieceForSpecificPieceT<SpecificPiece>::value;
      pieces.bbs[piece] |= squareBb;
      pieces.bbs[AllPieces] |= squareBb;
    }

    inline BoardT copyForMove(const BoardT& oldBoard) {
      BoardT board = oldBoard;
      // En-passant squares are always lost after a move.
      // Only actually need to clear one side's square, so could template this on Color...
      board.pieces[White].epSquare = InvalidSquare;
      board.pieces[Black].epSquare = InvalidSquare;

      return board;
    }

    template <ColorT Color, PushOrCaptureT PushOrCapture, bool IsPawnPushTwo = false> inline BoardT move(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      BoardT board = copyForMove(oldBoard);

      if(PushOrCapture == Capture) {
    	removePiece<OtherColorT<Color>::value>(board, captureSquare);
      }

      SpecificPieceT specificPiece = removePiece<Color>(board, from);

      placePiece<Color>(board, specificPiece, to);

      // Set en-passant square
      if(IsPawnPushTwo) {
    	board.pieces[Color].epSquare = (SquareT)((from+to)/2);
      }
      
      return board;
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece, PushOrCaptureT PushOrCapture, bool IsPawnPushTwo = false> inline BoardT moveSpecificPiece(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      BoardT board = copyForMove(oldBoard);

      if(PushOrCapture == Capture) {
	removePiece<OtherColorT<Color>::value>(board, captureSquare);
      }

      removeSpecificPiece<Color, SpecificPiece>(board, from);

      placeSpecificPiece<Color, SpecificPiece>(board, to);

      // Set en-passant square
      if(IsPawnPushTwo) {
	board.pieces[Color].epSquare = (SquareT)((from+to)/2);
      }
      
      return board;
    }
    
    template <ColorT Color, SpecificPieceT SpecificPiece, SpecificPieceT SpecificPieceCaptured> inline BoardT captureSpecificPiece(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      BoardT board = copyForMove(oldBoard);

      removeSpecificPiece<OtherColorT<Color>::value, SpecificPieceCaptured>(board, captureSquare);

      removeSpecificPiece<Color, SpecificPiece>(board, from);

      placeSpecificPiece<Color, SpecificPiece>(board, to);

      return board;
    }
    
    template <ColorT Color, PushOrCaptureT PushOrCapture, bool IsPawnPushTwo = false> inline BoardT move(const BoardT& oldBoard, const SquareT from, const SquareT to) {
      return move<Color, PushOrCapture, IsPawnPushTwo>(oldBoard, from, to, /*captureSquare = */to);
    }

    template <ColorT Color, SpecificPieceT SpecificPiece, PushOrCaptureT PushOrCapture, bool IsPawnPushTwo = false> inline BoardT moveSpecificPiece(const BoardT& oldBoard, const SquareT from, const SquareT to) {
      return moveSpecificPiece<Color, SpecificPiece, PushOrCapture, IsPawnPushTwo>(oldBoard, from, to, /*captureSquare = */to);
    }

    template <ColorT Color> inline BoardT captureEp(const BoardT& oldBoard, const SquareT from, const SquareT to, const SquareT captureSquare) {
      return captureSpecificPiece<Color, SpecificPawn, SpecificPawn>(oldBoard, from, to, captureSquare);
    }
    
    extern BoardT startingPosition();

    extern void printBoard(const BoardT& board);
    extern void printBb(BitBoardT bb);
  }
    
}

#endif //ndef BOARD_HPP
