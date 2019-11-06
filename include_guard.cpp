#include <iostream>  // for std::cout, std::cerr
#include <string>  // for std::string
#include <fstream>  // for std::ofstream
#include <cctype>  // for std::isalpha
#include <string_view>  // for std::string_view
#include <filesystem>  // for std::filesystem::exists, std::filesystem::path
namespace fs = std::filesystem;

std::string get_macro_name(std::string_view file_name) {
    std::string macro_name;
    macro_name.reserve(file_name.length());
    for (const char& character : file_name) {
        if (std::isalnum(character)) {
            macro_name += toupper(character);
        } else {
            macro_name += '_';
        }
    }
    return macro_name;
}

int main(const int argc, const char **argv) {
    if (argc == 1) {
        std::cout << argv[0] << ": No files specified." << std::endl;
        return 1;
    }
    // Number of files that could not be opened / written to
    unsigned num_fails = 0;
    std::cout << "Creating include guards..." << std::endl;
    for (unsigned i = 1; i < argc; i++) {
        std::string_view file_name(argv[i]);
        fs::path file_path(file_name);
        // Extract file name, if a path is given
#ifdef _WIN32
        const std::size_t last_delim = file_name.find_last_of('\\');
#else
        const std::size_t last_delim = file_name.find_last_of('/');
#endif
        if (last_delim != std::string::npos) {
            file_name = file_name.substr(last_delim + 1);
        }
        std::cout << file_name << std::endl;
        // Check if the file exists
        if (fs::exists(file_path)) {
            // Open file
            std::fstream file(argv[i], std::ios::out);
            if (file.is_open()) {
                // Write include guard
                const std::string macro_name = get_macro_name(file_name);
                file << "#ifndef " << macro_name << '\n';
                file << "#define " << macro_name << '\n';
                file << "#endif /* !" << macro_name << " */\n";
                file.close();
            } else {
                std::cerr << " could not be opened" << std::endl;
                ++num_fails;
            }
        } else {
            std::cout << " does not exist" << std::endl;
            ++num_fails;
        }
    }
    std::cout << "Finished." << std::endl;
    if (num_fails) {
        std::cout << num_fails << " error" << (num_fails == 1 ? ' ' : 's') << std::endl;
    }
    return 1;
}

