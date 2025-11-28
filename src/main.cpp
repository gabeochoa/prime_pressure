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
#include "testing/test_macros.h"
#include "testing/tests/all_tests.h"
#include <iostream>

using namespace afterhours;

int main(int argc, char *argv[]) {
  argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

  if (cmdl["--help"]) {
    std::cerr << "Prime Pressure\n\n";
    std::cerr << "Usage: warehouse_game [OPTIONS]\n\n";
    std::cerr << "Options:\n";
    std::cerr << "  --help                        Show this help message\n";
    std::cerr
        << "  -w, --width <pixels>          Screen width (default: 1280)\n";
    std::cerr
        << "  -h, --height <pixels>         Screen height (default: 720)\n";
    std::cerr << "  --list-tests                 List all available tests\n";
    std::cerr << "  --run-test <name>            Run a specific test\n";
    std::cerr
        << "  --slow-test                  Run test in slow mode (visible)\n";
    return 0;
  }

  if (cmdl["--list-tests"]) {
    TestRegistry &registry = TestRegistry::get();
    std::cerr << "Available tests:\n";
    for (const auto &[name, func] : registry.tests) {
      std::cerr << "  - " << name << "\n";
    }
    return 0;
  }

  std::string test_name;
  if (cmdl({"--run-test"}) >> test_name) {
    bool slow_mode = cmdl["--slow-test"];

    int screenWidth, screenHeight;
    cmdl({"-w", "--width"}, 1280) >> screenWidth;
    cmdl({"-h", "--height"}, 720) >> screenHeight;

    Settings::get().load_save_file(screenWidth, screenHeight);

    Preload::get() //
        .init("Prime Pressure")
        .make_singleton();
    Settings::get().refresh_settings();

    run_test(test_name, slow_mode);

    Settings::get().write_save_file();

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
