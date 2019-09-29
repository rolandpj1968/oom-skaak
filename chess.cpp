#include <cstdio>

#include <boost/preprocessor/iteration/local.hpp>

#include "chess.hpp"
#include "move_gen.hpp"

using namespace Chess;

#define BOOST_PP_LOCAL_MACRO(n) \
  (n),

#define BOOST_PP_LOCAL_LIMITS (0, 15)

int a[] = {
#include BOOST_PP_LOCAL_ITERATE()
};

int main(int argc, char* argv[]) {

  printf("Hallo RPJ - sizeof(Color) is %zu, f<Black> is %d, f<White> is %d\n", sizeof(ColorT), f<Black>(), f<White>());
  printf("Hallo again RPJ - sizeof(a) is %zu, sizeof(a)/sizeof(a[0]) = %zu, a[4] = %d, a[0] = %d\n", sizeof(a), sizeof(a)/sizeof(a[0]), a[4], a[0]);
  return 0;
}
