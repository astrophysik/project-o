#pragma once

#include <expected>
#include <string>
#include <unordered_map>

#include "compiler/analysis/semantic/symbol-table.h"
#include "compiler/ast/ast-visitor.h"
#include "compiler/ast/ast.h"

namespace analysis::semantic::phases {

namespace details {

class class_collector : public ast::visitor {
public:
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

    std::expected<std::unique_ptr<analysis::semantic::symbol_table>, std::string> get_result() {
        if (!error_message.empty()) {
            return std::unexpected{error_message};
        }
        return std::move(program_symbol_table);
    }

private:
    std::string error_message{};
    std::unique_ptr<symbol_table> program_symbol_table{std::make_unique<symbol_table>(nullptr)};
};

} // namespace details

inline std::expected<std::unique_ptr<symbol_table>, std::string> collect_program_classes(const std::unique_ptr<ast::program>& program) {
    details::class_collector class_collector;
    program->accept(class_collector);

    return std::move(class_collector.get_result());
}
} // namespace analysis::semantic::phases
