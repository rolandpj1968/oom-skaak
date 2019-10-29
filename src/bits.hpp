#ifndef BITS_H
#define BITS_H

#include "types.hpp"

namespace Chess {

  namespace Bits {
    
    inline int popLsb(BitBoardT &board) {
      int lsbIndex = __builtin_ffsll(board) - 1;
      board &= board - 1;
      return lsbIndex;
    }

    inline int count(const BitBoardT board) {
      return __builtin_popcountll(board);
    }

    inline int lsb(const BitBoardT board) {
      if(board == 0) {
	return -1;
      }
      return __builtin_ffsll(board) - 1;
    }

    inline int msb(const BitBoardT board) {
      if(board == 0) {
	return -1;
      }
      return 63 - __builtin_clzll(board);
    }

    inline BitBoardT eastNTimes(const BitBoardT board, const int n) {
      BitBoardT newBoard = board;
      for(int i = 0; i < n; i++) {
	newBoard = (newBoard << 1) & ~FileA;
      }

      return newBoard;
    }

    inline BitBoardT westNTimes(const BitBoardT board, const int n) {
      BitBoardT newBoard = board;
      for(int i = 0; i < n; i++) {
	newBoard = (newBoard >> 1) & ~FileH;
      }

      return newBoard;
    }

  } // namespace Bits

} // namespace Chess

#endif //ndef BITS_H
