#pragma once

#include <expected>

#include "compiler/compilation-structures/symbol-table.h"
#include "compiler/compilation-structures/ast-visitor.h"
#include "compiler/compilation-structures/ast.h"

namespace analysis::semantic::phases {

namespace details {

class class_content_checker : public ast::visitor {
public:
    explicit class_content_checker(structures::symbol_table& t)
        : program_symbol_table(t) {}

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

    std::expected<void, std::string> get_result() {
        if (!error_message.empty()) {
            return std::unexpected{error_message};
        }
        return {};
    }

private:
    std::string error_message{};
    structures::symbol_table& program_symbol_table;
    structures::symbol_table* current_scope = nullptr;
};

} // namespace details

inline std::expected<void, std::string> check_classes_content(const std::unique_ptr<ast::program>& program, structures::symbol_table& program_table) {
    details::class_content_checker class_content_checker(program_table);
    program->accept(class_content_checker);
    return class_content_checker.get_result();
}

} // namespace analysis::semantic::phases
