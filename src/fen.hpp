#ifndef FEN_HPP
#define FEN_HPP

#include <stdexcept>
#include <string>
#include <utility>

#include "board.hpp"
#include "types.hpp"

namespace Chess {
  
  using namespace Board;
  
  namespace Fen {

    std::pair<SimpleBoardT, ColorT> parseFen(const std::string& fen);

    std::string toFen(const SimpleBoardT& board, const ColorT colorToMove);

  } // namespace Fen
} // namespace Chess

#endif //ndef FEN_HPP
