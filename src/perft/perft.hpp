#ifndef PERFT_HPP
#define PERFT_HPP

#include "types.hpp"
#include "board.hpp"
#include "move-gen.hpp"
#include "bits.hpp"

namespace Chess {
  using namespace MoveGen;
  
  namespace Perft {

    struct PerftStatsT {
      u64 nodes;
      u64 captures;
      u64 eps;
      u64 castles;
      u64 promos;
      u64 checks;
      // Note that 'discoverychecks' here includes checks delivered by castling, which is contraversial.
      // According to stats from: https://www.chessprogramming.org/Perft_Results
      // Would probably be better to separate this out into real discoveries and checks-from-castling
      u64 discoverychecks;
      u64 doublechecks;
      u64 checkmates;
      u64 invalids;
      u64 invalidsnon0;
      u64 non0inpinpath;
      u64 non0withdiagpins;
      u64 non0withorthogpins;
      u64 l0nondiscoveries;
      u64 l0checkertakable;
      u64 l0checkkingcanmove;
    };

    template <typename BoardTraitsT>
    inline PerftStatsT perft(const BoardT& board, const int depthToGo);
    
    template <typename BoardTraitsT>
    inline void perftImpl(PerftStatsT& stats, const BoardT& board, const int depthToGo, const MoveInfoT moveInfo);
  
    template <typename BoardTraitsT, typename To2FromFn, bool IsPushTwo>
    inline void perftImplPawnsPush(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPush) {
      // No captures here - these are just pawn pushes and already filtered from all target clashes.
      while(pawnsPush) {
	const SquareT to = Bits::popLsb(pawnsPush);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = moveSpecificPiece<BoardTraitsT::Color, SpecificPawn, Push, IsPushTwo>(board, from, to);

	perftImpl<typename BoardTraitsT::ReverseT>(stats, newBoard, depthToGo-1, MoveInfoT(PushMove, to));
      }
    }

    template <typename BoardTraitsT>
    inline void perftImplPawnsPushOne(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPushOne) {
      perftImplPawnsPush<BoardTraitsT, PawnPushOneTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/false>(stats, board, depthToGo, pawnsPushOne);
    }
    
    template <typename BoardTraitsT>
    inline void perftImplPawnsPushTwo(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsPushTwo) {
      perftImplPawnsPush<BoardTraitsT, PawnPushTwoTo2FromFn<BoardTraitsT::Color>, /*IsPushTwo =*/true>(stats, board, depthToGo, pawnsPushTwo);
    }

    template <ColorT Color>
    inline SquareT pawnAttackLeftTo2From(const SquareT square);
    template <> inline SquareT pawnAttackLeftTo2From<White>(const SquareT square) { return square - 7; }
    template <> inline SquareT pawnAttackLeftTo2From<Black>(const SquareT square) { return square + 9; }

    template <ColorT Color>
    struct PawnAttackLeftTo2FromFn {
      static inline SquareT fn(const SquareT from) { return pawnAttackLeftTo2From<Color>(from); }
    };

    template <ColorT Color>
    inline SquareT pawnAttackRightTo2From(const SquareT square);
    template <> inline SquareT pawnAttackRightTo2From<White>(const SquareT square) { return square - 9; }
    template <> inline SquareT pawnAttackRightTo2From<Black>(const SquareT square) { return square + 7; }

    template <ColorT Color>
    struct PawnAttackRightTo2FromFn {
      static inline SquareT fn(const SquareT from) { return pawnAttackRightTo2From<Color>(from); }
    };

    template <typename BoardTraitsT, typename To2FromFn>
    inline void perftImplPawnsCapture(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCapture) {
      while(pawnsCapture) {
	const SquareT to = Bits::popLsb(pawnsCapture);
	const SquareT from = To2FromFn::fn(to);

	const BoardT newBoard = moveSpecificPiece<BoardTraitsT::Color, SpecificPawn, Capture>(board, from, to);

	perftImpl<typename BoardTraitsT::ReverseT>(stats, newBoard, depthToGo-1, MoveInfoT(CaptureMove, to));
      }
    }

