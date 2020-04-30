#include <cstdio>

#include <boost/preprocessor/iteration/local.hpp>

#include "board.hpp"
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
using namespace Fen;
using namespace MoveGen;

template <typename BoardT>
static void dumpAttacks(const typename PieceAttackBbsImplType<BoardT>::PieceAttackBbsT& pieceAttackBbs) {
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

int main(int argc, char* argv[]) {

  printf("Hallo RPJ - sizeof(Color) is %zu\n", sizeof(ColorT));
  printf("Hallo again RPJ - sizeof(a) is %zu, sizeof(a)/sizeof(a[0]) = %zu, a[4] = %d, a[0] = %d\n", sizeof(a), sizeof(a)/sizeof(a[0]), a[4], a[0]);
  printf("H8 is %u\n", H8);

  typedef SimpleBoardT BoardT;
  typedef typename BoardT::ColorStateT ColorStateT;
  typedef typename PieceBbsImplType<BoardT>::PieceBbsT PieceBbsT;
  typedef typename ColorPieceBbsImplType<BoardT>::ColorPieceBbsT ColorPieceBbsT;
  
  BoardT board;

  if(argc > 1) {
    board = parseFen(argv[1]).first;
  } else {
    board = Board::startingPosition();
  }

  printBoard(board);

  return 0;
  
  ColorStateT& w = board.state[(size_t)White];
  ColorStateT& b = board.state[(size_t)Black];

  const PieceBbsT& pieceBbs = genPieceBbs<BoardT, White>(board);

  const ColorPieceBbsT& wPieceBbs = pieceBbs.colorPieceBbs[(size_t)White];
  const ColorPieceBbsT& bPieceBbs = pieceBbs.colorPieceBbs[(size_t)Black];
  
  const BitBoardT allWPiecesBb = wPieceBbs.bbs[AllPieceTypes];
  const BitBoardT allBPiecesBb = bPieceBbs.bbs[AllPieceTypes];
  const BitBoardT allPiecesBb = allWPiecesBb | allBPiecesBb;
  
  auto whiteAttacks = genPieceAttackBbs<BoardT, White>(w, allPiecesBb);

  printf("\nWhite:\n");
  dumpAttacks<BoardT>(whiteAttacks);

  printf("\n%d attacks, %d valid moves, all white pieces %016lx\n", countAttacks<BoardT>(whiteAttacks), countAttacks<BoardT>(whiteAttacks, allWPiecesBb, allBPiecesBb), allPiecesBb);
  
  auto blackAttacks = genPieceAttackBbs<BoardT, Black>(b, allPiecesBb);

  printf("\nBlack:\n");
  dumpAttacks<BoardT>(blackAttacks);

  printf("\n%d attacks, %d valid moves, all black pieces %016lx\n\n", countAttacks<BoardT>(blackAttacks), countAttacks<BoardT>(blackAttacks, allBPiecesBb, allWPiecesBb), allPiecesBb);
  
  return 0;
}
