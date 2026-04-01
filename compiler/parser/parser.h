#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "compiler/ast/ast.h"
#include "compiler/lexer/token.h"

namespace parser {

class parse_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class parser {
public:
    explicit parser(std::vector<lexer::token> tokens);

    std::unique_ptr<ast::program> parse();

private:
    std::vector<lexer::token> tokens;
    size_t current{0};

    void advance();
    void retreat();
    void skip_newlines();
    bool check(lexer::token_type type) const;
    bool match(lexer::token_type type);
    lexer::token peek() const;
    lexer::token previous() const;
    lexer::token consume(lexer::token_type expected, std::string_view message);

    std::unique_ptr<ast::program> parse_program();
    std::unique_ptr<ast::block> parse_block();
    std::unique_ptr<ast::class_declaration> parse_class_declaration();
    std::unique_ptr<ast::entity> parse_member_expression();
    std::unique_ptr<ast::variable_declaration> parse_variable_declaration();
    std::unique_ptr<ast::method_declaration> parse_method_declaration();
    std::vector<std::unique_ptr<ast::parameter_declaration>> parse_parameters();
    std::unique_ptr<ast::parameter_declaration> parse_parameter();
    std::unique_ptr<ast::constructor_declaration> parse_constructor_declaration();
    std::unique_ptr<ast::entity> parse_statement();
    std::unique_ptr<ast::assignment_statement> parse_assignment();
    std::unique_ptr<ast::while_statement> parse_while();
    std::unique_ptr<ast::if_statement> parse_if();
    std::unique_ptr<ast::return_statement> parse_return();
    std::unique_ptr<ast::expression> parse_expression();
    std::unique_ptr<ast::expression> parse_primary();
    std::vector<std::unique_ptr<ast::expression>> parse_arguments();
    std::vector<std::string> parse_type_arguments();
};

} // namespace parser
