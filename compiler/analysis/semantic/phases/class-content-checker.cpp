#include "class-content-checker.h"

#include "type-checker.h"

#include <cassert>

namespace analysis::semantic::phases::details {

void class_content_checker::visit(ast::program& node) {
    for (auto& cls : node.classes) {
        cls->accept(*this);
    }
}

void class_content_checker::visit(ast::class_declaration& node) {
    auto* cls = program_symbol_table.typed_lookup<class_symbol>(node.name);
    assert(cls != nullptr);
    current_scope = cls->class_scope.get();
    current_class_name_ = node.name;
    for (const auto& field : node.fields)
        field->accept(*this);
}

void class_content_checker::visit(ast::variable_declaration& node) {
    auto result = inferrer_.infer(node.initializer.get(), *current_scope);
    if (!result) {
        error_message += std::format("Field '{}': {}\n", node.name, result.error());
        return;
    }
    auto* sym = current_scope->typed_lookup<variable_symbol>(node.name);
    if (sym) sym->type = *result;
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
