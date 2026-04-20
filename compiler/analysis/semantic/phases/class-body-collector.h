#pragma once

#include <expected>
#include <string>
#include <unordered_set>

#include "compiler/compilation-structures/ast-visitor.h"
#include "compiler/compilation-structures/ast.h"
#include "compiler/compilation-structures/symbol-table.h"

namespace analysis::semantic::phases {

namespace details {

class class_body_collector : public ast::visitor {
public:
    class_body_collector(structures::symbol_table& symbol_table, structures::type_table& type_table)
        : program_symbol_table(symbol_table),
          program_type_table(type_table) {}

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
    structures::symbol_table& program_symbol_table;
    structures::type_table& program_type_table;
    structures::class_symbol* current_class = nullptr;
    std::string error_message{};

    std::unordered_set<std::string> field_names;
    std::unordered_set<std::string> method_names;
    std::unordered_set<std::string> constructor_signatures;
};

} // namespace details

inline std::expected<void, std::string> process_classes_content(const std::unique_ptr<ast::program>& program,
                                                                structures::symbol_table& program_symbol_table,
                                                                structures::type_table& program_type_table) {
    details::class_body_collector body_collector(program_symbol_table, program_type_table);
    program->accept(body_collector);
    return body_collector.get_result();
}

} // namespace analysis::semantic::phases
