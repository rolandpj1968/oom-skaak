#include <cstdio>

#include <boost/preprocessor/iteration/local.hpp>

#include "chess.hpp"
#include "board.hpp"
#include "move-gen.hpp"

using namespace Chess;

#define BOOST_PP_LOCAL_MACRO(n) \
  (n),

#define BOOST_PP_LOCAL_LIMITS (0, 15)

int a[] = {
#include BOOST_PP_LOCAL_ITERATE()
};

using namespace MoveGen;
using namespace Board;

static void dumpAttacks(const PieceAttacksT& pieceAttacks) {
  printf("pawn attacks left:   %016lx\n", pieceAttacks.pawnsLeftAttacks);
  printf("pawn attacks right:  %016lx\n", pieceAttacks.pawnsRightAttacks);
  printf("pawn push one:       %016lx\n", pieceAttacks.pawnsPushOne);
  printf("pawn push two:       %016lx\n", pieceAttacks.pawnsPushTwo);
  printf("q rook attacks:      %016lx\n", pieceAttacks.pieceAttacks[QueenRook]);
  printf("q knight attacks:    %016lx\n", pieceAttacks.pieceAttacks[QueenKnight]);
  printf("b bishop attacks:    %016lx\n", pieceAttacks.pieceAttacks[BlackBishop]);
  printf("queen attacks:       %016lx\n", pieceAttacks.pieceAttacks[SpecificQueen]);
  printf("promo queen attacks: %016lx\n", pieceAttacks.pieceAttacks[PromoQueen]);
  printf("king attacks:        %016lx\n", pieceAttacks.pieceAttacks[SpecificKing]);
  printf("w bishop attacks:    %016lx\n", pieceAttacks.pieceAttacks[WhiteBishop]);
  printf("k knight attacks:    %016lx\n", pieceAttacks.pieceAttacks[KingKnight]);
  printf("k rook attacks:      %016lx\n", pieceAttacks.pieceAttacks[KingRook]);
      // BitBoardT pawnsRightAttacks;
      // BitBoardT pawnsPushOne;     // Not actually attacks - possibly remove
      // BitBoardT pawnsPushTwo;     // Not actually attacks - possible remove

      // // Piece moves
      // BitBoardT pieceAttacks[NSpecificPieceTypes];
      
      // // Uncommon promo piece moves - one for each pawn - one for each promo piece except 2nd queen.
      // BitBoardT promoPieceMoves[NPawns];
      
  printf("all attacks:         %016lx\n", pieceAttacks.allAttacks);
  //BitBoardT allAttacks;
}

int main(int argc, char* argv[]) {

  printf("Hallo RPJ - sizeof(Color) is %zu, f<Black> is %d, f<White> is %d\n", sizeof(ColorT), f<Black>(), f<White>());
  printf("Hallo again RPJ - sizeof(a) is %zu, sizeof(a)/sizeof(a[0]) = %zu, a[4] = %d, a[0] = %d\n", sizeof(a), sizeof(a)/sizeof(a[0]), a[4], a[0]);
  printf("H8 is %u\n", H8);

  BoardT startingBoard = Board::startingPosition();
  
  PiecesForColorT& w = startingBoard.pieces[White];
  PiecesForColorT& b = startingBoard.pieces[Black];
  
  auto whiteAttacks = genPieceAttacks<White>(w, w.bbs[AllPieces] | b.bbs[AllPieces]);

  printf("\nWhite:\n");
  dumpAttacks(whiteAttacks);

  auto blackAttacks = genPieceAttacks<Black>(b, w.bbs[AllPieces] | b.bbs[AllPieces]);

  printf("\nBlack:\n");
  dumpAttacks(blackAttacks);

  printf("\nStarting long run...\n");

  BitBoardT sumAllAttacks = 0;
  
  for(int i = 0; i < 1000; i++) {
    BoardT start = startingBoard;
  
    PiecesForColorT& w = start.pieces[White];
    PiecesForColorT& b = start.pieces[Black];

    auto whiteAttacks = genPieceAttacks<White>(w, w.bbs[AllPieces] | b.bbs[AllPieces]);
    auto blackAttacks = genPieceAttacks<Black>(b, w.bbs[AllPieces] | b.bbs[AllPieces]);

    sumAllAttacks += whiteAttacks.allAttacks + blackAttacks.allAttacks;
  }

  printf("\n... done long run\n");
  
  return 0;
}
