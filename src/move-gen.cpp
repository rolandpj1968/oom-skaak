#include <cstdio>

#include "move-gen.hpp"
//#include "bits.hpp"

namespace Chess {
  namespace MoveGen {
    BitBoardT Rays[8][64];

    static void initRays() {
      for(SquareT square = A1; square <= H8; square++) {
	// North
	Rays[N][square] = 0x0101010101010100ULL << square;

	// South
	Rays[S][square] = 0x0080808080808080ULL >> (63 - square);

	// East
	Rays[E][square] = 2 * (bbForSquare(square | 7) - bbForSquare(square));

	// West
	Rays[W][square] = bbForSquare(square) - bbForSquare(square & 56);

	// North West
	Rays[NW][square] = Bits::westNTimes(0x102040810204000ULL, 7 - fileOf(square)) << (rankOf(square) * 8);

	// North East
	Rays[NE][square] = Bits::eastNTimes(0x8040201008040200ULL, fileOf(square)) << (rankOf(square) * 8);

	// South West
	Rays[SW][square] = Bits::westNTimes(0x40201008040201ULL, 7 - fileOf(square)) >> ((7 - rankOf(square)) * 8);

	// South East
	Rays[SE][square] = Bits::eastNTimes(0x2040810204080ULL, fileOf(square)) >> ((7 - rankOf(square)) * 8);
      }
    }

    static BitBoardT blockersForIndex(const int index, BitBoardT mask) {
      BitBoardT blockers = 0;
      int nBits = Bits::count(mask);
      for (int i = 0; i < nBits; i++) {
	SquareT square = Bits::popLsb(mask);
	if(index & (1 << i)) {
	  blockers |= bbForSquare(square);
	}
      }
      return blockers;
    }

    
    BitBoardT RookMagicBbTable[64+1][4096];
    
    static BitBoardT rookAttacksSlow(const int square, const BitBoardT blockers) {
      BitBoardT attacks = 0;

      // North
      attacks |= Rays[N][square];
      if(Rays[N][square] & blockers) {
	attacks &= ~Rays[N][Bits::lsb(Rays[N][square] & blockers)];
      }

      // South
      attacks |= Rays[S][square];
      if(Rays[S][square] & blockers) {
	attacks &= ~Rays[S][Bits::msb(Rays[S][square] & blockers)];
      }

      // East
      attacks |= Rays[E][square];
      if(Rays[E][square] & blockers) {
	attacks &= ~Rays[E][Bits::lsb(Rays[E][square] & blockers)];
      }

      // West
      attacks |= Rays[W][square];
      if(Rays[W][square] & blockers) {
	attacks &= ~Rays[W][Bits::msb(Rays[W][square] & blockers)];
      }

      return attacks;
    }
    
    static void initRookMagicBbTable() {
      for(SquareT square = A1; square <= H8; square++) {
	for(int blockerIndex = 0; blockerIndex < (1 << RookMagicBbIndexBits[square]); blockerIndex++) {
	  BitBoardT blockers = blockersForIndex(blockerIndex, RookBlockers[square]);
	  RookMagicBbTable[square][(blockers * RookMagicBbMultipliers[square]) >> (64 - RookMagicBbIndexBits[square])] = rookAttacksSlow(square, blockers);
	}
      }
      RookMagicBbTable[InvalidSquare][0] = BbNone; // InvalidSquare (non-)moves
    }
    
    BitBoardT BishopMagicBbTable[64+1][1024];
    
    static BitBoardT bishopAttacksSlow(const int square, const BitBoardT blockers) {
      BitBoardT attacks = 0;

      // North West
      attacks |= Rays[NW][square];
      if(Rays[NW][square] & blockers) {
	attacks &= ~Rays[NW][Bits::lsb(Rays[NW][square] & blockers)];
      }

      // North East
      attacks |= Rays[NE][square];
      if(Rays[NE][square] & blockers) {
	attacks &= ~Rays[NE][Bits::lsb(Rays[NE][square] & blockers)];
      }

      // South East
      attacks |= Rays[SE][square];
      if(Rays[SE][square] & blockers) {
	attacks &= ~Rays[SE][Bits::msb(Rays[SE][square] & blockers)];
      }

      // South West
      attacks |= Rays[SW][square];
      if(Rays[SW][square] & blockers) {
	attacks &= ~Rays[SW][Bits::msb(Rays[SW][square] & blockers)];
      }

      return attacks;
    }

    static void initBishopMagicBbTable() {
      // For all squares
      for(SquareT square = A1; square <= H8; square++) {
	// For all possible blockers for this square
	for(int blockerIndex = 0; blockerIndex < (1 << BishopMagicBbIndexBits[square]); blockerIndex++) {
	  BitBoardT blockers = blockersForIndex(blockerIndex, BishopBlockers[square]);
	  BishopMagicBbTable[square][(blockers * BishopMagicBbMultipliers[square]) >> (64 - BishopMagicBbIndexBits[square])] = bishopAttacksSlow(square, blockers);
	}
      }
      BishopMagicBbTable[InvalidSquare][0] = BbNone; // InvalidSquare (non)moves
    }

    struct MagicBbInit {
      MagicBbInit() {
	// Order is important here!
	initRays();
	
	initRookMagicBbTable();
	initBishopMagicBbTable();
      }
    } magicBbInit;
    
  } // namespace MoveGen

} // namespace Chess
