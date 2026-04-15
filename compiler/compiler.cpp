#include <fstream>
#include <print>
#include <string>

#include "compiler/analysis/print/ast-print.h"
#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser.h"
#include "compiler/analysis/semantic/semantic-check.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::print("Usage: ./compiler <input_file>\n");
        return 1;
    }

    std::ifstream s(argv[1]);
    std::string file_content((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());

    try {
        auto tokens_res = lexer::tokenize_text(file_content);
        auto parser = parser::parser(tokens_res);
        auto ast = parser.parse();
        analysis::semantic::check_program(ast);
    } catch (std::exception& e) {
        std::cerr << "Compilation error : \n" << e.what();
    }
}
