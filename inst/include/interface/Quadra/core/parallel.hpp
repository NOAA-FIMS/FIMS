#ifndef PARALLEL_HPP
#define PARALLEL_HPP
#pragma once

#include <algorithm>
#include <thread>
#include <vector>

namespace quadra {

// Generic parallel reduce over index range.
// The result type defaults to double, but can be AD/AD3/etc.
// Note: reverse-mode AD types that rely on a thread-local/global tape are
// generally not safe to build concurrently unless each worker owns its tape.
template <typename Result = double, typename Func>
Result parallel_reduce(int n_items, Func &&func) {
  unsigned int n_threads = std::thread::hardware_concurrency();
  if (n_threads == 0)
    n_threads = 4;

  int chunk =
      (n_items + static_cast<int>(n_threads) - 1) / static_cast<int>(n_threads);

  std::vector<std::thread> threads;
  std::vector<Result> results(n_threads, Result(0.0));

  for (unsigned int t = 0; t < n_threads; ++t) {
    int start = static_cast<int>(t) * chunk;
    int end = std::min(start + chunk, n_items);

    if (start >= end)
      break;

    threads.emplace_back([&, t, start, end]() {
      Result local = Result(0.0);

      for (int i = start; i < end; ++i)
        local += func(i);

      results[t] = local;
    });
  }

  for (auto &th : threads)
    th.join();

  Result total = Result(0.0);
  for (const auto &v : results)
    total += v;

  return total;
}

} // namespace quadra

#endif // PARALLEL_HPP
