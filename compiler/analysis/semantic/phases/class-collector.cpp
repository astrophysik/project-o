#include "compiler/analysis/semantic/phases/class-collector.h"

#include <exception>
#include <format>
#include <memory>
#include <utility>

#include "compiler/compilation-structures/ast.h"
#include "compiler/compilation-structures/symbol-table.h"

namespace analysis::semantic::phases::details {

void class_collector::visit(ast::program& node) {
    for (auto& cls : node.classes) {
        cls->accept(*this);
    }
}

void class_collector::visit(ast::class_declaration& node) {
    if (program_symbol_table->lookup(node.name) != nullptr) {
        error_message += std::format("Class redefinition {}\n", node.name);
        return;
    }
    structures::class_symbol* base_class = nullptr;
    if (node.base_class.has_value()) {
        base_class = program_symbol_table->typed_lookup<structures::class_symbol>(*node.base_class);
        if (base_class == nullptr) {
            error_message += std::format("Class {} inheritance of undefined class {}\n", node.name, *node.base_class);
            return;
        }
    }
    auto sym{std::make_unique<structures::class_symbol>(node.name, program_symbol_table.get(), base_class)};
    program_symbol_table->add(std::move(sym));
    program_type_table->addClass(node.name, &node);
}

void class_collector::visit(ast::block& node) {}
void class_collector::visit(ast::variable_declaration& node) {}
void class_collector::visit(ast::parameter_declaration& node) {}
void class_collector::visit(ast::method_declaration& node) {}
void class_collector::visit(ast::constructor_declaration& node) {}
void class_collector::visit(ast::assignment_statement& node) {}
void class_collector::visit(ast::while_statement& node) {}
void class_collector::visit(ast::if_statement& node) {}
void class_collector::visit(ast::return_statement& node) {}
void class_collector::visit(ast::literal_expression& node) {}
void class_collector::visit(ast::this_expression& node) {}
void class_collector::visit(ast::identifier_expression& node) {}
void class_collector::visit(ast::parameterized_identifier_expression& node) {}
void class_collector::visit(ast::member_expression& node) {}
void class_collector::visit(ast::call_expression& node) {}
void class_collector::visit(ast::grouping_expression& node) {}

} // namespace analysis::semantic::phases::details
