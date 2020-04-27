#include <algorithm>
#include <array>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cstdio>

#include "board.hpp"
#include "fen.hpp"

// Split a string on ' '-delimiter
static std::vector<std::string> split(const std::string& s) {
  std::istringstream iss(s);
  std::vector<std::string> results(std::istream_iterator<std::string>{iss},
				   std::istream_iterator<std::string>());
  return results;
}

using Chess::Board::BoardT;

namespace Chess {

  using namespace Board;
  
  namespace Fen {

    struct PieceTypeAndColorT {
      const PieceTypeT pieceType;
      const ColorT color;

      PieceTypeAndColorT(const PieceTypeT pieceType, const ColorT color):
	pieceType(pieceType), color(color) {}
    };

    static const std::map<char, PieceTypeAndColorT> FenPieces = {
      { 'P', PieceTypeAndColorT(Pawn, White) },
      { 'N', PieceTypeAndColorT(Knight, White) },
      { 'B', PieceTypeAndColorT(Bishop, White) },
      { 'R', PieceTypeAndColorT(Rook, White) },
      { 'Q', PieceTypeAndColorT(Queen, White) },
      { 'K', PieceTypeAndColorT(King, White) },
      { 'p', PieceTypeAndColorT(Pawn, Black) },
      { 'n', PieceTypeAndColorT(Knight, Black) },
      { 'b', PieceTypeAndColorT(Bishop, Black) },
      { 'r', PieceTypeAndColorT(Rook, Black) },
      { 'q', PieceTypeAndColorT(Queen, Black) },
      { 'k', PieceTypeAndColorT(King, Black) },
    };

    static void parseRank(std::map<ColorT, std::map<PieceTypeT, std::vector<SquareT>>>& pieceTypeMap, int fenRank, const std::string& pieces) {
      int file = 0;
      for(char c: pieces) {

	if(file >= 8) {
	  throw std::invalid_argument("Rank has too few items in FEN piece placement string - expecting 8");
	}

	if('1' <= c && c <= '8') {
	  file += c - '0';
	} else {
	  auto it = FenPieces.find(c);

	  if(it == FenPieces.end()) {
	    throw std::invalid_argument("Invalid piece character FEN piece placement string");
	  }

	  const PieceTypeAndColorT& pieceTypeAndColor = it->second;

	  // FEN goes from rank 8 down to rank 1
	  const SquareT sq = squareOf(7 - fenRank, file);

	  pieceTypeMap[pieceTypeAndColor.color][pieceTypeAndColor.pieceType].push_back(sq);

	  file++;
	}

	if(file > 8) {
	  throw std::invalid_argument("Rank has too many items in FEN piece placement string - expecting 8");
	}
      }
    }

    // Return map: color -> map: piece-type -> squares
    // We will add the pieces to the board once we've worked out what the pieces are wrt castling rights, bishop color, promo pieces etc.
    static std::map<ColorT, std::map<PieceTypeT, std::vector<SquareT>>> parsePieces(std::string pieces) {
      std::map<ColorT, std::map<PieceTypeT, std::vector<SquareT>>> pieceTypeMap;
      
      // Split into ranks with a little hack
      std::replace(pieces.begin(), pieces.end(), '/', ' ');
      auto ranks = split(pieces);

      if(ranks.size() != 8) {
	throw std::invalid_argument("Wrong number of ranks in FEN piece placement string - expecting 8");
      }

      for(int rank = 0; rank < 8; rank++) {
	parseRank(pieceTypeMap, rank, ranks[rank]);
      }

      return pieceTypeMap;
    }
    
    static ColorT parseColor(const std::string& color) {
      if(color == "w") {
	return White;
      } else if(color == "b") {
	return Black;
      } else {
	throw std::invalid_argument("Invalid FEN color field");
      }
    }

