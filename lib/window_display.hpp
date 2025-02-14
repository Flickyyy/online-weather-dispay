#pragma once
#include "nlohmann/json.hpp"
#include "weather_manager.hpp"
#include <cstddef>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <string>

class window_display {
public:
  window_display() : manager("config.json") {}
  int Run();

private:
  ftxui::Element weather_window(const std::wstring &name, std::size_t index);
  ftxui::Element GetDayInfo(std::size_t index);
  ftxui::Element GetNormalName(const std::string &param,
                               const std::string &val);
  void Regenerate();

  weather_manager manager;
  ftxui::Elements days;
  nlohmann::json data_;
  nlohmann::json time_;
  const std::size_t days_default_count_ = 3;
};
