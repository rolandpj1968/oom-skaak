#include <cstdio>
#include <string>

#include <boost/preprocessor/iteration/local.hpp>

#include "board.hpp"
#include "board-utils.hpp"
#include "bounded-hash-map.hpp"
#include "fen.hpp"
#include "move-gen.hpp"

using namespace Chess;

#define BOOST_PP_LOCAL_MACRO(n) \
  (n),

#define BOOST_PP_LOCAL_LIMITS (0, 15)

int a[] = {
#include BOOST_PP_LOCAL_ITERATE()
};

using namespace Board;

int main(int argc, char* argv[]) {
  auto board = Fen::parseFen("rnbqkb1r/pppppp1p/7n/6pP/8/8/PPPPPPP1/RNBQKBNR w KQkq g6").first;

  BoardUtils::printBoard(board);

  printf("\nFen: %s\n\nFen2: %s\n\n", Fen::toFen(board, White).c_str(), Fen::toFen(board, White, true).c_str());

  return 0;
}

template <typename BoardT>
static void dumpAttacks(const typename MoveGen::PieceAttackBbsImplType<BoardT>::PieceAttackBbsT& pieceAttackBbs) {
  printf("pawn attacks left:   %016lx\n", pieceAttackBbs.pawnsLeftAttacksBb);
  printf("pawn attacks right:  %016lx\n", pieceAttackBbs.pawnsRightAttacksBb);
  printf("pawn push one:       %016lx\n", pieceAttackBbs.pawnsPushOneBb);
  printf("pawn push two:       %016lx\n", pieceAttackBbs.pawnsPushTwoBb);
  printf("q rook attacks:      %016lx\n", pieceAttackBbs.pieceAttackBbs[Rook1]);
  printf("q knight attacks:    %016lx\n", pieceAttackBbs.pieceAttackBbs[Knight1]);
  printf("b bishop attacks:    %016lx\n", pieceAttackBbs.pieceAttackBbs[Bishop1]);
  printf("queen attacks:       %016lx\n", pieceAttackBbs.pieceAttackBbs[TheQueen]);
  printf("king attacks:        %016lx\n", pieceAttackBbs.pieceAttackBbs[TheKing]);
  printf("w bishop attacks:    %016lx\n", pieceAttackBbs.pieceAttackBbs[Bishop2]);
  printf("k knight attacks:    %016lx\n", pieceAttackBbs.pieceAttackBbs[Knight2]);
  printf("k rook attacks:      %016lx\n", pieceAttackBbs.pieceAttackBbs[Rook2]);
      
  printf("all attacks:         %016lx\n", pieceAttackBbs.allAttacksBb);
}

int main1(int argc, char* argv[]) {

  printf("Hallo RPJ - sizeof(Color) is %zu\n", sizeof(ColorT));
  printf("Hallo again RPJ - sizeof(a) is %zu, sizeof(a)/sizeof(a[0]) = %zu, a[4] = %d, a[0] = %d\n", sizeof(a), sizeof(a)/sizeof(a[0]), a[4], a[0]);
  printf("H8 is %u\n", H8);

  typedef BasicBoardT BoardT;
  typedef typename BoardT::ColorStateT ColorStateT;
  typedef typename MoveGen::PieceBbsImplType<BoardT>::PieceBbsT PieceBbsT;
  typedef typename MoveGen::ColorPieceBbsImplType<BoardT>::ColorPieceBbsT ColorPieceBbsT;
  
  BoardT board;

  if(argc > 1) {
    board = Fen::parseFen(argv[1]).first;
  } else {
    board = BoardUtils::startingPosition();
  }

  BoardUtils::printBoard(board);

  return 0;
  
  ColorStateT& w = board.state[(size_t)White];
  ColorStateT& b = board.state[(size_t)Black];

  const PieceBbsT& pieceBbs = MoveGen::genPieceBbs<BoardT, White>(board);

  const ColorPieceBbsT& wPieceBbs = pieceBbs.colorPieceBbs[(size_t)White];
  const ColorPieceBbsT& bPieceBbs = pieceBbs.colorPieceBbs[(size_t)Black];
  
  const BitBoardT allWPiecesBb = wPieceBbs.bbs[AllPieceTypes];
  const BitBoardT allBPiecesBb = bPieceBbs.bbs[AllPieceTypes];
  const BitBoardT allPiecesBb = allWPiecesBb | allBPiecesBb;
  
  auto whiteAttacks = MoveGen::genPieceAttackBbs<BoardT, White>(w, allPiecesBb);

  printf("\nWhite:\n");
  dumpAttacks<BoardT>(whiteAttacks);

  printf("\n%d attacks, %d valid moves, all white pieces %016lx\n", BoardUtils::countAttacks<BoardT>(whiteAttacks), BoardUtils::countAttacks<BoardT>(whiteAttacks, allWPiecesBb, allBPiecesBb), allPiecesBb);
  
  auto blackAttacks = MoveGen::genPieceAttackBbs<BoardT, Black>(b, allPiecesBb);

  printf("\nBlack:\n");
  dumpAttacks<BoardT>(blackAttacks);

  printf("\n%d attacks, %d valid moves, all black pieces %016lx\n\n", BoardUtils::countAttacks<BoardT>(blackAttacks), BoardUtils::countAttacks<BoardT>(blackAttacks, allBPiecesBb, allWPiecesBb), allPiecesBb);
  
  return 0;
}

