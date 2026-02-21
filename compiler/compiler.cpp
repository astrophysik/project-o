#include <print>

#include "compiler/lexer/lexer.h"

// General todo:
// - add read file to string
// - add parse tok_int, tok_real (int.int) , tok_bool
// - add parse names, keywords (parse name => after check is name is keyword and which by swich)
// - add skip comments
// - now token value is just string. Maybe it should be redone by variant or virtual inheritance

int main() {
   auto tokens = lexer::tokenize_text("(), \n=>");
   std::print("{}", tokens);
}