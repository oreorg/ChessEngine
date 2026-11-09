#include <charconv>
#include <chess/core/position.h>
#include <iostream>
#include <string_view>

PieceType get_piece_type_by_char(char character) {
  switch (character) {
  case 'p': return PAWN;
  case 'n': return KNIGHT;
  case 'b': return BISHOP;
  case 'r': return ROOK;
  case 'q': return QUEEN;
  case 'k': return KING;
  default: return NO_PIECE_TYPE;
  }
}

Position parse_fen(std::string_view fen) {
  Position position;
  size_t first_space = fen.find(' ');
  std::string_view piece_placement = fen.substr(0, first_space);
  std::string_view special = fen.substr(first_space + 1, fen.length());
  position.clear();

  int square = SQ_A8;
  for (const auto piece : piece_placement) {
    if (piece == '/') {
      square -= 16;
    } else if (piece >= '1' && piece <= '8') {
      square += piece - '0';
    } else {
      PieceColor piece_color = std::isupper(static_cast<unsigned char>(piece)) ? WHITE : BLACK;
      PieceType piece_type = get_piece_type_by_char(static_cast<char>(std::tolower(static_cast<unsigned char>(piece))));
      position.place_piece(piece_type, piece_color, square++);
    }
  }
  position.side_to_move = special[0] == 'w' ? WHITE : BLACK;

  const size_t active_end = fen.find(' ', first_space + 1);
  const size_t castling_end = fen.find(' ', active_end + 1);
  const std::string_view castling = fen.substr(active_end + 1, castling_end - active_end - 1);

  position.castling_rights = 0;

  for (char right : castling) {
    switch (right) {
    case 'K': position.castling_rights |= WHITE_KING_SIDE; break;
    case 'Q': position.castling_rights |= WHITE_QUEEN_SIDE; break;
    case 'k': position.castling_rights |= BLACK_KING_SIDE; break;
    case 'q': position.castling_rights |= BLACK_QUEEN_SIDE; break;
    }
  }

  const size_t ep_end = fen.find(' ', castling_end + 1);
  const std::string_view ep = fen.substr(castling_end + 1, ep_end - castling_end - 1);
  position.en_passant_square = -1;
  if (ep != "-") {
    position.en_passant_square = (ep[1] - '1') * 8 + (ep[0] - 'a');
  }

  const size_t halfmove_end = fen.find(' ', ep_end + 1);
  const std::string_view halfmove = fen.substr(ep_end + 1, halfmove_end - ep_end + 1);

  std::from_chars(halfmove.data(), halfmove.data() + halfmove.size(), position.half_moves);
  return position;
}