    // Return map: color -> map: piece-type -> squares
    // We will add the pieces to the board once we've worked out what the pieces are wrt castling rights, bishop color, promo pieces etc.
    static std::map<ColorT, CastlingRightsT> parseCastlingRights(std::string castlingRightsString) {
      std::map<ColorT, CastlingRightsT> castlingRights;

      if(castlingRightsString == "-") {
	return castlingRights;
      }

      for(char c: castlingRightsString) {
	ColorT color;
	CastlingRightsT rights;
	
	switch(c) {
	case 'K': color = White; rights = CanCastleKingside; break;
	case 'Q': color = White; rights = CanCastleQueenside; break;
	case 'k': color = Black; rights = CanCastleKingside; break;
	case 'q': color = Black; rights = CanCastleQueenside; break;
	default:
	  throw std::invalid_argument("Invalid character in FEN castling rights");
	}

	castlingRights[color] = (CastlingRightsT) (castlingRights[color] | rights);
      }

      return castlingRights;
    }

    static SquareT parseEpSquare(const std::string& ep) {
      if(ep == "-") {
	return InvalidSquare;
      }
      
      if(ep.size() != 2) {
	throw std::invalid_argument("Invalid FEN en-passant field");
      }

      char fileC = ep[0];
      auto file = std::string("abcdefgh").find(fileC);
      if(file == std::string::npos) {
	throw std::invalid_argument("Invalid FEN en-passant file - expecting a-h");
      }

      char rankC = ep[1];
      if(rankC < '1' || '8' < rankC) {
	throw std::invalid_argument("Invalid FEN en-passant rank - expecting 1-8");
      }
      int rank = rankC - '1';

      return squareOf(rank, (int)file);
    }

