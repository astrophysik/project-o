#pragma once

#include <stdexcept>
#include <string>

#include "compiler/analysis/semantic/error.h"
#include "compiler/compilation-structures/ast/parsing/ast-visitor.h"
#include "compiler/compilation-structures/ast/parsing/ast.h"
#include "compiler/compilation-structures/symbol-table.h"

namespace analysis::semantic::phases {

namespace details {

class class_field_checker : public ast::visitor {
public:
    class_field_checker(structures::symbol_table& symbol_table, structures::type_table& type_table, const error_formatter& errors)
        : program_symbol_table(symbol_table), program_type_table(type_table), errors(errors) {}

    void visit(ast::program& node) override;
    void visit(ast::block& node) override;
    void visit(ast::class_declaration& node) override;
    void visit(ast::variable_declaration& node) override;
    void visit(ast::parameter_declaration& node) override;
    void visit(ast::method_declaration& node) override;
    void visit(ast::constructor_declaration& node) override;
    void visit(ast::assignment_statement& node) override;
    void visit(ast::while_statement& node) override;
    void visit(ast::if_statement& node) override;
    void visit(ast::return_statement& node) override;
    void visit(ast::literal_expression& node) override;
    void visit(ast::this_expression& node) override;
    void visit(ast::identifier_expression& node) override;
    void visit(ast::member_expression& node) override;
    void visit(ast::call_expression& node) override;
    void visit(ast::grouping_expression& node) override;

    void get_result() {
        if (!error_message.empty()) {
            throw std::runtime_error{error_message};
        }
    }

private:
    std::string error_message{};
    structures::symbol_table& program_symbol_table;
    structures::type_table& program_type_table;
    const error_formatter& errors;
    structures::class_symbol* current_class_symbol = nullptr;
};

} // namespace details

inline void check_field_content(const std::unique_ptr<ast::program>& program, structures::symbol_table& symbol_table, structures::type_table& type_table, const error_formatter& errors) {
    details::class_field_checker class_content_checker(symbol_table, type_table, errors);
    program->accept(class_content_checker);
    class_content_checker.get_result();
}

} // namespace analysis::semantic::phases
