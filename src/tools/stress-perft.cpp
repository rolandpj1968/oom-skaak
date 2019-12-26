#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <unistd.h>

#include "board.hpp"
#include "perft.hpp"

using namespace Chess;
using namespace Perft;

u64 time_ms() {
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    return (u64)spec.tv_sec * 1000 + (u64)spec.tv_nsec / 1000000;
}

// g++ -O -I .. -I ../perft -o stress-perft stress-perft.cpp ../board.cpp ../move-gen.cpp
int main(int argc, char* argv[]) {
  if(argc != 4) {
    fprintf(stderr, "usage: %s <depth> <n> <ms>\n", argv[0]);
    return -1;
  }

  int depth = atoi(argv[1]);
  int n = atoi(argv[2]);
  int ms = atoi(argv[3]);

  printf("%s depth=%d n=%d ms=%d\n", argv[0], depth, n, ms);

  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  srand((unsigned int)spec.tv_nsec);
  
  usleep((rand()%ms)*1000);

  u64 last = time_ms();
  
  for(int i = 0;; i++) {
    u64 next = last + ms + (rand()%(ms/2)) - ms/4;
    u64 now = time_ms();

    if(now > next) {
      printf("-");
      fflush(stdout);
    } else {
      u64 sleep = next - now;

      if(usleep(sleep*1000)) {
	fprintf(stderr, "usleep failed errno %d: %s\n", errno, strerror(errno));
	return -3;
      }
    }
    
    last = next;

    for(int j = 0; j < n; j++ ) {
      BoardT startingBoard = Board::startingPosition();
      PerftStatsT stats = perft<StartingBoardTraitsT>(startingBoard, depth);
    }

    if(1) {
      printf(".");
      fflush(stdout);
    }
  }

  return 0;
}