    // Place pieces on the board - this is fiddlier than ideal because we have to map to sensible concrete pieces like BlackBishop etc.
    // Where there are castling rights, it's critical that rooks are assigned correctly.
    static void placePieces(BoardT& board, const ColorT color, std::map<PieceTypeT, std::vector<SquareT>>& pieceTypeMap, const CastlingRightsT castlingRights) {
      // Pawns - there MUST be 8 at most
      const std::vector<SquareT>& pawnSquares = pieceTypeMap[Pawn];
      if(pawnSquares.size() > 8) {
	throw std::invalid_argument("Too many pawns in FEN - expecting 8 at most per color");
      }
      for(SquareT pawnSq: pawnSquares) {
	placePawn(board, color, pawnSq);
      }
      
      // King - there MUST be only one
      const std::vector<SquareT>& kingSquares = pieceTypeMap[King];
      if(kingSquares.size() != 1) {
	throw std::invalid_argument("Invalid number of kings in FEN - there must be (only) one");
      }
      const SquareT kingSq = kingSquares[0];
      // If there are castling rights then the king MUST be on its starting square
      const SquareT KingHomes[(size_t)NColors] = {E1, E8};
      if(castlingRights != NoCastlingRights && kingSq != KingHomes[(size_t)color]) {
	throw std::invalid_argument("Castling rights specific in FEN but king is not on its starting square");
      }
      placePiece(board, color, kingSq, TheKing);

      // Queen - we only handle one at present
      const std::vector<SquareT>& queenSquares = pieceTypeMap[Queen];
      if(queenSquares.size() > 1) {
	throw std::invalid_argument("Invalid number of queens in FEN - we cannot handle promos at present");
      }
      if(queenSquares.size() > 0) {
	const SquareT queenSq = queenSquares[0];
	placePiece(board, color, queenSq, TheQueen);
      }

      // TODO - handling of rooks, knights and bishops should be factored

      // Rooks - we only handle two at present; if castling rights are specified then the corresponding rook must be on its starting position
      std::vector<SquareT> rookSquares = pieceTypeMap[Rook];
      if(rookSquares.size() > 2) {
	throw std::invalid_argument("Invalid number of rooks in FEN - we cannot handle promos at present");
      }

      const SquareT Rook1Homes[(size_t)NColors] = {A1, A8};
      const SquareT Rook1Home = Rook1Homes[(size_t)color];

      SquareT rook1Sq = InvalidSquare;
      
      auto rook1SqIt = find(rookSquares.begin(), rookSquares.end(), Rook1Home);
      if(rook1SqIt != rookSquares.end()) {
	rookSquares.erase(rook1SqIt);
	rook1Sq = Rook1Home;
      }
	
      const SquareT Rook2Homes[(size_t)NColors] = {H1, H8};
      const SquareT Rook2Home = Rook2Homes[(size_t)color];

      SquareT rook2Sq = InvalidSquare;
      
      auto rook2SqIt = find(rookSquares.begin(), rookSquares.end(), Rook2Home);
      if(rook2SqIt != rookSquares.end()) {
	rookSquares.erase(rook2SqIt);
	rook2Sq = Rook2Home;
      }

      // Otherwise allocate the rooks arbitrarily
      unsigned i = 0;
      if(rook1Sq == InvalidSquare && i < rookSquares.size()) {
	rook1Sq = rookSquares[i++];
      }
      if(rook2Sq == InvalidSquare && i < rookSquares.size()) {
	rook2Sq = rookSquares[i++];
      }

      // Check that castling rights are valid
      if((castlingRights & CanCastleQueenside) && rook1Sq != Rook1Home) {
	throw std::invalid_argument("Queen side castling specified in FEN but queen rook is not on its home square");
      }
      if(rook1Sq != InvalidSquare) {
	placePiece(board, color, rook1Sq, Rook1);
      }
	
      // Check that castling rights are valid
      if((castlingRights & CanCastleKingside) && rook2Sq != Rook2Home) {
	throw std::invalid_argument("King side castling specified in FEN but king rook is not on its home square");
      }
      if(rook2Sq != InvalidSquare) {
	placePiece(board, color, rook2Sq, Rook2);
      }
	
      // Knights - we only handle two at present; we don't really need to allocate them this way but meh!
      std::vector<SquareT> knightSquares = pieceTypeMap[Knight];
      if(knightSquares.size() > 2) {
	throw std::invalid_argument("Invalid number of knights in FEN - we cannot handle promos at present");
      }

      const SquareT Knight1Homes[(size_t)NColors] = {A1, A8};
      const SquareT Knight1Home = Knight1Homes[(size_t)color];

      SquareT knight1Sq = InvalidSquare;
      
      auto knight1SqIt = find(knightSquares.begin(), knightSquares.end(), Knight1Home);
      if(knight1SqIt != knightSquares.end()) {
	knightSquares.erase(knight1SqIt);
	knight1Sq = Knight1Home;
      }
	
      const SquareT Knight2Homes[(size_t)NColors] = {H1, H8};
      const SquareT Knight2Home = Knight2Homes[(size_t)color];

      SquareT knight2Sq = InvalidSquare;
      
      auto knight2SqIt = find(knightSquares.begin(), knightSquares.end(), Knight2Home);
      if(knight2SqIt != knightSquares.end()) {
	knightSquares.erase(knight2SqIt);
	knight2Sq = Knight2Home;
      }

      // Otherwise allocate the knights arbitrarily
      /*unsigned*/ i = 0;
      if(knight1Sq == InvalidSquare && i < knightSquares.size()) {
	knight1Sq = knightSquares[i++];
      }
      if(knight2Sq == InvalidSquare && i < knightSquares.size()) {
	knight2Sq = knightSquares[i++];
      }

      if(knight1Sq != InvalidSquare) {
	placePiece(board, color, knight1Sq, Knight1);
      }
	
      if(knight2Sq != InvalidSquare) {
	placePiece(board, color, knight2Sq, Knight2);
      }
	
      // Bishops - we only handle two at present; ditto - this is overkill but meh!
      std::vector<SquareT> bishopSquares = pieceTypeMap[Bishop];
      //printf("Color %d bishopSquares.size() is %d\n", color, bishopSquares.size());
      if(bishopSquares.size() > 2) {
	throw std::invalid_argument("Invalid number of bishops in FEN - we cannot handle promos at present");
      }
      SquareT bishop2Sq = InvalidSquare;
      SquareT bishop1Sq = InvalidSquare;
      if(bishopSquares.size() > 0) {
	bishop2Sq = bishopSquares[0];
      }
      if(bishopSquares.size() > 1) {
	bishop1Sq = bishopSquares[1];
      }

      if(bishop2Sq != InvalidSquare) {
	placePiece(board, color, bishop2Sq, Bishop2);
      }

      if(bishop1Sq != InvalidSquare) {
	placePiece(board, color, bishop1Sq, Bishop1);
      }
    }
    
