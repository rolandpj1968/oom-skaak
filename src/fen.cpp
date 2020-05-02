#include "fen.hpp"

namespace Chess {

  using namespace Board;
  
  //typedef SimpleBoardT BoardT;
    
  namespace Fen {

    const std::map<char, PieceTypeAndColorT> FenPieces = {
      { 'P', PieceTypeAndColorT(Pawn, White) },
      { 'N', PieceTypeAndColorT(Knight, White) },
      { 'B', PieceTypeAndColorT(Bishop, White) },
      { 'R', PieceTypeAndColorT(Rook, White) },
      { 'Q', PieceTypeAndColorT(Queen, White) },
      { 'K', PieceTypeAndColorT(King, White) },
      { 'p', PieceTypeAndColorT(Pawn, Black) },
      { 'n', PieceTypeAndColorT(Knight, Black) },
      { 'b', PieceTypeAndColorT(Bishop, Black) },
      { 'r', PieceTypeAndColorT(Rook, Black) },
      { 'q', PieceTypeAndColorT(Queen, Black) },
      { 'k', PieceTypeAndColorT(King, Black) },
    };


  } // namespace Fen
} // namespace Chess
