#include <fstream>
#include <print>
#include <string>

#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::print("Usage: ./compiler <input_file>\n");
        return 1;
    }

    std::ifstream s(argv[1]);
    std::string file_content((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());

    auto tokens_res = lexer::tokenize_text(file_content);
    std::println("{}", tokens_res);
}