int main2(int argc, char* argv[]) {
  using BoundedHashMap::BoundedHashMap;

  const int MAX_SIZE = 4;
  BoundedHashMap<std::string, int> map(MAX_SIZE);

  printf("bounded map: empty() is %d, size() is %lu, max_size is %lu\n", map.empty(), map.size(), map.max_size());

  printf("mru list: {");
  for(auto it = map.mru.begin(); it != map.mru.end(); ++it) {
    printf(" %s", it->c_str());
  }
  printf(" }\n");

  std::string one("1");
  map.put(one, 1);

  printf("bounded map: empty() is %d, size() is %lu, max_size is %lu\n", map.empty(), map.size(), map.max_size());

  printf("mru list: {");
  for(auto it = map.mru.begin(); it != map.mru.end(); ++it) {
    printf(" %s", it->c_str());
  }
  printf(" }\n");

  std::string two("2");
  map.put(two, 2);

  printf("bounded map: empty() is %d, size() is %lu, max_size is %lu\n", map.empty(), map.size(), map.max_size());

  printf("mru list: {");
  for(auto it = map.mru.begin(); it != map.mru.end(); ++it) {
    printf(" %s", it->c_str());
  }
  printf(" }\n");

  std::string three("3");
  map.put(three, 3);

  printf("bounded map: empty() is %d, size() is %lu, max_size is %lu\n", map.empty(), map.size(), map.max_size());

  printf("mru list: {");
  for(auto it = map.mru.begin(); it != map.mru.end(); ++it) {
    printf(" %s", it->c_str());
  }
  printf(" }\n");

  std::string four("4");
  map.put(four, 4);

  printf("bounded map: empty() is %d, size() is %lu, max_size is %lu\n", map.empty(), map.size(), map.max_size());

  printf("mru list: {");
  for(auto it = map.mru.begin(); it != map.mru.end(); ++it) {
    printf(" %s", it->c_str());
  }
  printf(" }\n");

  std::string five("5");
  map.put(five, 5);

  printf("bounded map: empty() is %d, size() is %lu, max_size is %lu\n", map.empty(), map.size(), map.max_size());

  printf("mru list: {");
  for(auto it = map.mru.begin(); it != map.mru.end(); ++it) {
    printf(" %s", it->c_str());
  }
  printf(" }\n");

  printf("map.at(three) is %d\n", map.at(three));
  printf("mru list: {");
  for(auto it = map.mru.begin(); it != map.mru.end(); ++it) {
    printf(" %s", it->c_str());
  }
  printf(" }\n");

  printf("map.at(two) is %d\n", map.at(two));
  printf("mru list: {");
  for(auto it = map.mru.begin(); it != map.mru.end(); ++it) {
    printf(" %s", it->c_str());
  }
  printf(" }\n");

  std::string six("6");
  map.put(six, 6);

  printf("bounded map: empty() is %d, size() is %lu, max_size is %lu\n", map.empty(), map.size(), map.max_size());

  printf("mru list: {");
  for(auto it = map.mru.begin(); it != map.mru.end(); ++it) {
    printf(" %s", it->c_str());
  }
  printf(" }\n");

  return 0;
}
