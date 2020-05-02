#include "types.hpp"

namespace Chess {
  
  namespace BoardUtils {
    char PieceChar[NColors][NPieceTypes+1] = {
      // White
      { ".PNBRQK" },
      // Black
      { ".pnbrqk" }
    };

  } // namespace BoardUtils
  
} // namespace Chess
