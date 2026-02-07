#include "../inc/parser.hpp"
#include "../inc/program.hpp"
#include <fstream>
#include <iostream>

std::string read_file(const std::string& filename) {
    std::ifstream file(filename, file.ate);
    std::string str(file.tellg(), '\0');
    file.seekg(file.beg);
    file.read(str.data(), str.size());
    return str;
}

int main(int argc, const char* argv[]) {
    std::optional<std::string> input_filename;
    std::optional<std::string> output_filename;

    enum class NextArg {
        NONE,
        INPUT_FILENAME,
        OUTPUT_FILENAME,
    };

    NextArg next_arg = NextArg::NONE;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "-o" || arg == "--output")
            next_arg = NextArg::OUTPUT_FILENAME;
        else if (arg == "-i" || arg == "--input")
            next_arg = NextArg::INPUT_FILENAME;
        else { 
            switch (next_arg) {
            case NextArg::INPUT_FILENAME:
                if (input_filename) {
                    std::cerr << "Error: input filename already specified.\n";
                    return EINVAL;
                }
                input_filename = arg;
                break;
            case NextArg::OUTPUT_FILENAME:
                if (output_filename) {
                    std::cerr << "Error: output filename already specified.\n";
                    return EINVAL;
                }
                output_filename = arg;
                break;
            default:
                std::cerr << "Warning: unknown argument \"" << arg << "\".\n";
                break;
            }
            next_arg = NextArg::NONE;
        }
    }

    if (!input_filename) {
        std::cerr << "Error: input filename not specified.\n";
        return EINVAL;
    }
    if (!output_filename) {
        std::cerr << "Error: output filename not specified.\n";
        return EINVAL;
    }

    const std::string file = read_file(*input_filename);
    Program program = parse(file);
    const std::vector<uint8_t> result = program.assemble();

    std::ofstream output(*output_filename, output.binary);
    output.write(reinterpret_cast<const char*>(result.data()), result.size());
}