    template <typename BoardTraitsT>
    inline void perftImplPawnsCaptureLeft(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureLeft) {
      perftImplPawnsCapture<BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(stats, board, depthToGo, pawnsCaptureLeft);
    }
    
    template <typename BoardTraitsT>
    inline void perftImplPawnsCaptureRight(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureRight) {
      perftImplPawnsCapture<BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(stats, board, depthToGo, pawnsCaptureRight);
    }

    template <typename BoardTraitsT, typename To2FromFn>
    inline void perftImplPawnEpCapture(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsEpCaptureBb) {
      // There can be only 1 en-passant capture, so no need to loop
      if(pawnsEpCaptureBb) {
	const SquareT to = Bits::lsb(pawnsEpCaptureBb);
	const SquareT from = To2FromFn::fn(to);
	const SquareT captureSq = pawnPushOneTo2From<BoardTraitsT::Color>(to);

	const BoardT newBoard = captureEp<BoardTraitsT::Color>(board, from, to, captureSq);

	perftImpl<typename BoardTraitsT::ReverseT>(stats, newBoard, depthToGo-1, MoveInfoT(EpCaptureMove, to));
      }
    }

    template <typename BoardTraitsT>
    inline void perftImplPawnEpCaptureLeft(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureLeft) {
      perftImplPawnEpCapture<BoardTraitsT, PawnAttackLeftTo2FromFn<BoardTraitsT::Color>>(stats, board, depthToGo, pawnsCaptureLeft);
    }
    
    template <typename BoardTraitsT>
    inline void perftImplPawnEpCaptureRight(PerftStatsT& stats, const BoardT& board, const int depthToGo, BitBoardT pawnsCaptureRight) {
      perftImplPawnEpCapture<BoardTraitsT, PawnAttackRightTo2FromFn<BoardTraitsT::Color>>(stats, board, depthToGo, pawnsCaptureRight);
    }

    // template <typename BoardTraitsT>
    // inline void perftImplPawnEpMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, SquareT epSquare, const BitBoardT allPiecesBb, const BitBoardT legalPawnsLeftBb, const BitBoardT legalPawnsRightBb) {
    //   const BitBoardT epSquareBb = bbForSquare(epSquare);

    //   const BitBoardT semiLegalEpCaptureLeftBb = legalPawnsLeftBb & epSquareBb;
    //   const BitBoardT semiLegalEpCaptureRightBb = legalPawnsRightBb & epSquareBb;

    //   // Only do the heavy lifting of detecting discovered check through the captured pawn if there really is an en-passant opportunity
    //   if((semiLegalEpCaptureLeftBb | semiLegalEpCaptureRightBb) != BbNone) {
    // 	const ColorStateT& yourState = board.pieces[BoardTraitsT::OtherColor];
    // 	const ColorStateT& myState = board.pieces[BoardTraitsT::Color];
    // 	const SquareT myKingSq = myState.pieceSquares[SpecificKing];
	  
    // 	const SquareT to = Bits::lsb(semiLegalEpCaptureLeftBb | semiLegalEpCaptureRightBb);
    // 	const SquareT captureSq = pawnPushOneTo2From<BoardTraitsT::Color>(to);
    // 	const BitBoardT captureSquareBb = bbForSquare(captureSq);

    // 	// Note that a discovered check can only be diagonal or horizontal, because the capturing pawn ends up on the same file as the captured pawn.
    // 	const BitBoardT diagPinnedEpPawnBb = genPinnedPiecesBb<Diagonal>(myKingSq, allPiecesBb, captureSquareBb, yourState);
    // 	// Horizontal is really tricky because it involves both capturing and captured pawn.
    // 	// We detect it by removing them both and looking for a king attack - could optimise this... TODO anyhow
    // 	const BitBoardT orthogPinnedEpPawnBb = BbNone; //genPinnedPiecesBb<Orthogonal>(myKingSq, allPiecesBb, captureSquareBb, yourState);

