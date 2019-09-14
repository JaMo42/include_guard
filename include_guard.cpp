#include <iostream>  // std::cout, std::cerr
#include <string>  // std::string
#include <fstream>  // std::ofstream
#include <cctype>  // std::isalpha

std::string get_macro_name(const std::string& file_name) {
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
    std::cout << "Creating include guards ..." << std::endl;
    for (unsigned i = 1; i < argc; i++) {
        std::string file_name(argv[i]);
        // Extract file name, if a path is given
#ifdef _WIN32
        const std::size_t last_delim = file_name.find_last_of('\\');
#else // _WIN32
        const std::size_t last_delim = file_name.find_last_of('/');
#endif // _WIN32
        if (last_delim != std::string::npos) {
            file_name = file_name.substr(last_delim + 1);
        }
        // Open file
        std::cout << file_name << std::endl;
        std::fstream file(argv[i]);
        if (file.is_open()) {
            // Write include guard
            const std::string macro_name = get_macro_name(file_name);
            file << "#ifndef " << macro_name << '\n';
            file << "#define " << macro_name << '\n';
            file << "#endif /* !" << macro_name << " */" << '\n';
            file.close();
        } else {
            std::cerr << "...could not be opened!" << std::endl;
            ++num_fails;
        }
    }
    std::cout << "Finished." << std::endl;
    if (num_fails) {
        std::cout << num_fails << " error" << (num_fails == 1 ? "!" : "s!") << std::endl;
    }
    return 1;
}

