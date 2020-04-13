#ifndef BOARD_HPP
#define BOARD_HPP

#include "types.hpp"

namespace Chess {
  
  namespace Board {

    struct ColorStateT {
      // All pieces including strange promos.
      BitBoardT pawnsBb;

      // All pieces except pawns and strange promos.
      // MUST be InvalidSquare if a piece is not present - see emptyBoard()
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

    // Piece map container
    struct ColorPieceMapT {
      PieceT board[64+1]; // Allow board[InvalidSquare]
    };
    
    // Generate the (non-pawn) piece map for a color
    template <typename ColorTraitsImplT>
    inline ColorPieceMapT genColorPieceMap(const ColorStateT& state) {
      ColorPieceMapT pieceMap = {};

      // TODO promos
      pieceMap.board[state.pieceSquares[Knight1]] = Knight1;
      pieceMap.board[state.pieceSquares[Knight2]] = Knight2;
      pieceMap.board[state.pieceSquares[BlackBishop]] = BlackBishop;
      pieceMap.board[state.pieceSquares[WhiteBishop]] = WhiteBishop;
      pieceMap.board[state.pieceSquares[QueenRook]] = QueenRook;
      pieceMap.board[state.pieceSquares[KingRook]] = KingRook;
      pieceMap.board[state.pieceSquares[TheQueen]] = TheQueen;
      pieceMap.board[state.pieceSquares[TheKing]] = TheKing;
      
      return pieceMap;
    }

    template <ColorT Color>
    inline PieceT removePiece(BoardT& board, const SquareT square, const PieceT piece) {
      // TODO handle non-standard promos
      ColorStateT &pieces = board.pieces[(size_t)Color];

      pieces.pieceSquares[piece] = InvalidSquare;

      pieces.castlingRights = (CastlingRightsT) (pieces.castlingRights & ~CastlingRightsForPiece[piece]);
      
      return piece;
    }
    
    template <ColorT Color>
    inline void removePawn(BoardT& board, const SquareT square) {
      // TODO handle non-standard promos
      ColorStateT &pieces = board.pieces[(size_t)Color];

      const BitBoardT squareBb = bbForSquare(square);

      pieces.pawnsBb &= ~squareBb;
    }
    
    template <ColorT Color>
    inline PieceT removePiece(BoardT& board, const ColorPieceMapT& yourPieceMap, const SquareT square) {
      const PieceT piece = yourPieceMap.board[square];

      return removePiece<Color>(board, square, piece);
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
      const PieceT piece = pieceMap.board[square]; // NoPiece for pawns
      pieces.pieceSquares[piece] = InvalidSquare;

      pieces.castlingRights = (CastlingRightsT) (pieces.castlingRights & ~CastlingRightsForPiece[piece]);
    }

    template <ColorT Color, PieceT Piece>
    inline void removePiece(BoardT& board, const SquareT square) {
      removePiece<Color>(board, square, Piece);
    }

    // TODO -  promos
    inline void placePiece(BoardT& board, const ColorT color, const SquareT square, const PieceT piece) {
      // TODO handle non-standard promos
      ColorStateT &pieces = board.pieces[(size_t)color];

      pieces.pieceSquares[piece] = square;
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

    // TODO - non-standard promos
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
    
    template <ColorT Color, bool IsPawnPushTwo = false>
    inline BoardT captureWithPawn(const BoardT& oldBoard, const ColorPieceMapT& yourPieceMap, const SquareT from, const SquareT to) {
      BoardT board = oldBoard;

      removePieceOrPawn<OtherColorT<Color>::value>(board, yourPieceMap, to);

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
    
    void printBoard(const BoardT& board);
    void printBb(BitBoardT bb);
  }
    
}

#endif //ndef BOARD_HPP
