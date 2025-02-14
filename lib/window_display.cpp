#include "window_display.hpp"
#include "ftxui/component/component.hpp" // for Button, Horizontal, Renderer
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive
#include "ftxui/dom/elements.hpp" // for gauge, separator, text, vbox, operator|, Element, border
#include "ftxui/screen/color.hpp" // for Color, Color::Blue, Color::Green, Color::Red
#include "loger.hpp"
#include "nlohmann/detail/value_t.hpp"
#include "weather_manager.hpp"
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <ftxui/component/event.hpp> // for Event
#include <ftxui/component/loop.hpp>
#include <ftxui/component/mouse.hpp> // for ftxui
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/string.hpp>
#include <string>

auto GetColor(const std::string &param, const std::string &val) {
  using namespace ftxui;
  if (param == "temperature_2m") {
    return val[0] == '-' ? color(Color::Blue) : color(Color::Red);
  }
  if (param == "wind_speed_10m") {
    return isdigit(val[1]) ? color(Color::OrangeRed1)
                           : color(Color::SpringGreen1);
  }
  if (param == "relative_humidity_2m") {
    return val[0] < '6' ? color(Color::Wheat1) : color(Color::RoyalBlue1);
  }
  if (param == "visibility") {
    return val.size() < 5 || val[0] < '2' ? color(Color::LightSlateGrey)
           : val[0] > '2'                 ? color(Color::Red3)
                                          : color(Color::Cyan3);
  }
  return color(Color::White);
}

ftxui::Element window_display::GetNormalName(const std::string &param,
                                             const std::string &val) {
  return ftxui::text(
             val +
             (data_["hourly_units"][param] != nlohmann::detail::value_t::null
                  ? data_["hourly_units"][param].get<std::string>()
                  : "???")) |
         GetColor(param, val);
}

ftxui::Element GetWeather(int ind) {
  using namespace ftxui;
  if (ind == 0) {
    return vbox({
        hbox(text("_ - _ - _ -")),
        hbox(text(" _ - _ - _ ")),
        hbox(text("_ - _ - _ -")),
        hbox(text(" _ - _ - _ ")),
    });
  }
  if (ind == 1) {
    return vbox({
        hbox(text("   \\  /  ")),
        hbox(text("_ /\"\".-. ")),
        hbox(text("  \\_(   ).")),
        hbox(text("  /(___(__)")),
    });
  }
  return vbox({
      hbox(text("     .--.   ")),
      hbox(text("  .-(    ). ")),
      hbox(text(" (___.__)__)")),
  });
}

ftxui::Element window_display::weather_window(const std::wstring &name,
                                              size_t index) {
  using namespace ftxui;
  Elements params;
  for (const auto &[a, b] : data_["hourly"].items()) {
    if (a == "time" || a == "weather_code") {
      continue;
    }
    auto value = b[index].is_number() ? std::to_string(b[index].get<int>())
                                      : b[index].get<std::string>();
    params.push_back(GetNormalName(a, value));
  }

  auto content = hbox({GetWeather(data_["hourly"]["weather_code"][index]),
                       separatorLight(), vbox(params)});
  return window(text(name) | hcenter, content);
};

ftxui::Element window_display::GetDayInfo(std::size_t index) {
  using namespace ftxui;
  return vbox({text(time_[index * 24].get<std::string>().substr(0, 10)) |
                   hcenter | borderHeavy | color(Color::LightSkyBlue3),
               hbox({
                   weather_window(L"朝 Утро", index * 24 + 10) | flex |
                       color(Color::LightCoral),
                   weather_window(L"日 День", index * 24 + 14) | flex |
                       color(Color::LightSkyBlue1),
                   weather_window(L"夕べ Вечер", index * 24 + 18) | flex |
                       color(Color::DeepSkyBlue1),
                   weather_window(L"夜 ночь", index * 24 + 24 + 5) | flex |
                       color(Color::DeepSkyBlue4Bis),
               })});
}

void window_display::Regenerate() {
  using namespace ftxui;
  data_ = manager.GetWeather();
  flicky::Loger::CheckLog();
  time_ = data_["hourly"]["time"];

  auto city_vbox = window(text(""), vbox(text(manager.GetCity()) | hcenter |
                                         color(Color::DarkOrange))) |
                   flex | hcenter;
  auto old_size = days.size();
  days = {city_vbox};
  for (std::size_t i = 0;
       i < (old_size == 0 ? days_default_count_ : old_size - 1); ++i) {
    days.push_back(GetDayInfo(i));
  }
}

int window_display::Run() {
  using namespace ftxui;

  Regenerate();
  auto screen = ScreenInteractive::FitComponent();

  auto component = Renderer(
      [&] { return vbox(days | size(WIDTH, ftxui::GREATER_THAN, 80)); });

  component |= CatchEvent([&](Event event) {
    if (event == Event::Escape) {
      screen.Exit();
    }
    if (event.input() == "-" && days.size() > 1) {
      days.pop_back();
    }
    if (event.input() == "+") {
      if (days.size() < time_.size() / 24) {
        days.push_back(GetDayInfo(days.size() - 1));
      }
    }
    if (event.input() == "n") {
      manager.ChangeCity(1);
      Regenerate();
    }
    if (event.input() == "p") {
      manager.ChangeCity(-1);
      Regenerate();
    }
    if (event == Event::Custom) {
      Regenerate();
    }
    return true;
  });

  Loop loop(&screen, component);

  std::thread([&]() {
    while (!loop.HasQuitted()) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(manager.GetRefreshRate()));
      screen.PostEvent(Event::Custom);
    }
  }).detach();

  while (!loop.HasQuitted()) {
    loop.RunOnce();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  return EXIT_SUCCESS;
}
