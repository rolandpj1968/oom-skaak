#include <algorithm>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
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
      for(unsigned i = 0; i < pieces.size(); i++) {

	if(file >= 8) {
	  throw std::invalid_argument("Rank has too few items in FEN piece placement string - expecting 8");
	}

	char c = pieces[i];

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

    std::pair<BoardT, ColorT> parseFen(const std::string& fen) {
      // split the FEN string into fields
      auto fields = split(fen);

      if(fields.size() < 4 || 6 < fields.size()) {
	throw std::invalid_argument("Invalid number of FEN fields - we allow 4 to 6");
      }

      // Place the pieces
      auto pieceTypeMap = parsePieces(fields[0]);

      ColorT color = parseColor(fields[1]);
      
      BoardT board;
      
      throw std::invalid_argument("implement me");

      return std::make_pair(board, color);
    }

  } // namespace Fen
} // namespace Chess
