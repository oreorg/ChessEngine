#include <SFML/Graphics.hpp>
#include <chess/gui/board_graphics.h>
#include <future>

using namespace std;
using namespace sf;

BoardGraphics::BoardGraphics() {
  if (!load_skins(current_board_skin, current_piece_skin)) {
    std::cerr << "[BoardGraphics Error] Failed to initialize textures!" << std::endl;
  }
}

bool BoardGraphics::load_skins(const string &board_skin, const string &piece_skin) {
  current_board_skin = board_skin;
  current_piece_skin = piece_skin;

  const bool board_success = board_texture.loadFromFile("assets/boards/" + current_board_skin + ".png");
  bool all_pieces_loaded = true;
  for (int i = 0; i < 12; ++i) {
    const string_view piece_name = get_piece_name_by_index(i);
    const string full_path = format("assets/pieces/{}/{}.png", current_piece_skin, piece_name);

    const bool piece_loaded = piece_textures[i].loadFromFile(full_path);

    if (!piece_loaded) {
      cerr << "Failed to load piece texture: " << full_path << endl;
    }
    all_pieces_loaded = piece_loaded && all_pieces_loaded;
  }

  if (!board_success) {
    cerr << "Failed to load board texture" << endl;
  }
  return board_success && all_pieces_loaded;
}

void BoardGraphics::draw_board(RenderWindow &window, Texture &board_texture) {
  Sprite square_sprite(board_texture);
  for (int square = SQ_A1; square <= SQ_H8; square++) {
    int col = get_file(square);
    int rank = get_rank(square);
    int texture_tile_size = board_texture.getSize().x / 8;

    square_sprite.setTextureRect(IntRect({col * texture_tile_size, (rank)*texture_tile_size}, {texture_tile_size, texture_tile_size}));

    square_sprite.setPosition({static_cast<float>(tile_size * col), static_cast<float>(tile_size * rank)});
    window.draw(square_sprite);
  }
}

Sprite BoardGraphics::create_piece_sprite(Texture &piece_texture, int square) {
  int rank = inverse ? 7 - get_rank(square) : get_rank(square);
  int col = get_file(square);

  Sprite piece_sprite(piece_texture);

  float scale_x = (tile_size / piece_texture.getSize().x) * piece_scale;
  float scale_y = (tile_size / piece_texture.getSize().y) * piece_scale;

  piece_sprite.setScale({scale_x, scale_y});

  float offset_x = (tile_size - (piece_texture.getSize().x * scale_x)) / 2.0f;
  float offset_y = (tile_size - (piece_texture.getSize().y * scale_y)) / 2.0f;

  piece_sprite.setPosition({static_cast<float>(tile_size * col) + offset_x, static_cast<float>(tile_size * (7 - rank)) + offset_y});
  return piece_sprite;
}

void BoardGraphics::draw_square(RenderWindow &window, int target_square) {
  RectangleShape square(Vector2f(tile_size, tile_size));
  const int file = target_square % 8;
  const int rank = inverse ? 7 - (target_square / 8) : (target_square / 8);
  square.setPosition({tile_size * file, tile_size * (7 - rank)});
  square.setFillColor(sf::Color{65, 105, 225, 200});
  square.setSize({tile_size, tile_size});
  window.draw(square);
}

