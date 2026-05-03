#include "compiler/analysis/semantic/phases/class-method-checker.h"

#include "type-checker.h"

#include <cassert>
#include <expected>
#include <format>
#include <variant>

namespace {
template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
} // namespace

namespace analysis::semantic::phases::details {

void class_method_checker::visit(ast::program& node) {
    for (auto& cls : node.classes) {
        cls->accept(*this);
    }
}

void class_method_checker::visit(ast::class_declaration& node) {
    auto* cls = program_symbol_table.typed_lookup<structures::class_symbol>(node.name);
    assert(cls != nullptr);
    current_class_symbol = cls;
    for (const auto& method : node.methods) {
        try {
            method->accept(*this);
        } catch (const std::exception& e) {
            error_message += std::format("semantic error on checking method {} : {}", method->name, e.what());
        }
    }
    for (const auto &constructor : node.constructors) {
        try {
            constructor->accept(*this);
        } catch (const std::exception& e) {
            error_message += std::format("semantic error on checking constructor {} : {}", cls->name, e.what());
        }

    }
}

void class_method_checker::visit(ast::method_declaration& node) {
    if (!node.body.has_value()) {
        return;
    }
    if (!node.return_type.has_value()) {
        error_message += "methods without return type are forbidden\n";
        return;
    }
    current_symbol_table = std::make_unique<structures::symbol_table>(current_class_symbol->class_scope.get());
    for (const auto& param : node.parameters) {
        const auto& param_name = param->name;
        const auto* param_type = program_type_table.resolveType(param->type_name);
        current_symbol_table->add(std::make_unique<structures::variable_symbol>(param_name, param_type));
    }
    const auto* return_type = program_type_table.resolveType(*node.return_type);
    const auto& method_body = *node.body;

    auto err = std::visit(
        overloaded{
            [&](const std::unique_ptr<ast::block>& body) -> std::optional<std::string> {
                method_return_type = return_type;
                body->accept(*this);
                return std::nullopt;
            },
            [&](const std::unique_ptr<ast::expression>& body) -> std::optional<std::string> {
                const auto* type = structures::type::inferExpressionType(body.get(), {&program_type_table, current_class_symbol, current_symbol_table.get()});
                if (!structures::type::isSubtype(type, return_type)) {
                    return std::format("type mismatched\n"); // todo implement type to string method
                }
                return std::nullopt;
            }},
        method_body);
    if (err.has_value()) {
        error_message += *err;
    }
}

void class_method_checker::visit(ast::constructor_declaration& node) {
    current_symbol_table = std::make_unique<structures::symbol_table>(current_class_symbol->class_scope.get());
    for (const auto& param : node.parameters) {
        const auto& param_name = param->name;
        const auto* param_type = program_type_table.resolveType(param->type_name);
        current_symbol_table->add(std::make_unique<structures::variable_symbol>(param_name, param_type));
    }
    method_return_type = nullptr;
    node.body->accept(*this);
}

void class_method_checker::visit(ast::block& node) {
    for (auto& entity : node.items) {
        entity->accept(*this);
    }
}

void class_method_checker::visit(ast::variable_declaration& node) {
    const auto * type_res = structures::type::inferExpressionType(node.initializer.get(), {&program_type_table, current_class_symbol, current_symbol_table.get()});
    current_symbol_table->add(std::make_unique<structures::variable_symbol>(node.name, type_res));
}

void class_method_checker::visit(ast::if_statement& node) {
    const auto * condition_type = structures::type::inferExpressionType(node.condition.get(), {&program_type_table, current_class_symbol, current_symbol_table.get()});
    if (!structures::type::typesEqual(condition_type, program_type_table.resolveType("Boolean"))) {
        error_message += "condition expression should have boolean type\n";
        return;
    }
    node.true_branch->accept(*this);
    if (node.false_branch != nullptr) {
        node.false_branch->accept(*this);
    }
}

void class_method_checker::visit(ast::return_statement& node) {
    if (method_return_type == nullptr) {
        // this is return in constructor definition, so just return
        return;
    }

    const auto * return_type = structures::type::inferExpressionType(node.value.get(), {&program_type_table, current_class_symbol, current_symbol_table.get()});
    if (!structures::type::isSubtype(return_type, method_return_type)) {
        error_message += "type mismatched"; // todo add type print
    }
}

void class_method_checker::visit(ast::while_statement& node) {
    const auto * condition_type = structures::type::inferExpressionType(node.condition.get(), {&program_type_table, current_class_symbol, current_symbol_table.get()});
    if (!structures::type::typesEqual(condition_type, program_type_table.resolveType("Boolean"))) {
        error_message += "condition expression should have boolean type\n";
        return;
    }
    node.body->accept(*this);
}

void class_method_checker::visit(ast::assignment_statement& node) {
    const auto * target_symbol = current_symbol_table->typed_lookup<structures::variable_symbol>(node.target);
    if (target_symbol == nullptr) {
        error_message += std::format("{} should be already defined variable\n", node.target);
        return;
    }
    const auto * expr_type = structures::type::inferExpressionType(node.value.get(), {&program_type_table, current_class_symbol, current_symbol_table.get()});
    if (!structures::type::isSubtype(expr_type, target_symbol->type)) {
        error_message += "assignment expression should have subtype of variable symbol\n";
        return;
    }
}

void class_method_checker::visit(ast::parameter_declaration& node) {}
void class_method_checker::visit(ast::literal_expression& node) {}
void class_method_checker::visit(ast::this_expression& node) {}
void class_method_checker::visit(ast::identifier_expression& node) {}
void class_method_checker::visit(ast::parameterized_identifier_expression& node) {}
void class_method_checker::visit(ast::member_expression& node) {}
void class_method_checker::visit(ast::call_expression& node) {}
void class_method_checker::visit(ast::grouping_expression& node) {}

} // namespace analysis::semantic::phases::details
