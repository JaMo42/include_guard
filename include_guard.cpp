#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <experimental/iterator>  // make_ostream_joiner
using namespace std::literals;
namespace fs = std::filesystem;

enum TriState { Yes, No, Undefined };

enum ArgumentError { Success, Invalid, NoArg, ExtraArg };

// Consider these extensions as C++ files, use `#pragma once` by default for
// them
std::array kCppExtensions = { ".hh"sv, ".hpp"sv, ".hxx"sv };

namespace Options
{
const char *lib_prefix = nullptr;
bool keep_full_path = false;
TriState use_pragma_override = TriState::Undefined;
bool force = false;
}

bool G_did_skip_because_not_empty = false;

void
get_macro_name (const fs::path &path, std::string &macro_name)
{
  const std::string file_name = (Options::keep_full_path
                                 ? path.generic_string ()
                                 : path.filename ().generic_string ());
  macro_name.clear ();
  if (Options::lib_prefix)
    {
      macro_name.append (Options::lib_prefix);
      macro_name.push_back ('_');
    }
  for (auto c : file_name)
    {
      if (c & 0x80)
        {
          // Leave unicode untouched
          macro_name.push_back (c);
        }
      else
        {
          if (std::isalnum (c))
            macro_name.push_back (std::toupper (c));
          else
            macro_name.push_back ('_');
        }
    }
}

void
process_file (const fs::path &path)
{
  const std::string path_str = path.generic_string ();
  if (!fs::exists (path))
    {
      std::cout << path_str << ": file does not exist\n";
      return;
    }
  if (!fs::is_regular_file (path))
    {
      std::cout << path_str << ": not a regular file (skipping)\n";
      return;
    }
  if (!fs::is_empty (path) && !Options::force)
    {
      G_did_skip_because_not_empty = true;
      std::cout << path_str << ": not empty (skipping)\n";
      return;
    }

  const bool use_pragma = (Options::use_pragma_override != TriState::Undefined
                           ? Options::use_pragma_override == TriState::Yes
                           : std::any_of (kCppExtensions.begin (),
                                          kCppExtensions.end (),
                                          [&path](std::string_view ext) {
                                            return path.extension () == ext;
                                          }));
  std::ofstream stream (path_str);
  static std::string macro_name = {};

  std::cout << path_str << std::endl;
  if (use_pragma)
    stream << "#pragma once\n";
  else
    {
      get_macro_name (path, macro_name);
      stream << "#ifndef " << macro_name << '\n';
      stream << "#define " << macro_name << '\n';
      stream << "#endif /* " << macro_name << " */\n";
    }
}

ArgumentError
handle_short_option (char flag)
{
  switch (flag)
    {
      break; case 'p': Options::use_pragma_override = TriState::Yes;
      break; case 'P': Options::use_pragma_override = TriState::No;
      break; case 'f': Options::force = true;
      break; case 'F': Options::keep_full_path = true;
      break; default: return ArgumentError::Invalid;
    }
  return ArgumentError::Success;
}

ArgumentError
handle_long_option (std::string_view arg, std::string_view &option_name_out)
{
  const std::size_t eq_pos = arg.find ('=');
  const bool has_value = eq_pos != std::string_view::npos;
  const std::string_view option = arg.substr (0, eq_pos);
  const std::string_view value = has_value ? arg.substr (eq_pos + 1) : "";

  option_name_out = option;

  if (option == "full-path"sv)
    {
      if (has_value)
        return ArgumentError::ExtraArg;
      Options::keep_full_path = true;
    }
  else if (option == "force"sv)
    {
      if (has_value)
        return ArgumentError::ExtraArg;
      Options::force = true;
    }
  else if (option == "pragma"sv)
    {
      if (has_value)
        return ArgumentError::ExtraArg;
      Options::use_pragma_override = TriState::Yes;
    }
  else if (option == "no-pragma"sv)
    {
      if (has_value)
        return ArgumentError::ExtraArg;
      Options::use_pragma_override = TriState::No;
    }
  else if (option == "lib-prefix"sv)
    {
      if (!has_value)
        return ArgumentError::NoArg;
      Options::lib_prefix = value.data ();
    }
  else
    return ArgumentError::Invalid;
  return ArgumentError::Success;
}

constexpr std::string_view
argument_error_text (ArgumentError err)
{
  switch (err)
    {
      break; case ArgumentError::Invalid:
        return "does not exist"sv;
      break; case ArgumentError::NoArg:
        return "requires an argument"sv;
      break; case ArgumentError::ExtraArg:
        return "does not take an argument"sv;
    }
  return {};
}

void
usage (const char *program)
{
  std::cout << "Usage: " << program << " [OPTION]... [FILE]...\n";
  std::cout << "Write a C/C++ include guard into the FILEs.\n";

  std::cout << "\nOptions:\n";
  std::cout << "  -f, --force      Overwrite non-empty files.\n";
  std::cout << "  -p, --pragma     Always use '#pragma once' instead of macros.\n";
  std::cout << "  -P, --no-pragma  Never use '#pragma once'.\n";
  std::cout << "  -F, --full-path  Use the entire given path to generate macro names instead\n";
  std::cout << "                     of just the file name.\n";
  std::cout << "      --lib-prefix=PREFIX\n";
  std::cout << "                   Add 'PREFIX_' in front of macro names.\n";

  std::cout << "\nBy default '#pragma once' is used for C++ files and macros for C files.\n";
  std::cout << "The extensions to classify a file as C++ are: ";
  std::copy (kCppExtensions.begin (), kCppExtensions.end (),
             std::experimental::make_ostream_joiner (std::cout, ", "));
  std::cout << ".\n";

  std::cout << "\nMacro names are derived from the file names by:\n";
  std::cout << "  1) Replacing alpha-numeric ascii characters with their upper-case variant.\n";
  std::cout << "  2) Replacing all other ascii characters with underscores.\n";
  std::cout << "  3) Copying all unicode characters literally." << std::endl;
}

int
main (const int argc, const char *argv[])
{
  std::vector<fs::path> files;

  auto usage_tip = [&argv]() {
    std::cerr << "Try '" << argv[0] << " --help' for more information\n";
  };

  for (int i = 1; i < argc; ++i)
    {
      if ("--help"sv == argv[i])
        {
          usage (argv[0]);
          return 0;
        }
      else if (argv[i][0] == '-')
        {
          if (argv[i][1] == '-')
            {
              std::string_view option_name;
              const auto result = handle_long_option (std::string_view (argv[i] + 2),
                                                      option_name);
              if (result != ArgumentError::Success)
                {
                  std::cerr << argv[0] << ": option ‘--" << option_name
                            << "’ " << argument_error_text (result) << '\n';
                  usage_tip ();
                  return 1;
                }
            }
          else
            {
              std::string_view flags {argv[i] + 1};
              for (auto c : flags)
                {
                  const auto result = handle_short_option (c);
                  if (result != ArgumentError::Success)
                    {
                      std::cerr << argv[0] << ": invalid option -- " << c << '\n';
                      usage_tip ();
                      return 1;
                    }
                }
            }
        }
      else
        files.emplace_back (argv[i]);
    }

  if (files.empty ())
    {
      std::cout << "No files.\n";
      return 0;
    }

  std::cout << "Generating include guards...\n";
  for (const auto &f : files)
    {
      process_file (f);
    }

  if (G_did_skip_because_not_empty)
    std::cout << "Note: run with -f/--force to overwrite non-empty files.\n";
}