void BoardGraphics::run_loop(Game &game, Engine &engine) {
  std::future<std::optional<Move>> engine_future;
  bool engine_searching = false;
  sf::Clock ai_wait_clock;

  RenderWindow window(VideoMode({screen_size, screen_size}), "Chess Engine");
  window.setFramerateLimit(60);

  optional<int> dragged_square;
  Vector2i drag_position{};

  auto square_at = [this, &game](Vector2i pixel) -> optional<int> {
    if (pixel.x < 0 || pixel.x >= static_cast<int>(screen_size) || pixel.y < 0 || pixel.y >= static_cast<int>(screen_size)) return nullopt;
    const int col = pixel.x / static_cast<int>(tile_size);
    const int rank = game.player_color == WHITE ? 7 - (pixel.y / static_cast<int>(tile_size)) : (pixel.y / static_cast<int>(tile_size));
    return get_square(col, rank);
  };

  while (window.isOpen()) {
    while (const std::optional event = window.pollEvent()) {
      if (event->is<Event::Closed>()) {
        window.close();
        break;
      }

      if (const auto *press = event->getIf<Event::MouseButtonPressed>()) {
        if (!(press && press->button == Mouse::Button::Left)) continue;
        if (!(Mouse::isButtonPressed(Mouse::Button::Left))) continue;

        const auto square = square_at(press->position);

        if (!(square)) continue;
        if (!(game.position.board[*square] != NO_PIECE_TYPE)) continue;
        PieceColor piece_color = game.position.board[*square] >= 6 ? BLACK : WHITE;
        if (piece_color == game.player_color) {
          drag_position = press->position;
          dragged_square = square;
        }
      }

      if (const auto *key_press = event->getIf<Event::KeyPressed>()) {
        if (key_press->code == sf::Keyboard::Key::R) {
          game.position.undo_move();
          ai_wait_clock.restart();
        }
      }

      if (const auto *moved = event->getIf<Event::MouseMoved>()) drag_position = moved->position;

      if (const auto *release = event->getIf<Event::MouseButtonReleased>()) {
        if (!(release && release->button == Mouse::Button::Left && dragged_square)) continue;
        if (const auto target = square_at(release->position); target && *target != *dragged_square) {
          vector<Move> matching_moves;

          std::ranges::copy_if(generate_legal_moves(game.position), std::back_inserter(matching_moves),
                               [&](const Move &move) { return move.square() == *dragged_square && move.target_square() == *target; });
          int chosen_piece = 0;
          char selection;
          if (!matching_moves.empty()) {
            if (matching_moves[0].type() == PROMOTION) {
              cout << "choose n : KNIGHT | b : BISHOP | r : ROOK | q : QUEEN" << endl;
              cin >> selection;
              switch (selection) {
              case 'n': chosen_piece = 1; break;
              case 'b': chosen_piece = 2; break;
              case 'r': chosen_piece = 3; break;
              case 'q': chosen_piece = 4; break;
              default: chosen_piece = 4; break;
              }
            }
            game.position.make_move(matching_moves[chosen_piece]);
            cout << "hash: " << hex << game.position.hash_key
                 << ", castling_rights: " << static_cast<unsigned int>(game.position.castling_rights)
                 << ", side_to_move: " << game.position.side_to_move << dec << ", enpassant_square: " << game.position.en_passant_square
                 << endl;
            ai_wait_clock.restart();
          }
        }
        dragged_square.reset();
      }
    }
    window.clear();
    draw_board(window, board_texture);
    if (dragged_square) {
      for (const auto move : generate_legal_moves(game.position)) {
        if (!(dragged_square == move.square())) continue;
        draw_square(window, move.target_square());
      }
    }
    for (int square = SQ_A1; square <= SQ_H8; square++) {
      if (dragged_square == square) continue;
      int index = game.position.board[square];
      if (index == NO_PIECE_TYPE) continue;
      Sprite piece_sprite = create_piece_sprite(piece_textures[index], square);
      window.draw(piece_sprite);
    }

    if (dragged_square) {
      const int index = game.position.board[*dragged_square];
      Vector2f new_position = {drag_position.x - offset, drag_position.y - offset};
      Sprite piece_sprite = create_piece_sprite(piece_textures[index], *dragged_square);
      piece_sprite.setPosition(new_position);
      piece_sprite.setScale({0.7f, 0.7f});
      window.draw(piece_sprite);
    }
    window.display();

    // After drawing each frame:
    if (!engine_searching && engine.ai_color == game.position.side_to_move) {
      Position snapshot = game.position;
      engine_future =
          std::async(std::launch::async, [&engine, snapshot = std::move(snapshot)]() mutable { return engine.find_best_move(snapshot); });
      engine_searching = true;
    }

    if (engine_searching && engine_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
      if (auto move = engine_future.get()) game.position.make_move(*move);
      engine_searching = false;
    }
  }
}