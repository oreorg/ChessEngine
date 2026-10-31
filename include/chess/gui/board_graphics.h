#pragma once
#include <iostream>
#include <format>
#include <SFML/Graphics.hpp>
#include <chess/core/position.h>
#include <chess/core/types.h>

class BoardGraphics{
    private:
        sf::Texture boardTexture;
        sf::Texture pieceTexture[12];
        std::string currentPieceSkin = "game_room";
        std::string currentBoardSkin = "dark_wood";
        
        unsigned int SCREEN_SIZE = 800;
        float TILE_SIZE = SCREEN_SIZE / 8;
        float Offset = 50.0f;
        float PIECE_SCALE = 0.9f;

    public:
        BoardGraphics();

        bool loadSkins(const std::string boardSkin, const std::string PieceSkin);
        void drawBoard(sf::RenderWindow& window,sf::Texture &boardTexture);
        sf::Sprite createPieceSprite(sf::Texture &PieceTexture, int square);
        void run_loop(Position position);
        void drawSquare(sf::RenderWindow &window, int targetSquare);
};
