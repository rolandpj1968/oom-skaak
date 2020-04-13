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

static void dumpAttacks(const PieceAttacksT& pieceAttacks) {
  printf("pawn attacks left:   %016lx\n", pieceAttacks.pawnsLeftAttacks);
  printf("pawn attacks right:  %016lx\n", pieceAttacks.pawnsRightAttacks);
  printf("pawn push one:       %016lx\n", pieceAttacks.pawnsPushOne);
  printf("pawn push two:       %016lx\n", pieceAttacks.pawnsPushTwo);
  printf("q rook attacks:      %016lx\n", pieceAttacks.pieceAttacks[QueenRook]);
  printf("q knight attacks:    %016lx\n", pieceAttacks.pieceAttacks[Knight1]);
  printf("b bishop attacks:    %016lx\n", pieceAttacks.pieceAttacks[BlackBishop]);
  printf("queen attacks:       %016lx\n", pieceAttacks.pieceAttacks[TheQueen]);
  printf("king attacks:        %016lx\n", pieceAttacks.pieceAttacks[TheKing]);
  printf("w bishop attacks:    %016lx\n", pieceAttacks.pieceAttacks[WhiteBishop]);
  printf("k knight attacks:    %016lx\n", pieceAttacks.pieceAttacks[Knight2]);
  printf("k rook attacks:      %016lx\n", pieceAttacks.pieceAttacks[KingRook]);
      
  printf("all attacks:         %016lx\n", pieceAttacks.allAttacks);
}

int main(int argc, char* argv[]) {

  printf("Hallo RPJ - sizeof(Color) is %zu\n", sizeof(ColorT));
  printf("Hallo again RPJ - sizeof(a) is %zu, sizeof(a)/sizeof(a[0]) = %zu, a[4] = %d, a[0] = %d\n", sizeof(a), sizeof(a)/sizeof(a[0]), a[4], a[0]);
  printf("H8 is %u\n", H8);

  BoardT board;

  if(argc > 1) {
    board = parseFen(argv[1]).first;
  } else {
    board = Board::startingPosition();
  }

  printBoard(board);

  return 0;
  
  ColorStateT& w = board.pieces[(size_t)White];
  ColorStateT& b = board.pieces[(size_t)Black];

  const PieceBbsT& pieceBbs = genPieceBbs<StartingBoardTraitsT>(board);

  const ColorPieceBbsT& wPieceBbs = pieceBbs.colorPieceBbs[(size_t)White];
  const ColorPieceBbsT& bPieceBbs = pieceBbs.colorPieceBbs[(size_t)Black];
  
  const BitBoardT allWPiecesBb = wPieceBbs.bbs[AllPieceTypes];
  const BitBoardT allBPiecesBb = bPieceBbs.bbs[AllPieceTypes];
  const BitBoardT allPiecesBb = allWPiecesBb | allBPiecesBb;
  
  auto whiteAttacks = genPieceAttacks<WhiteStartingColorTraitsT>(w, allPiecesBb);

  printf("\nWhite:\n");
  dumpAttacks(whiteAttacks);

  printf("\n%d attacks, %d valid moves, all white pieces %016lx\n", countAttacks(whiteAttacks), countAttacks(whiteAttacks, allWPiecesBb, allBPiecesBb), allPiecesBb);
  
  auto blackAttacks = genPieceAttacks<BlackStartingColorTraitsT>(b, allPiecesBb);

  printf("\nBlack:\n");
  dumpAttacks(blackAttacks);

  printf("\n%d attacks, %d valid moves, all black pieces %016lx\n\n", countAttacks(blackAttacks), countAttacks(blackAttacks, allBPiecesBb, allWPiecesBb), allPiecesBb);
  
  return 0;
}
