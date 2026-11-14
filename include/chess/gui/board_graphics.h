#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <chess/core/game.h>
#include <chess/core/zobrist_hash.h>
#include <chess/engine/engine.h>
#include <chess/core/move_generator.h>
#include <format>
#include <iostream>

class BoardGraphics {
private:
  sf::Texture board_texture;
  sf::Texture piece_textures[12];
  std::string current_piece_skin = "game_room";
  std::string current_board_skin = "dark_wood";

  unsigned int screen_size = 800;
  float tile_size = screen_size / 8;
  float offset = 50.0f;
  float piece_scale = 0.9f;

public:
  bool inverse;
  BoardGraphics();

  bool load_skins(const std::string &board_skin, const std::string &piece_skin);
  void draw_board(sf::RenderWindow &window, sf::Texture &board_texture);
  sf::Sprite create_piece_sprite(sf::Texture &piece_texture, int square);
  void run_loop(Game &game, Engine &engine);
  void draw_square(sf::RenderWindow &window, int target_square);
};
