#include <chess/core/position.h>
#include <chess/core/move_generator.h>
#include <chrono>
#include <thread>

namespace Debugging {
  U64 count_nodes(Position &pos, int depth) {
    if (depth == 0) return 1ULL;

    U64 nodes = 0;
    for (const auto move : generate_legal_moves(pos)) {
      if (!pos.make_move(move)) continue;
      nodes += count_nodes(pos, depth - 1);
      pos.undo_move();
    }
    return nodes;
  }

  void perft(Position &pos, int depth) {
    if (depth == 0) return;

    U64 total_nodes = 0;

    MoveList legal_moves = generate_legal_moves(pos);
    std::vector<Move> moves(legal_moves.begin(), legal_moves.end());
    std::vector<U64> results(legal_moves.size());
    std::vector<std::thread> workers;
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < moves.size(); i++) {
      workers.emplace_back([&, i] {
        Position worker_position = pos;
        if (worker_position.make_move(moves[i])) {
          results[i] = count_nodes(worker_position, depth - 1);
        }
      });
    }

    for (auto &worker : workers) {
      worker.join();
    }

    for (const auto result : results) {
      total_nodes += result;
    }

    auto end = std::chrono::high_resolution_clock::now();

    // FIXED: Changed total duration to milliseconds as well, then converted to seconds for NPS
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    double duration_sec = duration_ms / 1000.0;

    std::cout << "\nTotal Nodes: " << total_nodes << "\nDepth: " << depth;
    std::cout << "\nTime: " << duration_sec << " sec (" << duration_ms << " ms)\n";

    if (duration_sec > 0.0) {
      std::cout << "NPS: " << (unsigned long long)(total_nodes / duration_sec) << "\n";
    }
  }
} // namespace Debugging