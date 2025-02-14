#pragma once
#include <cpr/cpr.h>
#include <cstddef>
#include <initializer_list>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

class weather_manager {
public:
  weather_manager() = delete;
  explicit weather_manager(const std::string &cfg_path);
  ~weather_manager();
  const nlohmann::json GetWeather();
  std::string GetCity();
  void ChangeCity(int offset);
  int GetRefreshRate();

private:
  void check_on_default(const std::string &arg,
                        const std::string &default_value);
  void check_on_default(const std::string &arg, const int &default_value);
  void
  check_on_default(const std::string &arg,
                   const std::initializer_list<std::string> &default_value);
  void CheckCoordinates(const std::string &city);
  std::pair<std::string, std::string> GetLocation(const std::string &city);

  std::string config_path_ = "config.json";
  int city_ind_ = 0;
  nlohmann::json config_;
};