    // 	if((diagPinnedEpPawnBb | orthogPinnedEpPawnBb) != BbNone) {
    // 	  static bool done = false;
    // 	  if(!done) {
    // 	    printf("\n============================================== EP avoidance EP square is %d ===================================\n\n", epSquare);
    // 	    printBoard(board);
    // 	    printf("\n");
    // 	    done = true;
    // 	  }
    // 	}
	
    // 	if((diagPinnedEpPawnBb | orthogPinnedEpPawnBb) == BbNone) {
    // 	  perftImplPawnEpCaptureLeft<BoardTraitsT>(stats, board, depthToGo, semiLegalEpCaptureLeftBb);
    // 	  perftImplPawnEpCaptureRight<BoardTraitsT>(stats, board, depthToGo, semiLegalEpCaptureRightBb);
    // 	}
    //   }
    // }
    
    // template <typename BoardTraitsT>
    // inline void perftImplPawnMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const PieceAttacksT& myAttacks, SquareT epSquare, const BitBoardT allYourPiecesBb, const BitBoardT allPiecesBb, const BitBoardT legalMoveMaskBb, const PiecePinMasksT& pinMasks) {

    //   // Pawn pushes
	
    //   const BitBoardT legalPawnsPushOneBb = myAttacks.pawnsPushOne & legalMoveMaskBb & pinMasks.pawnsPushOnePinMask;
    //   perftImplPawnsPushOne<BoardTraitsT>(stats, board, depthToGo, legalPawnsPushOneBb);

    //   const BitBoardT legalPawnsPushTwoBb = myAttacks.pawnsPushTwo & legalMoveMaskBb & pinMasks.pawnsPushTwoPinMask;
    //   perftImplPawnsPushTwo<BoardTraitsT>(stats, board, depthToGo, legalPawnsPushTwoBb);
	
    //   // Pawn captures

    //   const BitBoardT legalPawnsLeftBb = myAttacks.pawnsLeftAttacks & legalMoveMaskBb & pinMasks.pawnsLeftPinMask;
    //   perftImplPawnsCaptureLeft<BoardTraitsT>(stats, board, depthToGo, legalPawnsLeftBb & allYourPiecesBb);
      
    //   const BitBoardT legalPawnsRightBb = myAttacks.pawnsRightAttacks & legalMoveMaskBb & pinMasks.pawnsRightPinMask;
    //   perftImplPawnsCaptureRight<BoardTraitsT>(stats, board, depthToGo, legalPawnsRightBb & allYourPiecesBb);
      
    //   // Pawn en-passant captures
    //   // En-passant is tricky because the captured pawn is not on the same square as the capturing piece, and might expose a discovered check itself.
    //   if(epSquare != InvalidSquare) {
    // 	perftImplPawnEpMoves<BoardTraitsT>(stats, board, depthToGo, epSquare, allPiecesBb, legalPawnsLeftBb, legalPawnsRightBb);
    //   }
    // }
    
    template <typename BoardTraitsT>
    inline void perftImplPawnMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const PawnPushesAndCapturesT& pawnMoves) {

      // Pawn pushes
      perftImplPawnsPushOne<BoardTraitsT>(stats, board, depthToGo, pawnMoves.pushesOneBb);
      perftImplPawnsPushTwo<BoardTraitsT>(stats, board, depthToGo, pawnMoves.pushesTwoBb);
	
      // Pawn captures
      perftImplPawnsCaptureLeft<BoardTraitsT>(stats, board, depthToGo, pawnMoves.capturesLeftBb);
      perftImplPawnsCaptureRight<BoardTraitsT>(stats, board, depthToGo, pawnMoves.capturesRightBb);
      
      // Pawn en-passant captures
      perftImplPawnEpCaptureLeft<BoardTraitsT>(stats, board, depthToGo, pawnMoves.epCaptures.epLeftCaptureBb);
      perftImplPawnEpCaptureRight<BoardTraitsT>(stats, board, depthToGo, pawnMoves.epCaptures.epRightCaptureBb);
    }
    
