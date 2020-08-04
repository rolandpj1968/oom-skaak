#ifndef PAWN_MOVE_HPP
#define PAWN_MOVE_HPP

#include "types.hpp"

namespace Chess {
  
  namespace PawnMove {

    enum PawnMoveDirT {
      PushOne,
      PushTwo,
      AttackLeft,
      AttackRight
    };

    template <ColorT, PawnMoveDirT>
    struct PawnMoveTraits;

    template <> struct PawnMoveTraits<White, PushOne> {
      static const int Offset = 8;
      static const BitBoardT Mask = BbAll;
    };

    template <> struct PawnMoveTraits<Black, PushOne> {
      static const int Offset = -8;
      static const BitBoardT Mask = BbAll;
    };

    template <> struct PawnMoveTraits<White, PushTwo> {
      static const int Offset = 16;
      static const BitBoardT Mask = BbAll;
    };

    template <> struct PawnMoveTraits<Black, PushTwo> {
      static const int Offset = -16;
      static const BitBoardT Mask = BbAll;
    };

    template <> struct PawnMoveTraits<White, AttackLeft> {
      static const int Offset = 7;
      static const BitBoardT Mask = ~FileA;
    };

    template <> struct PawnMoveTraits<Black, AttackLeft> {
      static const int Offset = -9;
      static const BitBoardT Mask = ~FileA;
    };

    template <> struct PawnMoveTraits<White, AttackRight> {
      static const int Offset = 9;
      static const BitBoardT Mask = ~FileH;
    };

    template <> struct PawnMoveTraits<Black, AttackRight> {
      static const int Offset = -7;
      static const BitBoardT Mask = ~FileH;
    };

    template <ColorT Color, PawnMoveDirT PawnMoveDir>
    inline BitBoardT from2ToBb(const BitBoardT fromBb) {
      const BitBoardT maskedFromBb = fromBb & PawnMoveTraits<Color, PawnMoveDir>::Mask;
      const int offset = PawnMoveTraits<Color, PawnMoveDir>::Offset;
      // Could template specialise this to avoid the branch but compiler optimisation should do it...
      return offset > 0
	? maskedFromBb << offset
	: maskedFromBb >> -offset;
    }

    template <ColorT Color, PawnMoveDirT PawnMoveDir>
    inline BitBoardT from2ToSq(const BitBoardT fromSq) {
      const int offset = PawnMoveTraits<Color, PawnMoveDir>::Offset;
      return fromSq + offset;
    }
    
    template <ColorT Color, PawnMoveDirT PawnMoveDir>
    inline BitBoardT to2FromBb(const BitBoardT toBb) {
      const int offset = PawnMoveTraits<Color, PawnMoveDir>::Offset;
      // Could template specialise this to avoid the branch but compiler optimisation should do it...
      return offset > 0
	? toBb >> offset
	: toBb << -offset;
    }

    template <ColorT Color, PawnMoveDirT PawnMoveDir>
    inline BitBoardT to2FromSq(const BitBoardT toSq) {
      const int offset = PawnMoveTraits<Color, PawnMoveDir>::Offset;
      return toSq - offset;
    }
    
  } // namespace PawnMove
  
} // namespace Chess

#endif //PAWN_MOVE_HPP
