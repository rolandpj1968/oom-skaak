#include "cstdio"

#include "types.hpp"

using namespace Chess;

int main() {
  // White pawn attackers which are white/lo-side of the attacked square
  printf("// White pawn attackers\n{\n");
  for(SquareT square = 0; square < 64; square++) {
    BitBoardT squareBb = bbForSquare(square);
    BitBoardT leftAttackerBb = (squareBb & ~(FileA | Rank1)) >> 9;
    BitBoardT rightAttackerBb = (squareBb & ~(FileH | Rank1)) >> 7;
    printf("0x%016lx,%s", (leftAttackerBb | rightAttackerBb), ((square+1) % 4 == 0 ? "\n" : " "));
  }
  printf("},\n");
  // Black pawn attackers which are black/hi-side of the attacked square
  printf("// Black pawn attackers\n{\n");
  for(SquareT square = 0; square < 64; square++) {
    BitBoardT squareBb = bbForSquare(square);
    BitBoardT leftAttackerBb = (squareBb & ~(FileA | Rank8)) << 7;
    BitBoardT rightAttackerBb = (squareBb & ~(FileH | Rank8)) << 9;
    printf("0x%016lx,%s", (leftAttackerBb | rightAttackerBb), ((square+1) % 4 == 0 ? "\n" : " "));
  }
  printf("},\n");
}
