#include "class-content-checker.h"

#include "type-checker.h"

#include <cassert>
#include <format>

namespace analysis::semantic::phases::details {


void class_content_checker::visit(ast::program& node) {
    for (auto& cls : node.classes) {
        cls->accept(*this);
    }
}

void class_content_checker::visit(ast::class_declaration& node) {
    auto* cls = program_symbol_table.typed_lookup<structures::class_symbol>(node.name);
    assert(cls != nullptr);
    current_class_symbol = cls;
    for (const auto& field : node.fields) {
        field->accept(*this);
    }
}

void class_content_checker::visit(ast::variable_declaration& node) {
    try {
        auto type_res = structures::type::inferExpressionType(node.initializer.get(), {&program_type_table, current_class_symbol});
        auto * symbol = current_class_symbol->class_scope->typed_lookup<structures::variable_symbol>(node.name);
        symbol->type = type_res;
    } catch (const std::runtime_error& e) {
        error_message += e.what();
    }
}

void class_content_checker::visit(ast::block& node) {}
void class_content_checker::visit(ast::parameter_declaration& node) {}
void class_content_checker::visit(ast::method_declaration& node) {}
void class_content_checker::visit(ast::constructor_declaration& node) {}
void class_content_checker::visit(ast::assignment_statement& node) {}
void class_content_checker::visit(ast::while_statement& node) {}
void class_content_checker::visit(ast::if_statement& node) {}
void class_content_checker::visit(ast::return_statement& node) {}
void class_content_checker::visit(ast::literal_expression& node) {}
void class_content_checker::visit(ast::this_expression& node) {}
void class_content_checker::visit(ast::identifier_expression& node) {}
void class_content_checker::visit(ast::parameterized_identifier_expression& node) {}
void class_content_checker::visit(ast::member_expression& node) {}
void class_content_checker::visit(ast::call_expression& node) {}
void class_content_checker::visit(ast::grouping_expression& node) {}

} // namespace analysis::semantic::phases::details
