#include "compiler/parser/parser.h"

#include <format>
#include <memory>
#include <vector>

#include "compiler/ast/ast.h"
#include "compiler/lexer/token.h"
#include "parser.h"

namespace parser {

parser::parser(std::vector<lexer::token> _tokens)
    : tokens(std::move(_tokens)) {}

void parser::advance() {
    if (current < tokens.size())
        ++current;
}

void parser::retreat() {
    if (current > 0)
        --current;
}

void parser::skip_newlines() {
    while (check(lexer::token_type::tok_new_line)) {
        advance();
    }
}

bool parser::check(lexer::token_type type) const {
    // if (current >= tokens.size()) return false;
    return tokens[current].type == type;
}

bool parser::match(lexer::token_type type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

lexer::token parser::peek() const {
    if (current >= tokens.size())
        return tokens.back(); // tok_eof
    return tokens[current];
}

lexer::token parser::previous() const {
    if (current == 0)
        return tokens[0];
    return tokens[current - 1];
}

lexer::token parser::consume(lexer::token_type expected, std::string_view message) {
    if (check(expected)) {
        auto tok = peek();
        advance();
        return tok;
    }
    throw parse_error(std::format("{} at line {} pos {}:{}", message, peek().span.line_num, peek().span.start_pos, peek().span.end_pos));
}

std::unique_ptr<ast::program> parser::parse() {
    return parse_program();
}

std::unique_ptr<ast::program> parser::parse_program() {
    auto program = std::make_unique<ast::program>();
    skip_newlines();
    while (not check(lexer::token_type::tok_eof)) {
        // class
        consume(lexer::token_type::tok_kw_class, "Expected 'class' ");

        program->classes.push_back(parse_class_declaration());
        skip_newlines();
    }
    return program;
}

std::unique_ptr<ast::class_declaration> parser::parse_class_declaration() {
    auto class_decl = std::make_unique<ast::class_declaration>();

    // class name
    auto name_tok = consume(lexer::token_type::tok_identifier, "Expected class name");
    class_decl->name = std::get<std::string>(name_tok.value);

    // extends
    if (match(lexer::token_type::tok_kw_extends)) {
        // base class name
        auto base_tok = consume(lexer::token_type::tok_identifier, "Expected base class name");
        class_decl->base_class = std::get<std::string>(base_tok.value);
        // skip_newlines();
    }

    // is
    consume(lexer::token_type::tok_kw_is, "Expected 'is' after class header");
    skip_newlines();

    // members
    while (!check(lexer::token_type::tok_kw_end) && !check(lexer::token_type::tok_eof)) {
        auto member = parse_member_expression();

        if (auto var_decl = dynamic_cast<ast::variable_declaration*>(member.get())) {
            member.release();
            class_decl->fields.push_back(std::unique_ptr<ast::variable_declaration>(var_decl));
        } else if (auto method_decl = dynamic_cast<ast::method_declaration*>(member.get())) {
            member.release();
            class_decl->methods.push_back(std::unique_ptr<ast::method_declaration>(method_decl));
        } else if (auto ctor_decl = dynamic_cast<ast::constructor_declaration*>(member.get())) {
            member.release();
            class_decl->constructors.push_back(std::unique_ptr<ast::constructor_declaration>(ctor_decl));
        }

        skip_newlines();
    }
    consume(lexer::token_type::tok_kw_end, "Expected 'end' to close class");
    return class_decl;
}

std::unique_ptr<ast::entity> parser::parse_member_expression() {
    skip_newlines();
    // var
    if (match(lexer::token_type::tok_kw_var)) {
        return parse_variable_declaration();
    }
    // method
    if (match(lexer::token_type::tok_kw_method)) {
        return parse_method_declaration();
    }
    // constructor
    if (match(lexer::token_type::tok_kw_this)) {
        return parse_constructor_declaration();
    }
    throw parse_error(std::format("Expected class member definition at line {} pos {}:{}", peek().span.line_num, peek().span.start_pos, peek().span.end_pos));
}

std::unique_ptr<ast::variable_declaration> parser::parse_variable_declaration() {
    auto var_decl = std::make_unique<ast::variable_declaration>();

    // var name
    auto name_tok = consume(lexer::token_type::tok_identifier, "Expected variable name");
    var_decl->name = std::get<std::string>(name_tok.value);

    // :
    consume(lexer::token_type::tok_colon, "Expected ':' after variable name");

    // expr
    var_decl->initializer = parse_expression();
    return var_decl;
}

std::vector<std::unique_ptr<ast::parameter_declaration>> parser::parse_parameters() {
    std::vector<std::unique_ptr<ast::parameter_declaration>> params;

    // (
    consume(lexer::token_type::tok_open_par, "Expected '(' after method name");

    if (not check(lexer::token_type::tok_close_par)) {
        do {
            params.push_back(parse_parameter());
        } while (match(lexer::token_type::tok_comma));
    }

    // )
    consume(lexer::token_type::tok_close_par, "Expected ')' after method name");

    return params;
}

std::unique_ptr<ast::parameter_declaration> parser::parse_parameter() {
    // parameter name
    auto name = consume(lexer::token_type::tok_identifier, "Expected parameter name");

    // :
    consume(lexer::token_type::tok_colon, "Expected ':'");

    // parameter type
    auto type = consume(lexer::token_type::tok_identifier, "Expected type name");
    return std::make_unique<ast::parameter_declaration>(std::get<std::string>(name.value), std::get<std::string>(type.value));
}

std::unique_ptr<ast::method_declaration> parser::parse_method_declaration() {
    auto method = std::make_unique<ast::method_declaration>();

    // method name
    auto name_tok = consume(lexer::token_type::tok_identifier, "Expected method name");
    method->name = std::get<std::string>(name_tok.value);

    // parameters
    method->parameters = parse_parameters();

    // return type
    if (match(lexer::token_type::tok_colon)) {
        auto ret_tok = consume(lexer::token_type::tok_identifier, "Expected return type identifier");
        method->return_type = std::get<std::string>(ret_tok.value);
        skip_newlines();
    }

    // is...end
    if (match(lexer::token_type::tok_kw_is)) {
        // method body
        auto body_items = parse_block();

        // end
        consume(lexer::token_type::tok_kw_end, "Expected 'end' after method body");
        method->body = std::move(body_items);
        // =>
    } else if (match(lexer::token_type::tok_fat_arrow)) {
        // expr
        auto expr = parse_expression();
        method->body = std::move(expr);
    } else {
        // forward method declaration
    }
    return method;
}

std::unique_ptr<ast::constructor_declaration> parser::parse_constructor_declaration() {
    auto ctor = std::make_unique<ast::constructor_declaration>();

    // parameters
    ctor->parameters = parse_parameters();

    // is
    consume(lexer::token_type::tok_kw_is, "Expected 'is' after constructor parameters");

    // constructor body
    ctor->body = parse_block();

    // end
    consume(lexer::token_type::tok_kw_end, "Expected 'end' after constructor body");
    return ctor;
}

std::unique_ptr<ast::block> parser::parse_block() {
    std::vector<std::unique_ptr<ast::entity>> items;
    while (true) {
        skip_newlines();
        if (match(lexer::token_type::tok_kw_var)) {
            // var
            items.push_back(parse_variable_declaration());
        } else if (auto statement{parse_statement()}; statement != nullptr) {
            // statements + call expr
            items.push_back(std::move(statement));
        } else {
            break;
        }
    }
    return std::make_unique<ast::block>(std::move(items));
}

std::unique_ptr<ast::entity> parser::parse_statement() {
    skip_newlines();
    if (match(lexer::token_type::tok_identifier) or match(lexer::token_type::tok_kw_this)) {
        // call class method (expression)
        if (check(lexer::token_type::tok_dot)) {
            retreat();
            return parse_expression();
        }
        // assign
        retreat();
        return parse_assignment();
    }
    if (match(lexer::token_type::tok_kw_while)) {
        // while
        return parse_while();
    }
    if (match(lexer::token_type::tok_kw_if)) {
        // if
        return parse_if();
    }
    if (match(lexer::token_type::tok_kw_return)) {
        // return
        return parse_return();
    }
    return nullptr;
}

std::unique_ptr<ast::assignment_statement> parser::parse_assignment() {
    // var name
    auto ident_tok = consume(lexer::token_type::tok_identifier, "Expected identifier in assignment");

    auto assign = std::make_unique<ast::assignment_statement>();
    assign->target = std::get<std::string>(ident_tok.value);

    // :=
    consume(lexer::token_type::tok_assignment, "Expected ':=' in assignment");

    // expr
    assign->value = parse_expression();
    return assign;
}

std::unique_ptr<ast::while_statement> parser::parse_while() {
    auto while_stmt = std::make_unique<ast::while_statement>();

    // expr
    while_stmt->condition = parse_expression();

    // loop
    consume(lexer::token_type::tok_kw_loop, "Expected 'loop' after while condition");

    // block
    while_stmt->body = parse_block();

    // end
    consume(lexer::token_type::tok_kw_end, "Expected 'end' after while loop");
    return while_stmt;
}

std::unique_ptr<ast::if_statement> parser::parse_if() {
    auto if_stmt = std::make_unique<ast::if_statement>();

    // expr
    if_stmt->condition = parse_expression();

    // then
    consume(lexer::token_type::tok_kw_then, "Expected 'then' after if condition");

    // block
    if_stmt->true_branch = parse_block();

    // else
    if (match(lexer::token_type::tok_kw_else)) {
        // block
        if_stmt->false_branch = parse_block();
    }

    // end
    consume(lexer::token_type::tok_kw_end, "Expected 'end' after if statement");
    return if_stmt;
}

std::unique_ptr<ast::return_statement> parser::parse_return() {
    auto ret_stmt = std::make_unique<ast::return_statement>();

    if (not check(lexer::token_type::tok_kw_end) && not check(lexer::token_type::tok_new_line)) {
        // expr
        ret_stmt->value = parse_expression();
    }
    return ret_stmt;
}

std::unique_ptr<ast::expression> parser::parse_expression() {
    // primary
    auto expr = parse_primary();
    while (match(lexer::token_type::tok_dot)) {
        auto member_tok = consume(lexer::token_type::tok_identifier, "Expected member name after '.'");
        std::string member = std::get<std::string>(member_tok.value);

        // method call
        if (match(lexer::token_type::tok_open_par)) {
            // method arguments
            auto args = parse_arguments();

            // call expr
            auto member_expr = std::make_unique<ast::member_expression>(std::move(expr), member);
            expr = std::make_unique<ast::call_expression>(std::move(member_expr), std::move(args));
        } else {
            // field access
            expr = std::make_unique<ast::member_expression>(std::move(expr), member);
        }
        skip_newlines();
    }
    return expr;
}

std::unique_ptr<ast::expression> parser::parse_primary() {
    skip_newlines();
    if (match(lexer::token_type::tok_int)) {
        // int
        int64_t val = std::get<int>(previous().value);
        return std::make_unique<ast::literal_expression>(val);
    }
    if (match(lexer::token_type::tok_real)) {
        // real
        double val = std::get<double>(previous().value);
        return std::make_unique<ast::literal_expression>(val);
    }
    if (match(lexer::token_type::tok_kw_true)) {
        // true
        return std::make_unique<ast::literal_expression>(true);
    }
    if (match(lexer::token_type::tok_kw_false)) {
        // false
        return std::make_unique<ast::literal_expression>(false);
    }
    if (match(lexer::token_type::tok_kw_this)) {
        // this
        return std::make_unique<ast::this_expression>();
    }
    if (match(lexer::token_type::tok_identifier)) {
        // identifier
        std::string id = std::get<std::string>(previous().value);
        auto expr = std::make_unique<ast::identifier_expression>(id);
        if (match(lexer::token_type::tok_open_par)) {
            auto args = parse_arguments();
            auto member_expr = std::make_unique<ast::member_expression>(std::move(expr), id);
            return std::make_unique<ast::call_expression>(std::move(member_expr), std::move(args));
        } else {
            return expr;
        }
    }

    // prioritization
    if (match(lexer::token_type::tok_open_par)) {
        auto inner = parse_expression();
        consume(lexer::token_type::tok_close_par, "Expected ')' after grouped expression");
        return std::make_unique<ast::grouping_expression>(std::move(inner));
    }
    throw parse_error(std::format("Unexpected token at line {} pos {}:{}", peek().span.line_num, peek().span.start_pos, peek().span.end_pos));
}

std::vector<std::unique_ptr<ast::expression>> parser::parse_arguments() {
    std::vector<std::unique_ptr<ast::expression>> args;

    if (not check(lexer::token_type::tok_close_par)) {
        do {
            // expr
            args.push_back(parse_expression());
        } while (match(lexer::token_type::tok_comma));
    }

    consume(lexer::token_type::tok_close_par, "Expected ')' after arguments");
    return args;
}

std::vector<std::string> parser::parse_type_arguments() {
    // TODO: only if u wanna [] for collections
    return {};
}

} // namespace parser
