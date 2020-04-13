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

    std::pair<BoardT, ColorT> parseFen(const std::string& fen);

  } // namespace Fen
} // namespace Chess

#endif //ndef FEN_HPP
