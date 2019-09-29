#ifndef CHESS_H
#define CHESS_H

#include "types.h"

namespace Chess {
  
  template <ColorT> int f();

  template <> inline int f<Black>() {
    return 1;
  }

  template <> inline int f<White>() {
    return 2;
  }
}

#endif //ndef CHESS_H
