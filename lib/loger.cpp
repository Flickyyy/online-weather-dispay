#include "loger.hpp"
#include <cstdlib>
#include <iostream>
#include <vector>

namespace flicky {
std::vector<std::string> messages_;

void Loger::Report(const std::string &report) { messages_.push_back(report); }
void Loger::CheckLog() {
  if (!messages_.empty()) {
    for (const auto &e : messages_) {
      std::cerr << "Loger::CheckLog:  " << e << '\n';
    }
    exit(EXIT_FAILURE);
  }
}
} // namespace flicky
