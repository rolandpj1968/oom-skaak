#include "cstdio"

#include "types.hpp"
#include "move-gen.hpp"

using namespace Chess;

int main() {
  // Rook rays generated using magic bitboards with no other pieces present on the board :D
  printf("// Rook rays\n{\n");
  for(SquareT square = 0; square < 64; square++) {
    BitBoardT rookRays = MoveGen::rookAttacks(square, BbNone);
    printf("0x%016lx,%s", rookRays, ((square+1) % 4 == 0 ? "\n" : " "));
  }
  printf("}\n");
}
