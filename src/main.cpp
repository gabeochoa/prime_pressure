#ifdef BACKWARD
#include "backward/backward.hpp"
namespace backward {
backward::SignalHandling sh;
} // namespace backward
#endif

#include "argh.h"
#include "game.h"
#include "preload.h"
#include "settings.h"

using namespace afterhours;

int main(int argc, char *argv[]) {
  argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

  if (cmdl["--help"]) {
    std::cout << "Prime Pressure\n\n";
    std::cout << "Usage: warehouse_game [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help                        Show this help message\n";
    std::cout
        << "  -w, --width <pixels>          Screen width (default: 1280)\n";
    std::cout
        << "  -h, --height <pixels>         Screen height (default: 720)\n";
    return 0;
  }

  int screenWidth, screenHeight;
  cmdl({"-w", "--width"}, 1280) >> screenWidth;
  cmdl({"-h", "--height"}, 720) >> screenHeight;

  Settings::get().load_save_file(screenWidth, screenHeight);

  Preload::get() //
      .init("Prime Pressure")
      .make_singleton();
  Settings::get().refresh_settings();

  game();

  Settings::get().write_save_file();

  return 0;
}
