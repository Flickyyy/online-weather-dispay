#include "lib/window_display.hpp"
#include "nlohmann/json_fwd.hpp"
#include <exception>
#include <iostream>

int main(int argc, char **argv) {
  try {
    window_display window;
    return window.Run();
  } catch (nlohmann::json::exception &sus) {
    std::cerr << "error json (mb config corrupted): " << sus.what() << '\n';
  } catch (std::exception &e) {
    std::cerr << "some exception occured: " << e.what() << '\n';
  }
}
