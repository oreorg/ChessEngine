#include <SFML/Graphics.hpp>
#include <algorithm>
#include <chess/gui/board_graphics.h>

using namespace std;
using namespace sf;

BoardGraphics::BoardGraphics() {
  if (!loadSkins(currentBoardSkin, currentPieceSkin)) {
    std::cerr << "[BoardGraphics Error] Failed to initialize textures!" << std::endl;
  }
}

bool BoardGraphics::loadSkins(const string boardSkin, const string PieceSkin) {
  currentBoardSkin = boardSkin;
  currentPieceSkin = PieceSkin;

  const bool boardSuccess = boardTexture.loadFromFile("assets/boards/" + currentBoardSkin + ".png");
  bool allPiecesLoaded = true;
  for (int i = 0; i < 12; ++i) {
    const string_view pieceName = GetPieceNameByIndex(i);
    const string fullPath = format("assets/pieces/{}/{}.png", currentPieceSkin, pieceName);

    const bool pieceLoaded = pieceTexture[i].loadFromFile(fullPath);

    if (!pieceLoaded) {
      cerr << "Failed to load piece texture: " << fullPath << endl;
    }
    allPiecesLoaded = pieceLoaded && allPiecesLoaded;
  }

  if (!boardSuccess) {
    cerr << "Failed to load board texture" << endl;
  }
  return boardSuccess && allPiecesLoaded;
}

void BoardGraphics::drawBoard(RenderWindow &window, Texture &boardTexture) {
  Sprite squareSprite(boardTexture);
  for (int square = SQ_A1; square <= SQ_H8; square++) {
    int col = get_file(square);
    int rank = get_rank(square);
    int texTileSize = boardTexture.getSize().x / 8;

    squareSprite.setTextureRect(IntRect({col * texTileSize, (rank)*texTileSize}, {texTileSize, texTileSize}));

    squareSprite.setPosition({static_cast<float>(TILE_SIZE * col), static_cast<float>(TILE_SIZE * rank)});
    window.draw(squareSprite);
  }
}

Sprite BoardGraphics::createPieceSprite(Texture &pieceTexture, int square) {
  int rank = get_rank(square);
  int col = get_file(square);

  Sprite pieceSprite(pieceTexture);

  float scaleX = (TILE_SIZE / pieceTexture.getSize().x) * PIECE_SCALE;
  float scaleY = (TILE_SIZE / pieceTexture.getSize().y) * PIECE_SCALE;

  pieceSprite.setScale({scaleX, scaleY});

  float offsetX = (TILE_SIZE - (pieceTexture.getSize().x * scaleX)) / 2.0f;
  float offsetY = (TILE_SIZE - (pieceTexture.getSize().y * scaleY)) / 2.0f;

  pieceSprite.setPosition(
      {static_cast<float>(TILE_SIZE * col) + offsetX, static_cast<float>(TILE_SIZE * (7 - rank)) + offsetY});
  return pieceSprite;
}

void BoardGraphics::drawSquare(RenderWindow &window, int targetSquare) {
  RectangleShape square(Vector2f(TILE_SIZE, TILE_SIZE));
  const int file = targetSquare % 8;
  const int rank = targetSquare / 8;
  square.setPosition({TILE_SIZE * file, TILE_SIZE * (7 - rank)});
  square.setFillColor(sf::Color{65, 105, 225, 200});
  square.setSize({TILE_SIZE, TILE_SIZE});
  window.draw(square);
}

void BoardGraphics::run_loop(Position position) {
  vector<Move> legalMoves;
  RenderWindow window(VideoMode({SCREEN_SIZE, SCREEN_SIZE}), "Chess Engine");
  window.setFramerateLimit(60);

  optional<int> draggedSquare;
  Vector2i dragPosition{};

  auto squareAt = [this](Vector2i pixel) -> optional<int> {
    if (pixel.x < 0 || pixel.x >= static_cast<int>(SCREEN_SIZE) || pixel.y < 0 ||
        pixel.y >= static_cast<int>(SCREEN_SIZE))
      return nullopt;
    const int col = pixel.x / static_cast<int>(TILE_SIZE);
    const int rank = 7 - (pixel.y / static_cast<int>(TILE_SIZE));
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

        legalMoves = position.getLegalMoves();
        const auto square = squareAt(press->position);

        if (!(square)) continue;
        if (!(position.board[*square] != NO_PIECE_TYPE)) continue;
        dragPosition = press->position;
        draggedSquare = square;
      }

      if (const auto *keyPress = event->getIf<Event::KeyPressed>()) {
        if (keyPress->code == sf::Keyboard::Key::R) {
          cout << position.undo_move() << endl;
        }
      }

      if (const auto *moved = event->getIf<Event::MouseMoved>()) dragPosition = moved->position;

      if (const auto *release = event->getIf<Event::MouseButtonReleased>()) {
        if (!(release && release->button == Mouse::Button::Left && draggedSquare)) continue;
        if (const auto target = squareAt(release->position); target && *target != *draggedSquare) {
          std::vector<Move> matchingMoves;

          std::ranges::copy_if(legalMoves, std::back_inserter(matchingMoves), [&](const Move &move) {
            return move.square() == *draggedSquare && move.target_square() == *target;
          });
          int chosenPiece = 0;
          char selection;
          if (!(matchingMoves.empty())) {
            if (matchingMoves[0].type() == PROMOTION) {
              cout << "choose n : KNIGHT | b : BISHOP | r : ROOK | q : QUEEN" << endl;
              cin >> selection;
              switch (selection) {
              case 'n':
                chosenPiece = 0;
                break;
              case 'b':
                chosenPiece = 1;
                break;
              case 'r':
                chosenPiece = 2;
                break;
              case 'q':
                chosenPiece = 3;
                break;
              default:
                chosenPiece = 3;
                break;
              }
            }
            position.make_move(matchingMoves[chosenPiece]);
          }
        }
        draggedSquare.reset();
      }
    }
    window.clear();
    drawBoard(window, boardTexture);
    if (draggedSquare) {
      for (const auto move : legalMoves) {
        if (!(draggedSquare == move.square())) continue;
        drawSquare(window, move.target_square());
      }
    }
    for (int square = SQ_A1; square <= SQ_H8; square++) {
      if (draggedSquare == square) continue;
      int index = position.board[square];
      if (index == NO_PIECE_TYPE) continue;
      Sprite pieceSprite = createPieceSprite(pieceTexture[index], square);
      window.draw(pieceSprite);
    }

    if (draggedSquare) {
      const int index = position.board[*draggedSquare];
      Vector2f newPosition = {dragPosition.x - Offset, dragPosition.y - Offset};
      Sprite pieceSprite = createPieceSprite(pieceTexture[index], *draggedSquare);
      pieceSprite.setPosition(newPosition);
      pieceSprite.setScale({0.7f, 0.7f});
      window.draw(pieceSprite);
    }

    window.display();
  }
}