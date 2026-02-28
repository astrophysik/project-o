#include <fstream>
#include <print>
#include <string>

#include "compiler/lexer/lexer.h"

// General todo:
// - add parse tok_int, tok_real (int.int) , tok_bool, others
// - add parse names, keywords (parse name => after check is name is keyword and
// which by swich)
// - now token value is just string. Maybe it should be redone by variant or
// virtual inheritance

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
