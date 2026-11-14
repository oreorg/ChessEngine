# Chess Engine

A C++20 chess project with a graphical board, legal move generation, and perft node counting. It uses bitboards for chess logic and SFML 3 for the graphical interface.

> This project is a work in progress. The engine currently selects moves using a basic material evaluation and shallow search; it is not yet a strong chess-playing AI.

## Features

- Chess position representation using bitboards
- Legal move generation, including castling, en passant, and promotions
- FEN position parsing
- Perft node counting for move-generation checks
- Basic material-based AI move selection
- SFML graphical board with draggable pieces
- Undo the last move with the `R` key
- Board and piece image assets included in `assets/`

## Requirements

- C++20 compiler
- CMake 3.16 or newer
- SFML 3.0 with Graphics, Window, and System components

## Build

From the project root, configure and build with CMake:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

SFML 3 must be installed and discoverable by CMake. If CMake cannot find SFML, point it to your SFML installation using the package path appropriate for your system.

## Run

Run the `ChessEngine` executable from its output directory. The build copies the `assets` folder beside the executable, and the program loads textures using relative paths.

At startup, the program prints perft results for depths 1 through 5, then opens the chessboard window.

## Controls

- Drag a piece to a highlighted legal destination to move it.
- Press `R` to undo the previous move.
- For a pawn promotion, choose a piece in the console when prompted.

## Project layout

```text
include/chess/   Public headers
src/core/        Position, FEN, attack, and move generation
src/engine/      Engine move selection
src/debug/       Perft node counting
src/gui/         SFML board interface
assets/          Board and piece textures
```

## License

See [LICENSE](LICENSE).