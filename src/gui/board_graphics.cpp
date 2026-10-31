#include <SFML/Graphics.hpp>
#include <chess/gui/board_graphics.h>

using namespace std;
using namespace sf;

BoardGraphics::BoardGraphics()
{
    if (!loadSkins(currentBoardSkin, currentPieceSkin))
    {
        std::cerr << "[BoardGraphics Error] Failed to initialize textures!" << std::endl;
    }
}

bool BoardGraphics::loadSkins(const string boardSkin, const string PieceSkin)
{
    currentBoardSkin = boardSkin;
    currentPieceSkin = PieceSkin;

    bool boardSuccess = boardTexture.loadFromFile("assets/boards/" + currentBoardSkin + ".png");
    bool pieceSuccess;
    for (int i = 0; i < 12; i++)
    {
        string_view pieceName = GetPieceNameByIndex(i);
        string fullPath = format("assets/pieces/{}/{}.png", currentPieceSkin, pieceName);

        pieceSuccess = pieceTexture[i].loadFromFile(fullPath);

        if (!pieceSuccess)
        {
            cerr << "Failed to load piece texture: " << fullPath << endl;
        }
    }

    if (!boardSuccess)
    {
        cerr << "Failed to load board texture" << endl;
    }
    return boardSuccess && pieceSuccess;
}

void BoardGraphics::drawBoard(RenderWindow &window, Texture &boardTexture)
{
    Sprite squareSprite(boardTexture);
    for (int square = SQ_A1; square <= SQ_H8; square++)
    {
        int col = get_file(square);
        int rank = get_rank(square);
        int texTileSize  = boardTexture.getSize().x / 8;

        squareSprite.setTextureRect(IntRect(
            {col * texTileSize,
            (rank) * texTileSize},
            {texTileSize,
             texTileSize}));

        squareSprite.setPosition({
            static_cast<float>(TILE_SIZE * col),
            static_cast<float>(TILE_SIZE * rank)
        });
        window.draw(squareSprite);
    }
}

Sprite BoardGraphics::createPieceSprite(Texture &pieceTexture, int square){   
    int rank = get_rank(square);
    int col = get_file(square);

    Sprite pieceSprite(pieceTexture);

    float scaleX = (TILE_SIZE / pieceTexture.getSize().x) * PIECE_SCALE;
    float scaleY = (TILE_SIZE / pieceTexture.getSize().y) * PIECE_SCALE;

    pieceSprite.setScale({scaleX,scaleY});

    float offsetX = (TILE_SIZE - (pieceTexture.getSize().x * scaleX)) / 2.0f;
    float offsetY = (TILE_SIZE - (pieceTexture.getSize().y * scaleY)) / 2.0f;

    pieceSprite.setPosition({
        static_cast<float>(TILE_SIZE * col) + offsetX,
        static_cast<float>(TILE_SIZE * (7-rank)) + offsetY
    });
    return pieceSprite;
}

void BoardGraphics::drawSquare(RenderWindow &window, int targetSquare){
    RectangleShape square(Vector2f(TILE_SIZE,TILE_SIZE));
    const int file = targetSquare % 8;
    const int rank = targetSquare / 8;
    square.setPosition({TILE_SIZE * file, TILE_SIZE * (7-rank)});
    square.setFillColor(sf::Color{65, 105, 225 , 200});
    square.setSize({TILE_SIZE,TILE_SIZE});
    window.draw(square);
}

void BoardGraphics::run_loop(Position position)
{
    RenderWindow window(VideoMode({SCREEN_SIZE, SCREEN_SIZE}), "Chess Engine");
    window.setFramerateLimit(60);

    optional<int> draggedSquare;
    Vector2i dragPosition{};
    
    auto squareAt = [this](Vector2i pixel) -> optional<int>{
        if (pixel.x < 0 || pixel.x >= static_cast<int>(SCREEN_SIZE) ||
            pixel.y < 0 || pixel.y >= static_cast<int>(SCREEN_SIZE))
            return nullopt;
        const int col = pixel.x/static_cast<int>(TILE_SIZE);
        const int rank = 7 - (pixel.y/static_cast<int>(TILE_SIZE));
        return get_square(col, rank);
    };

    while (window.isOpen()){
        while (const std::optional event = window.pollEvent()){
            if (event->is<Event::Closed>()){
                window.close();
                break;
            }
            
            if (const auto* press = event->getIf<Event::MouseButtonPressed>()){
                if (!(press && press->button == Mouse::Button::Left)) continue;
                if (!(Mouse::isButtonPressed(Mouse::Button::Left))) continue;
                const auto square = squareAt(press->position);
                if(!(square)) continue;
                if(!(position.board[*square] != NO_PIECE_TYPE)) continue;
                dragPosition = press->position;
                draggedSquare = square;
            }

            if (const auto* move = event->getIf<Event::MouseMoved>())
                dragPosition = move->position;

            if (const auto* release = event->getIf<Event::MouseButtonReleased>()){
                if (!(release && release->button == Mouse::Button::Left && draggedSquare)) continue;
                const auto target = squareAt(release->position);
                if(target || !(draggedSquare == target)){
                    position.make_move(*draggedSquare, *target);
                    draggedSquare.reset();
                }
            }
        }
        window.clear();
        drawBoard(window, boardTexture);
        if(draggedSquare){
            vector<Move> legalMoves = position.get_legal_moves();
            for(const auto move:legalMoves){
                if (!(draggedSquare == move.startSquare)) continue;
                drawSquare(window, move.targetSquare);
            }
        }
        for (int square = SQ_A1; square <= SQ_H8; square++){
            if (draggedSquare == square) continue; 
            int index = position.board[square];
            if (index == NO_PIECE_TYPE ) continue;
            Sprite pieceSprite = createPieceSprite(pieceTexture[index], square);
            window.draw(pieceSprite);
        }

        if (draggedSquare){
            const int index = position.board[*draggedSquare];
            Vector2f newPosition = {dragPosition.x - Offset, dragPosition.y - Offset} ;
            Sprite pieceSprite = createPieceSprite(pieceTexture[index], *draggedSquare);
            pieceSprite.setPosition(newPosition);
            pieceSprite.setScale({0.7f, 0.7f});
            window.draw(pieceSprite);
        }
        
        window.display();
    }
}