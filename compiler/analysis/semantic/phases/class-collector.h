#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include "compiler/compilation-structures/ast-visitor.h"
#include "compiler/compilation-structures/ast.h"
#include "compiler/compilation-structures/symbol-table.h"

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

    std::pair<std::unique_ptr<structures::symbol_table>, std::unique_ptr<structures::type_table>> get_result() {
        if (!error_message.empty()) {
            throw std::runtime_error{error_message};
        }
        return std::pair<std::unique_ptr<structures::symbol_table>, std::unique_ptr<structures::type_table>>{std::move(program_symbol_table),
                                                                                                             std::move(program_type_table)};
    }

private:
    std::string error_message{};
    std::unique_ptr<structures::symbol_table> program_symbol_table{std::make_unique<structures::symbol_table>(nullptr)};
    std::unique_ptr<structures::type_table> program_type_table{std::make_unique<structures::type_table>()};
};

} // namespace details

inline std::pair<std::unique_ptr<structures::symbol_table>, std::unique_ptr<structures::type_table>>
collect_program_classes(const std::unique_ptr<ast::program>& program) {
    details::class_collector class_collector;
    program->accept(class_collector);

    return std::move(class_collector.get_result());
}
} // namespace analysis::semantic::phases
