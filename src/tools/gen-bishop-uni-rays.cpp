#include "cstdio"

#include "types.hpp"
#include "move-gen.hpp"

using namespace Chess;
using namespace MoveGen;

// g++ -I ../ -o gen-bishop-uni-rays gen-bishop-uni-rays.cpp ../move-gen.cpp

int main() {
  // Bishop uni-rays
  printf("// Bishop uni-rays right\n{\n");
  for(SquareT square = 0; square < 64; square++) {
    BitBoardT bishopRaysRight = Rays[NE][square] | bbForSquare(square) | Rays[SW][square];
    printf("0x%016lxull,%s", bishopRaysRight, ((square+1) % 4 == 0 ? "\n" : " "));
  }
  printf("// Bishop uni-rays left\n{\n");
  for(SquareT square = 0; square < 64; square++) {
    BitBoardT bishopRaysRight = Rays[NW][square] | bbForSquare(square) | Rays[SE][square];
    printf("0x%016lxull,%s", bishopRaysRight, ((square+1) % 4 == 0 ? "\n" : " "));
  }
  printf("}\n");
}
