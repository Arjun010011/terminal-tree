#include <pthread.h>

#include <filesystem>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

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

struct File {
  fs::path path;
};

struct Dir {
  fs::path path;
};
struct Root {
  fs::path path;
};
using PrintableFileObject = variant<File, Dir, Root>;

class FileObjectPrinter {
public:
  void operator()(const File &file) { cout << file.path.filename() << "\n"; }
  void operator()(const Dir &dir) {
    cout << dir.path.filename() << "\n";
    print_children(dir.path);
  }
  void operator()(const Root &root) {
    cout << ".\n";
    print_children(root.path);
  }

private:
  void print_prefix(bool is_last) {
    for (bool has_mor_siblings : m_stack) {
      if (has_mor_siblings) {
        cout << "│   ";
      } else {
        cout << "    ";
      }
    }
    cout << (is_last ? "└── " : "├── ");
  }
  void print_children(const fs::path &path) {
    vector<fs::path> children;
    for (const auto &child : fs::directory_iterator(path)) {
      children.push_back(child);
    }
    for (size_t i{}; i < children.size(); i++) {
      bool is_last{i + 1 == children.size()};
      print_prefix(is_last);
      m_stack.push_back(!is_last);
      visit(*this, create_printable(children[i]));
      m_stack.pop_back();
    }
  }
  PrintableFileObject create_printable(const fs::path &path) {
    if (fs::is_directory(path)) {

      return Dir{path};
    }
    return File{path};
  }

private:
  vector<bool> m_stack;
};

int main(int argc, char **argv) {
  auto print_options{create_print_options({argv, static_cast<size_t>(argc)})};
  cout << "----OPTIONS----\n";
  cout << "dir to print " << print_options.dir_to_print.filename() << "\n";
  cout << "max depth "
       << (print_options.max_depth.has_value()
               ? to_string(*print_options.max_depth)
               : "infinite")
       << "\n";
  visit(FileObjectPrinter{},
        PrintableFileObject{Root(print_options.dir_to_print)});
  return 0;
}
