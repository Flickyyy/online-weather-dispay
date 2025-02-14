#include "weather_manager.hpp"
#include "loger.hpp"

// implementation

int weather_manager::GetRefreshRate() { return config_["refresh-rate"]; }
void weather_manager::check_on_default(const std::string &arg,
                                       const std::string &default_value) {
  if (config_[arg] == nlohmann::detail::value_t::null) {
    config_[arg] = default_value;
  }
}

void weather_manager::check_on_default(const std::string &arg,
                                       const int &default_value) {
  if (config_[arg] == nlohmann::detail::value_t::null) {
    config_[arg] = default_value;
  }
}

void weather_manager::check_on_default(
    const std::string &arg,
    const std::initializer_list<std::string> &default_value) {
  if (config_[arg] == nlohmann::detail::value_t::null) {
    config_[arg] = default_value;
  }
}

std::string weather_manager::GetCity() {
  return config_["cities"][city_ind_ % config_["cities"].size()];
}
void weather_manager::ChangeCity(int offset) {
  if (offset == -1 && city_ind_ == 0) {
    return;
  }
  if (offset == 1 && city_ind_ + 1 == config_["cities"].size()) {
    return;
  }
  city_ind_ += offset;
}

std::pair<std::string, std::string>
weather_manager::GetLocation(const std::string &city) {
  cpr::Response r = cpr::Get(cpr::Url{"https://api.api-ninjas.com/v1/city"},
                             cpr::Parameters{{"name", city}},
                             cpr::Header{{"X-Api-Key", config_["X-Api-Key"]}});
  nlohmann::json data = nlohmann::json::parse(r.text);
  data = data[0];

  if (r.status_code != 200) {
    flicky::Loger::Report(
        "bad response status. Get request response status must be equal 200");
    return {"", ""};
  }

  return {std::to_string(data["latitude"].get<double>()),
          std::to_string(data["longitude"].get<double>())};
}

void weather_manager::CheckCoordinates(const std::string &city) {
  if (config_["location"][city] == nlohmann::detail::value_t::null) {
    auto [latitude, longitude] = GetLocation(city);
    config_["location"][city]["latitude"] = latitude;
    config_["location"][city]["longitude"] = longitude;
  }
}

weather_manager::weather_manager(const std::string &cfg_path)
    : config_path_(cfg_path) {
  std::ifstream file(config_path_);
  try {
    config_ = nlohmann::json::parse(file);
  } catch (...) {
    // then config will be regenerated
  }

  check_on_default("refresh-rate", 12500);
  check_on_default("hourly", "temperature_2m,relative_humidity_2m,wind_speed_"
                             "10m,weather_code,visibility");
  check_on_default("timezone", "Europe/Moscow");
  check_on_default("forecast_days", "4");
  check_on_default("cities", {"Moscow", "Florida", "Boston"});

  if (config_["X-Api-Key"] == nlohmann::detail::value_t::null) {
    flicky::Loger::Report("X-Api-Key for api ninjas isn't in config file and "
                          "program cant be running");
  }
  flicky::Loger::CheckLog();

  for (auto city : config_["cities"]) {
    CheckCoordinates(city);
  }
}

const nlohmann::json weather_manager::GetWeather() {
  cpr::Response r =
      cpr::Get(cpr::Url{"https://api.open-meteo.com/v1/forecast"},
               cpr::Parameters{
                   {"latitude", config_["location"][GetCity()]["latitude"]},
                   {"longitude", config_["location"][GetCity()]["longitude"]},
                   {"hourly", config_["hourly"]},
                   {"timezone", config_["timezone"]},
                   {"forecast_days", config_["forecast_days"]}});
  if (r.status_code != 200) {
    flicky::Loger::Report(
        "bad response status. Get request response status must be equal 200");
    return nlohmann::json();
  }
  return nlohmann::json::parse(r.text);
}

weather_manager::~weather_manager() {
  // Save cities latitude and longitude for decreasing get requests
  std::ofstream out(config_path_);
  out << std::setw(4) << config_;
}
