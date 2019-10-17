#include "cstdio"

#include "types.hpp"
#include "move-gen.hpp"

using namespace Chess;

int main() {
  // Bishop rays generated using magic bitboards with no other pieces present on the board :D
  printf("// Bishop rays\n{\n");
  for(SquareT square = 0; square < 64; square++) {
    BitBoardT bishopRays = MoveGen::bishopAttacks(square, BbNone);
    printf("0x%016lx,%s", bishopRays, ((square+1) % 4 == 0 ? "\n" : " "));
  }
  printf("}\n");
}