    template <typename BoardTraitsT, SpecificPieceT SpecificPiece, PushOrCaptureT PushOrCapture>
    inline void perftImplSpecificPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, BitBoardT toBb, const MoveTypeT moveType) {
      while(toBb) {
	const SquareT to = Bits::popLsb(toBb);

	const BoardT newBoard = moveSpecificPiece<BoardTraitsT::Color, SpecificPiece, PushOrCapture>(board, from, to);

	perftImpl<typename BoardTraitsT::ReverseT>(stats, newBoard, depthToGo-1, MoveInfoT(moveType, to));
      }
    }
    
    template <typename BoardTraitsT, SpecificPieceT SpecificPiece>
    inline void perftImplSpecificPiecePushes(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, const BitBoardT pushesBb) {
      perftImplSpecificPieceMoves<BoardTraitsT, SpecificPiece, Push>(stats, board, depthToGo, from, pushesBb, PushMove);
    }
    
    template <typename BoardTraitsT, SpecificPieceT SpecificPiece>
    inline void perftImplSpecificPieceCaptures(PerftStatsT& stats, const BoardT& board, const int depthToGo, const SquareT from, const BitBoardT capturesBb) {
      perftImplSpecificPieceMoves<BoardTraitsT, SpecificPiece, Capture>(stats, board, depthToGo, from, capturesBb, CaptureMove);
    }

    template <typename BoardTraitsT, SpecificPieceT SpecificPiece>
    inline void perftImplSpecificPieceMoves(PerftStatsT& stats, const BoardT& board, const int depthToGo, const PushesAndCapturesT pushesAndCaptures) {
      const ColorStateT& myState = board.pieces[(size_t)BoardTraitsT::Color];
      const SquareT from = myState.pieceSquares[SpecificPiece];

      perftImplSpecificPiecePushes<BoardTraitsT, SpecificPiece>(stats, board, depthToGo, from, pushesAndCaptures.pushesBb);
      
      perftImplSpecificPieceCaptures<BoardTraitsT, SpecificPiece>(stats, board, depthToGo, from, pushesAndCaptures.capturesBb);
    }
    
    template <typename BoardTraitsT, CastlingRightsT CastlingRight>
    inline void perftImplCastlingMove(PerftStatsT& stats, const BoardT& board, const int depthToGo) {
      const ColorT Color = BoardTraitsT::Color;
      
      const BoardT newBoard1 = moveSpecificPiece<Color, SpecificKing, Push>(board, CastlingTraitsT<Color, CastlingRight>::KingFrom, CastlingTraitsT<Color, CastlingRight>::KingTo);
      const BoardT newBoard = moveSpecificPiece<Color, CastlingTraitsT<Color, CastlingRight>::SpecificRook, Push>(newBoard1, CastlingTraitsT<Color, CastlingRight>::RookFrom, CastlingTraitsT<Color, CastlingRight>::RookTo);

      // We pass the rook 'to' square cos we use it for discovered check and check from castling is not considered 'discovered'
      // Or maybe not - getting wrong discoveries count compared to wiki lore - let's try the king instead.
      perftImpl<typename BoardTraitsT::ReverseT>(stats, newBoard, depthToGo-1, MoveInfoT(CastlingMove, CastlingTraitsT<Color, CastlingRight>::KingTo));
    }

    template <typename BoardTraitsT>
    inline void perft0Impl(PerftStatsT& stats, const BoardT& board, const MoveInfoT moveInfo) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      typedef typename BoardTraitsT::YourColorTraitsT YourColorTraitsT;
      const ColorT Color = BoardTraitsT::Color;
      const ColorT OtherColor = BoardTraitsT::OtherColor;

      const ColorStateT& myState = board.pieces[(size_t)Color];
      const ColorStateT& yourState = board.pieces[(size_t)OtherColor];
      const BitBoardT allMyPiecesBb = myState.bbs[AllPieces];
      const BitBoardT allYourPiecesBb = yourState.bbs[AllPieces];
      const BitBoardT allPiecesBb = allMyPiecesBb | allYourPiecesBb;

      // It is strictly a bug if we encounter an invalid position - we are doing legal (only) move evaluation.
      const bool CheckForInvalid = false;
      if(CheckForInvalid) {
	// Is your king in check? If so we got here via an illegal move of the pseudo-move-generator
	const SquareAttackersT yourKingAttackers = genSquareAttackers<MyColorTraitsT>(yourState.pieceSquares[SpecificKing], myState, allPiecesBb);
	if(yourKingAttackers.pieceAttackers[AllPieces] != 0) {
	  // Illegal position - doesn't count
	  stats.invalids++;
	  static bool done = false;
	  if(!done) {
	    printf("\n============================================== Invalid Depth 0 - last move to %d! ===================================\n\n", moveInfo.to);
	    printBoard(board);
	    printf("\n");
	    done = true;
	  }
	  return;
	}
      }
	
      stats.nodes++;

      if(moveInfo.moveType == CaptureMove) {
	stats.captures++;
      }

      if(moveInfo.moveType == EpCaptureMove) {
	stats.captures++;
	stats.eps++;
      }

      if(moveInfo.moveType == CastlingMove) {
	stats.castles++;
      }

      const bool DoCheckStats = true;
      const bool DoCheckMateStats = true;
      if(!DoCheckStats) {
	return;
      }

      // Is my king in check?
      const SquareAttackersT myKingAttackers = genSquareAttackers<YourColorTraitsT>(myState.pieceSquares[SpecificKing], yourState, allPiecesBb);
      const BitBoardT allMyKingAttackers = myKingAttackers.pieceAttackers[AllPieces];
      if(allMyKingAttackers != 0) {
	stats.checks++;

	// If the moved piece is not attacking the king then this is a discovered check
	bool isDiscovery = false;
	if((bbForSquare(moveInfo.to) & allMyKingAttackers) == 0) {
	  isDiscovery = true;
	  stats.discoverychecks++;
	}
	  
	// If there are multiple king attackers then we have a double check
	bool isDoubleCheck = false;
	if(Bits::count(allMyKingAttackers) != 1) {
	  isDoubleCheck = true;
	  stats.doublechecks++;
	}

	if(DoCheckMateStats) {
	  bool isPossibleCheckmate = true;
	  // If it's a non-discovery and we can take the checker then it's not checkmate
	  if(isPossibleCheckmate && !isDiscovery && !isDoubleCheck) {
	    // See if the checking piece can be taken
	    stats.l0nondiscoveries++;
	    const SquareAttackersT checkerAttackers = genSquareAttackers<MyColorTraitsT>(moveInfo.to, myState, allPiecesBb);
	    // It's not safe for the king to capture cos he might still be in check
	    if(checkerAttackers.pieceAttackers[AllPieces] &~ checkerAttackers.pieceAttackers[SpecificKing]) {
	      stats.l0checkertakable++;
	      isPossibleCheckmate = false;
	    }
	  }
	  // It's not checkmate if the king can move to safety
	  if(isPossibleCheckmate) {
	    // Remove my king to expose moving away from a slider checker
	    const PieceAttacksT yourAttacks = genPieceAttacks<YourColorTraitsT>(yourState, allPiecesBb & ~myState.bbs[King]);
	    const SquareT myKingSq = myState.pieceSquares[SpecificKing];
	    const BitBoardT myKingAttacksBb = KingAttacks[myKingSq];
	    const BitBoardT myKingMovesBb = myKingAttacksBb & ~allMyPiecesBb;
	    if(myKingMovesBb & ~yourAttacks.allAttacks) {
	      stats.l0checkkingcanmove++;
	      isPossibleCheckmate = false;
	    }
	  }
	  // It's checkmate if there are no valid child nodes.
	  if(isPossibleCheckmate) {
	    PerftStatsT childStats = perft<BoardTraitsT>(board, 1);
	    if(childStats.nodes == 0) {
	      stats.checkmates++;
	    }
	  }
	}
      }
    }

    template <typename BoardTraitsT>
    inline void perftImplFull(PerftStatsT& stats, const BoardT& board, const int depthToGo, const MoveInfoT moveInfo) {
      typedef typename BoardTraitsT::MyColorTraitsT MyColorTraitsT;
      
      // Generate (legal) moves
      const LegalMovesT legalMoves = genLegalMoves<BoardTraitsT>(board);

      // Is this an illegal pos - note this should never happen(tm)
      if(legalMoves.isIllegalPos) {
	// Illegal position - don't count
	stats.invalidsnon0++;
	static bool done = false;
	if(!done) {
	  printf("\n============================================== Invalid - last move to %d! ===================================\n\n", moveInfo.to);
	  printBoard(board);
	  printf("\n");
	  done = true;
	}
	return;
      }
      
      // Double check can only be evaded by moving the king so only bother with other pieces if nChecks < 2
      if(legalMoves.nChecks < 2) {

	// Evaluate moves

	// Pawns
	
	//perftImplPawnMoves<BoardTraitsT>(stats, board, depthToGo, myAttacks, yourState.epSquare, allYourPiecesBb, allPiecesBb, legalMoveMaskBb, pinMasks);
	perftImplPawnMoves<BoardTraitsT>(stats, board, depthToGo, legalMoves.pawnMoves);
	
	// Knights

	perftImplSpecificPieceMoves<BoardTraitsT, QueenKnight>(stats, board, depthToGo, legalMoves.specificPieceMoves[QueenKnight]);

	perftImplSpecificPieceMoves<BoardTraitsT, KingKnight>(stats, board, depthToGo, legalMoves.specificPieceMoves[KingKnight]);
	
	// Bishops
      
	perftImplSpecificPieceMoves<BoardTraitsT, BlackBishop>(stats, board, depthToGo, legalMoves.specificPieceMoves[BlackBishop]); 

	perftImplSpecificPieceMoves<BoardTraitsT, WhiteBishop>(stats, board, depthToGo, legalMoves.specificPieceMoves[WhiteBishop]); 

	// Rooks

	perftImplSpecificPieceMoves<BoardTraitsT, QueenRook>(stats, board, depthToGo, legalMoves.specificPieceMoves[QueenRook]); 

	perftImplSpecificPieceMoves<BoardTraitsT, KingRook>(stats, board, depthToGo, legalMoves.specificPieceMoves[KingRook]); 

	// Queens

	perftImplSpecificPieceMoves<BoardTraitsT, SpecificQueen>(stats, board, depthToGo, legalMoves.specificPieceMoves[SpecificQueen]); 

	// TODO other promo pieces
	if(MyColorTraitsT::HasPromos) {
	  if(true/*myState.piecesPresent & PromoQueenPresentFlag*/) {
	    perftImplSpecificPieceMoves<BoardTraitsT, PromoQueen>(stats, board, depthToGo, legalMoves.specificPieceMoves[PromoQueen]);  
	  }
	}

	// Castling
	CastlingRightsT canCastleFlags = legalMoves.canCastleFlags;
	if(canCastleFlags) {

	  if((canCastleFlags & CanCastleQueenside)) {
	    perftImplCastlingMove<BoardTraitsT, CanCastleQueenside>(stats, board, depthToGo);
	  }

	  if((canCastleFlags & CanCastleKingside)) {
	    perftImplCastlingMove<BoardTraitsT, CanCastleKingside>(stats, board, depthToGo);
	  }	
	}

      } // nChecks < 2
      
      // King
      perftImplSpecificPieceMoves<BoardTraitsT, SpecificKing>(stats, board, depthToGo, legalMoves.specificPieceMoves[SpecificKing]); 


    }

    template <typename BoardTraitsT>
    inline void perftImpl(PerftStatsT& stats, const BoardT& board, const int depthToGo, const MoveInfoT moveInfo) {
      // If this is a leaf node, gather stats.
      if(depthToGo == 0) {
	perft0Impl<BoardTraitsT>(stats, board, moveInfo);
      } else {
	perftImplFull<BoardTraitsT>(stats, board, depthToGo, moveInfo);
      }
    }
      
    template <typename BoardTraitsT> inline PerftStatsT perft(const BoardT& board, const int depthToGo) {
      PerftStatsT stats = {0};

      perftImpl<BoardTraitsT>(stats, board, depthToGo, MoveInfoT(PushMove, InvalidSquare));

      return stats;
    }

  } // namespace Perf
} // namespace Chess

#endif //ndef PERFT_HPP
