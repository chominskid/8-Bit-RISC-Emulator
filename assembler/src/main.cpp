#include "../inc/parser.hpp"
#include "../inc/program.hpp"
#include <format>
#include <fstream>
#include <iostream>

std::string read_file(const std::string& filename) {
    std::ifstream file(filename, file.ate);
    std::string str(file.tellg(), '\0');
    file.seekg(file.beg);
    file.read(str.data(), str.size());
    return str;
}

void print_origin(const std::string& str, Origin origin) {
    static constexpr size_t MAX_LINE_LEN = 80;

    size_t start = str.find_last_of('\n', origin.start);
    if (start != str.npos) start += 1;
    else start = 0;

    size_t line = 0;
    for (size_t i = 0; i < start; ++i) {
        if (str[i] == '\n')
            ++line;
    }

    size_t end = str.find_first_of('\n', origin.end);
    if (end == str.npos) end = str.size();

    std::vector<std::string> lines;
    size_t n = 0;
    for (size_t i = start; i < end; ++i) {
        if (lines.size() == 0 || str[i] == '\n') {
            lines.emplace_back(std::format("{}: ", ++line));
            lines.emplace_back(lines.back().size(), ' ');
            if (str[i] == '\n')
                continue;
        } else if (n == MAX_LINE_LEN) {
            n = 0;
            lines.insert(lines.end(), 2, "    ");
            continue;
        }
        lines[lines.size() - 2].push_back(str[i]);
        char underline = (i >= origin.start && i < origin.end) ? '~' : ' ';
        lines[lines.size() - 1].push_back(underline);
    }

    for (const std::string& line: lines) {
        std::cout << line << '\n';
    }
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
    try {
        Program program = parse(file);
        const MemoryMap output = program.assemble();
        output.write(*output_filename);
    } catch (const AssemblerError& error) {
        print_origin(file, error.origin);
        std::cout << error.what() << '\n';
        return 0;
    }
}