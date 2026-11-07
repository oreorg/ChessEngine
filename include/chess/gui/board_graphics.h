#pragma once
#include <iostream>
#include <format>
#include <SFML/Graphics.hpp>
#include <chess/core/position.h>
#include <chess/core/types.h>

class BoardGraphics{
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
        BoardGraphics();

        bool load_skins(const std::string& board_skin, const std::string& piece_skin);
        void draw_board(sf::RenderWindow& window, sf::Texture& board_texture);
        sf::Sprite create_piece_sprite(sf::Texture& piece_texture, int square);
        void run_loop(Position position);
        void draw_square(sf::RenderWindow& window, int target_square);
};
