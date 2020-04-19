#ifndef BITS_H
#define BITS_H

#include "types.hpp"

namespace Chess {

  namespace Bits {
    
    inline int popLsb(BitBoardT &bb) {
      int lsbIndex = __builtin_ffsll(bb) - 1;
      bb &= bb - 1;
      return lsbIndex;
    }

    inline int count(const BitBoardT bb) {
      return __builtin_popcountll(bb);
    }

    inline int lsb(const BitBoardT bb) {
      if(bb == 0) {
	return -1;
      }
      return __builtin_ffsll(bb) - 1;
    }

    inline int msb(const BitBoardT bb) {
      if(bb == 0) {
	return -1;
      }
      return 63 - __builtin_clzll(bb);
    }

    inline BitBoardT eastNTimes(const BitBoardT bb, const int n) {
      BitBoardT newBb = bb;
      for(int i = 0; i < n; i++) {
	newBb = (newBb << 1) & ~FileA;
      }

      return newBb;
    }

    inline BitBoardT westNTimes(const BitBoardT bb, const int n) {
      BitBoardT newBb = bb;
      for(int i = 0; i < n; i++) {
	newBb = (newBb >> 1) & ~FileH;
      }

      return newBb;
    }

  } // namespace Bits

} // namespace Chess

#endif //ndef BITS_H