    static void placePieces(BoardT& board, std::map<ColorT, std::map<PieceTypeT, std::vector<SquareT>>>& pieceTypeMap, std::map<ColorT, CastlingRightsT>& castlingRights) {
      placePieces(board, White, pieceTypeMap[White], castlingRights[White]);
      placePieces(board, Black, pieceTypeMap[Black], castlingRights[Black]);
    }
    
    std::pair<BoardT, ColorT> parseFen(const std::string& fen) {
      // split the FEN string into fields
      auto fields = split(fen);

      if(fields.size() < 4 || 6 < fields.size()) {
	throw std::invalid_argument("Invalid number of FEN fields - we allow 4 to 6");
      }

      // Piece placement
      auto pieceTypeMap = parsePieces(fields[0]);

      // Color to move
      ColorT color = parseColor(fields[1]);

      // Castling rights
      auto castlingRights = parseCastlingRights(fields[2]);

      // EP square
      SquareT epSquare = parseEpSquare(fields[3]);
      
      BoardT board = emptyBoard();

      placePieces(board, pieceTypeMap, castlingRights);

      board.state[(size_t)White].castlingRights = castlingRights[White];
      board.state[(size_t)Black].castlingRights = castlingRights[Black];

      board.state[(size_t)otherColor(color)].epSquare = epSquare;
      
      return std::make_pair(board, color);
    }

    static std::string genPieces(const BoardT& board) {
      std::stringstream ss;
      
      auto pieceMap = genPieceMap(board);

      // FEN does rank 8 first
      for(int rank = 7; rank >= 0; --rank) {
	if(rank != 7) {
	  ss << '/';
	}
	int nEmpty = 0; // count of trailing empty squares in a rank
	for(int file = 0; file < 8; file++) {
	  SquareT sq = squareOf(rank, file);
	  char c = pieceChar(pieceMap[sq]);
	  if(c == '.') {
	    nEmpty++;
	  } else {
	    if(nEmpty) {
	      ss << (char)('0' + nEmpty);
	      nEmpty = 0;
	    }
	    ss << c;
	  }
	}
	if(nEmpty) {
	  ss << (char)('0' + nEmpty);
	}
      }
      
      return ss.str();
    }

    static char genColor(const ColorT colorToMove) {
      return colorToMove == White ? 'w' : 'b';
    }

    static std::string genCastlingRights(const BoardT& board) {
      CastlingRightsT wRights = board.state[(size_t)White].castlingRights;
      CastlingRightsT bRights = board.state[(size_t)Black].castlingRights;

      if(wRights == NoCastlingRights && bRights == NoCastlingRights) {
	return "-";
      }

      std::stringstream ss;

      if(wRights & CanCastleKingside) { ss << 'K'; }
      if(wRights & CanCastleQueenside) { ss << 'Q'; }

      if(bRights & CanCastleKingside) { ss << 'k'; }
      if(bRights & CanCastleQueenside) { ss << 'q'; }

      return ss.str();
    }

    static std::string genEpSquare(const BoardT& board, const ColorT colorToMove) {
      SquareT epSq = board.state[(size_t)otherColor(colorToMove)].epSquare;

      if(epSq == InvalidSquare) { return "-"; }

      std::stringstream ss;
      ss << "abcdefgh"[fileOf(epSq)] << "12345678"[rankOf(epSq)];

      return ss.str();
    }

    // TODO - halfmove clock and fullmove number when we support those
    std::string toFen(const BoardT& board, const ColorT colorToMove) {
      std::stringstream ss;

      ss << genPieces(board) << ' ' << genColor(colorToMove) << ' ' << genCastlingRights(board) << ' ' << genEpSquare(board, colorToMove);

      return ss.str();
    }


  } // namespace Fen
} // namespace Chess
