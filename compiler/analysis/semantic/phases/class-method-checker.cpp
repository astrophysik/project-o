#include "compiler/analysis/semantic/phases/class-method-checker.h"


#include <cassert>
#include <expected>
#include <format>
#include <variant>

#include "compiler/common/variant-helper.h"

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
    for (const auto& constructor : node.constructors) {
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

    std::vector<const structures::type *> param_types;
    for (auto& param : node.parameters) {
        param_types.push_back(program_type_table.resolveType(param->type_name));
    }

    std::string mangled = structures::mangle_method_name(node.name, param_types);
    auto *method_symbol = current_class_symbol->class_scope->typed_lookup<structures::method_symbol>(mangled);
    assert(method_symbol != nullptr);

    current_symbol_table = method_symbol->method_scope.get();
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
                if (!std::exchange(definitely_returns, false)) {
                    return std::format("method {} does not return on all code paths\n", node.name);
                }
                return std::nullopt;
            },
            [&](const std::unique_ptr<ast::expression>& body) -> std::optional<std::string> {
                const auto* type = structures::type::inferExpressionType(body.get(), {&program_type_table, current_class_symbol, current_symbol_table});
                if (!structures::type::isSubtype(type, return_type)) {
                    return std::format("expected return type {} but infer type {} from expression\n", return_type->toString(), type->toString());
                }
                return std::nullopt;
            }},
        method_body);
    if (err.has_value()) {
        error_message += *err;
    }
}

void class_method_checker::visit(ast::constructor_declaration& node) {
    std::vector<const structures::type *> param_types;
    for (auto& param : node.parameters) {
        param_types.push_back(program_type_table.resolveType(param->type_name));
    }
    std::string mangled = structures::mangle_method_name(current_class_symbol->name, param_types);
    auto *method_symbol = current_class_symbol->class_scope->typed_lookup<structures::method_symbol>(mangled);
    assert(method_symbol != nullptr);

    current_symbol_table = method_symbol->method_scope.get();
    for (const auto& param : node.parameters) {
        const auto& param_name = param->name;
        const auto* param_type = program_type_table.resolveType(param->type_name);
        current_symbol_table->add(std::make_unique<structures::variable_symbol>(param_name, param_type));
    }
    method_return_type = nullptr;
    node.body->accept(*this);
    std::exchange(definitely_returns, false);
}

void class_method_checker::visit(ast::block& node) {
    for (auto& entity : node.items) {
        entity->accept(*this);
    }
}

void class_method_checker::visit(ast::variable_declaration& node) {
    const auto* type_res =
        structures::type::inferExpressionType(node.initializer.get(), {&program_type_table, current_class_symbol, current_symbol_table});
    if (current_symbol_table->typed_lookup<structures::variable_symbol>(node.name) != nullptr) {
        throw std::runtime_error{std::format("variable {} is already defined", node.name)};
    }
    current_symbol_table->add(std::make_unique<structures::variable_symbol>(node.name, type_res));
}

void class_method_checker::visit(ast::if_statement& node) {
    const auto* condition_type =
        structures::type::inferExpressionType(node.condition.get(), {&program_type_table, current_class_symbol, current_symbol_table});
    if (!structures::type::typesEqual(condition_type, program_type_table.resolveType("Boolean"))) {
        error_message += std::format("expected boolean type for condition expression but infer {}\n", condition_type->toString());
        return;
    }
    node.true_branch->accept(*this);
    bool then_returns = std::exchange(definitely_returns, false);

    bool else_returns = false;
    if (node.false_branch != nullptr) {
        node.false_branch->accept(*this);
        else_returns = std::exchange(definitely_returns, false);
    }

    definitely_returns = then_returns && else_returns;
}

void class_method_checker::visit(ast::return_statement& node) {
    definitely_returns = true;
    if (method_return_type == nullptr) {
        if (node.value != nullptr) {
            error_message += std::format("constructors cannot return value\n");
        }
        // this is return in constructor definition, so just return
        return;
    }

    if (node.value == nullptr) {
        error_message += std::format("method must return a value of type {}\n", method_return_type->toString());
        return;
    }

    const auto* return_type = structures::type::inferExpressionType(node.value.get(), {&program_type_table, current_class_symbol, current_symbol_table});
    if (!structures::type::isSubtype(return_type, method_return_type)) {
        error_message += std::format("expected return type {} but infer type {} from expression\n", method_return_type->toString(), return_type->toString());
    }
}

void class_method_checker::visit(ast::while_statement& node) {
    const auto* condition_type =
        structures::type::inferExpressionType(node.condition.get(), {&program_type_table, current_class_symbol, current_symbol_table});
    if (!structures::type::typesEqual(condition_type, program_type_table.resolveType("Boolean"))) {
        error_message += std::format("expected boolean type for condition expression but infer {}\n", condition_type->toString());
        return;
    }
    node.body->accept(*this);
}

void class_method_checker::visit(ast::assignment_statement& node) {
    const auto* target_symbol = current_symbol_table->typed_lookup<structures::variable_symbol>(node.target);
    if (target_symbol == nullptr) {
        error_message += std::format("unknown field or variable with name {}\n", node.target);
        return;
    }
    const auto* expr_type = structures::type::inferExpressionType(node.value.get(), {&program_type_table, current_class_symbol, current_symbol_table});
    if (!structures::type::isSubtype(expr_type, target_symbol->type)) {
        error_message += std::format("expected type {} for assignment expression of variable {} but infer {}\n",
                                     target_symbol->type->toString(),
                                     target_symbol->name,
                                     expr_type->toString());
        return;
    }
}

void class_method_checker::visit(ast::call_expression& node) {
    error_message += "call expression with discard result are forbidden\n";
}

void class_method_checker::visit(ast::parameter_declaration& node) {}
void class_method_checker::visit(ast::literal_expression& node) {}
void class_method_checker::visit(ast::this_expression& node) {}
void class_method_checker::visit(ast::identifier_expression& node) {}
void class_method_checker::visit(ast::parameterized_identifier_expression& node) {}
void class_method_checker::visit(ast::member_expression& node) {}
void class_method_checker::visit(ast::grouping_expression& node) {}

} // namespace analysis::semantic::phases::details
