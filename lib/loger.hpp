#pragma once
#include <string>
namespace flicky {
class Loger {
public:
  Loger() = delete;
  Loger(const Loger &) = delete;
  Loger operator=(const Loger &) = delete;
  static void Report(const std::string &report);
  static void CheckLog();
};
} // namespace flicky
