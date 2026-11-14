#include <chess/core/move_generator.h>
#include <chess/core/position.h>
#include <chrono>
#include <thread>

namespace Debugging {
  U64 count_nodes(Position &pos, int depth) {
    if (depth == 0) return 1ULL;
    MoveList move_list;
    generate_pseudo_legal_moves(pos, move_list);
    U64 nodes = 0;
    for (const auto move : move_list) {
      if (!pos.make_move(move)) {
        pos.undo_move();
        continue;
      }
      nodes += count_nodes(pos, depth - 1);
      pos.undo_move();
    }
    return nodes;
  }

  void perft(Position &pos, int depth) {
    if (depth == 0) return;

    U64 total_nodes = 0;

    MoveList psuedo_legal;
    generate_pseudo_legal_moves(pos, psuedo_legal);
    std::vector<Move> moves(psuedo_legal.begin(), psuedo_legal.end());
    std::vector<U64> results(psuedo_legal.size());
    std::vector<std::thread> workers;
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < moves.size(); i++) {
      workers.emplace_back([&, i] {
        Position worker_position = pos;
        if (!worker_position.make_move(moves[i])) {
          worker_position.undo_move();
          results[i] = 0;
        } else {
          results[i] = count_nodes(worker_position, depth - 1);
          worker_position.undo_move();
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