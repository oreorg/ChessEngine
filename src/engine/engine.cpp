#include <chess/core/move_generator.h>
#include <chess/core/position.h>
#include <chess/engine/engine.h>
#include <chess/engine/evaluation.h>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

int MATE_SCORE = 50000;
int MAX_DEPTH = 1;

int negamax(Position &position, int depth, int alpha, int beta) {
  if (depth == 0) {
    const int score = evaluate(position);
    return position.side_to_move == WHITE ? score : -score;
  }

  int best_score = -MATE_SCORE;
  MoveList legal_moves = generate_legal_moves(position);

  for (const auto move : legal_moves) {
    position.make_move(move);
    int score = -negamax(position, depth - 1, alpha, beta);
    position.undo_move();

    if (score > best_score) best_score = score;
  }

  if (position.check_position() == CHECKMATE) {
    return -MATE_SCORE;
  }
  if (position.check_position() == STALEMATE) {
    return 0;
  }
  return best_score;
}

std::optional<Move> Engine::find_best_move(Position &position) {
  MoveList legal_moves = generate_legal_moves(position);
  if (legal_moves.empty()) return std::nullopt;

  int alpha = -MATE_SCORE;
  int beta = MATE_SCORE;

  std::vector<std::thread> workers = {};
  std::vector<Move> moves(legal_moves.begin(), legal_moves.end());
  Move best_move = Move(0, 0);
  std::vector<int> scores(moves.size());

  for (size_t i = 0; i < moves.size(); i++) {
    workers.emplace_back([&, i]() {
      Position worker_position = position;
      worker_position.make_move(moves[i]);
      int score = -negamax(worker_position, MAX_DEPTH, alpha, beta);
      worker_position.undo_move();
      scores[i] = score;
    });
  }

  for (auto &worker : workers) {
    worker.join();
  }
  int best_score = std::numeric_limits<int>::lowest();

  for (size_t i = 0; i < scores.size(); i++) {
    if (scores[i] > best_score) {
      best_move = moves[i];
      best_score = scores[i];
    }
  }

  std::cout << best_score << std::endl;

  return best_move;
}