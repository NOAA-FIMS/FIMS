#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <utility>

namespace quadra {

class RuntimeTimers {
public:
  using clock = std::chrono::high_resolution_clock;

  struct ScopedTimer {
    RuntimeTimers &timers_m;
    std::string name_m;
    clock::time_point start_m;

    ScopedTimer(RuntimeTimers &timers, std::string name)
        : timers_m(timers), name_m(std::move(name)), start_m(clock::now()) {}

    ScopedTimer(const ScopedTimer &) = delete;
    ScopedTimer &operator=(const ScopedTimer &) = delete;

    ScopedTimer(ScopedTimer &&other) noexcept
        : timers_m(other.timers_m), name_m(std::move(other.name_m)),
          start_m(other.start_m), active_m(other.active_m) {
      other.active_m = false;
    }

    ~ScopedTimer() {
      if (!active_m)
        return;
      const auto end = clock::now();
      const double ms =
          std::chrono::duration<double, std::milli>(end - start_m).count();
      timers_m.add(name_m, ms);
    }

  private:
    bool active_m = true;
  };

  void add(const std::string &name, double ms) {
    totals_m[name] += ms;
    counts_m[name] += 1;
  }

  ScopedTimer scoped(const std::string &name) {
    return ScopedTimer(*this, name);
  }

  void reset() {
    totals_m.clear();
    counts_m.clear();
  }

  double phase_ms(const std::string &name) const {
    const auto it = totals_m.find(name);
    return it == totals_m.end() ? 0.0 : it->second;
  }

  std::size_t phase_calls(const std::string &name) const {
    const auto it = counts_m.find(name);
    return it == counts_m.end() ? 0 : it->second;
  }

  double total_ms() const {
    double out = 0.0;
    for (const auto &kv : totals_m)
      out += kv.second;
    return out;
  }

  void print(const std::string &title = "Runtime timer report") const {
    const double total = total_ms();

    std::cout << "\n" << title << "\n";
    std::cout << std::left << std::setw(34) << "phase" << std::right
              << std::setw(14) << "total ms" << std::setw(12) << "calls"
              << std::setw(14) << "avg ms" << std::setw(12) << "% total"
              << "\n";

    std::cout << std::string(86, '-') << "\n";

    for (const auto &kv : totals_m) {
      const std::string &name = kv.first;
      const double ms = kv.second;
      const std::size_t n = counts_m.at(name);
      const double avg = n > 0 ? ms / static_cast<double>(n) : 0.0;
      const double pct = total > 0.0 ? 100.0 * ms / total : 0.0;

      std::cout << std::left << std::setw(34) << name << std::right
                << std::setw(14) << std::fixed << std::setprecision(3) << ms
                << std::setw(12) << n << std::setw(14) << avg << std::setw(11)
                << pct << "%" << "\n";
    }

    std::cout << std::string(86, '-') << "\n";
    std::cout << std::left << std::setw(34) << "TOTAL" << std::right
              << std::setw(14) << std::fixed << std::setprecision(3) << total
              << "\n\n";
  }

private:
  std::map<std::string, double> totals_m;
  std::map<std::string, std::size_t> counts_m;
};

} // namespace quadra
