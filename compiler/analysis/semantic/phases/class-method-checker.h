#pragma once

#include <stdexcept>
#include <string>

#include "compiler/compilation-structures/symbol-table.h"
#include "compiler/compilation-structures/ast-visitor.h"
#include "compiler/compilation-structures/ast.h"

namespace analysis::semantic::phases {

namespace details {

class class_method_checker : public ast::visitor {
public:
    explicit class_method_checker(structures::symbol_table& symbol_table, structures::type_table& type_table)
        : program_symbol_table(symbol_table), program_type_table(type_table) {}

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
    void visit(ast::parameterized_identifier_expression& node) override;
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
    std::unique_ptr<structures::symbol_table> current_symbol_table = nullptr;
    structures::class_symbol* current_class_symbol = nullptr;
    const structures::type* method_return_type = nullptr;
};

} // namespace details

inline void check_method_content(const std::unique_ptr<ast::program>& program, structures::symbol_table& symbol_table, structures::type_table& type_table) {
    details::class_method_checker check_method_content(symbol_table, type_table);
    program->accept(check_method_content);
    check_method_content.get_result();
}

} // namespace analysis::semantic::phases
