#include <pthread.h>

#include <filesystem>
#include <iostream>
#include <optional>
#include <span>
#include <string>

using namespace std;
namespace fs = filesystem;

struct PrintOptions {
  optional<size_t> max_depth{};
  fs::path dir_to_print{fs::current_path()};
};

static constexpr string_view PATH_ARGUMENT_BEGIN = "--path=";

PrintOptions create_print_options(span<char *> arguments) {
  PrintOptions print_options;
  size_t i = 1;
  while (i < arguments.size()) {
    string argument = arguments[i];
    i++;
    if (argument == "-L") {
      if (i == arguments.size()) {
        cerr << "have'nt specified the depth number";
        continue;
      }
      string number_argument = arguments[i];
      i++;
      try {
        print_options.max_depth = stoul(number_argument);
      } catch (...) {
        cerr << "used alphabet instead of specifying number";
        continue;
      }
    } else if (argument.starts_with(PATH_ARGUMENT_BEGIN)) {
      fs::path path = argument.substr(PATH_ARGUMENT_BEGIN.size());
      if (!fs::exists(path)) {
        cerr << "please provide a valid path";
        continue;
      } else {
        print_options.dir_to_print = path;
      }
    }
  }
  return print_options;
}

int main(int argc, char **argv) {
  auto print_options{create_print_options({argv, static_cast<size_t>(argc)})};
  cout << "----OPTIONS----\n";
  cout << "dir to print " << print_options.dir_to_print.filename() << "\n";
  cout << "max depth "
       << (print_options.max_depth.has_value()
               ? to_string(*print_options.max_depth)
               : "infinite")
       << "\n";
  return 0;
}
