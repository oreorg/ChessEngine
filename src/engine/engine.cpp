#include <chess/core/move_generator.h>
#include <chess/core/position.h>
#include <chess/engine/engine.h>
#include <chess/engine/evaluation.h>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

int MATE_SCORE = 50000;
int MAX_DEPTH = 6;

int negamax(Position &position, int depth, int alpha, int beta) {
  if (depth == 0) {
    const int score = evaluate(position);
    return position.side_to_move == WHITE ? score : -score;
  }
  int score;
  MoveList psuedo_legal;
  bool has_legal_moves = false;
  generate_pseudo_legal_moves(position, psuedo_legal);

  for (const auto move : psuedo_legal) {
    if (!position.make_move(move)) {
      position.undo_move();
      continue;
    }
    has_legal_moves = true;
    score = -negamax(position, depth - 1, -beta, -alpha);
    position.undo_move();

    if (score >= beta) {
      return beta;
    }
    if (score > alpha) {
      alpha = score;
    }
  };

  if (!has_legal_moves) {
    U64 king = position.piece_boards[KING + position.side_to_move * 6];
    PieceColor opponent = position.side_to_move == WHITE ? BLACK : WHITE;
    if (is_square_attacked(position, pop_lsb(king), opponent)) {
      return -MATE_SCORE + (MAX_DEPTH - depth);
    }
    return 0;
  }

  return alpha;
}

std::optional<Move> Engine::find_best_move(Position &position) {
  MoveList legal_moves;
  generate_pseudo_legal_moves(position, legal_moves);

  int alpha = -MATE_SCORE;
  int beta = MATE_SCORE;

  std::vector<std::thread> workers = {};
  std::vector<Move> moves(legal_moves.begin(), legal_moves.end());
  Move best_move = Move(0, 0);
  std::vector<int> scores(moves.size(), std::numeric_limits<int>::lowest());

  for (size_t i = 0; i < moves.size(); i++) {
    workers.emplace_back([&, i]() {
      Position worker_position = position;
      if (!worker_position.make_move(moves[i])) {
        worker_position.undo_move();
      } else {
        int score = -negamax(worker_position, MAX_DEPTH, -beta, -alpha);
        worker_position.undo_move();
        scores[i] = score;
      }
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

  if (best_score == std::numeric_limits<int>::lowest()) return std::nullopt;

  std::cout << "best score: " << best_score << ", best move: " << best_move.to_string() << std::endl;

  return best_move;
}