#include "compiler/analysis/semantic/phases/class-body-collector.h"

#include <format>
#include <memory>
#include <string>
#include <vector>

#include "compiler/compilation-structures/ast.h"

namespace {

std::string make_constructor_signature(const std::vector<std::unique_ptr<ast::parameter_declaration>>& params, const std::string & name) {
    std::string sig = name + "(";
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) {
            sig += ",";
        }
        sig += params[i]->type_name;
    }
    sig += ")";
    return sig;
}
} // namespace

namespace analysis::semantic::phases::details {

void class_body_collector::visit(ast::program& node) {
    for (auto& cls : node.classes) {
        cls->accept(*this);
    }
}

void class_body_collector::visit(ast::class_declaration& node) {
    current_class = program_symbol_table.typed_lookup<structures::class_symbol>(node.name);
    if (current_class == nullptr) {
        error_message += std::format("Internal error: class {} not found in symbol table\n", node.name);
        return;
    }

    field_names.clear();
    method_names.clear();
    constructor_signatures.clear();

    try {
        for (auto& field : node.fields) {
            field->accept(*this);
        }

        for (auto& method : node.methods) {
            method->accept(*this);
        }

        for (auto& ctor : node.constructors) {
            ctor->accept(*this);
        }
    } catch (std::exception & exception) {
        error_message += exception.what();
    }

    current_class = nullptr;
}

void class_body_collector::visit(ast::variable_declaration& node) {
    if (field_names.contains(node.name)) {
        error_message += std::format("Duplicate field '{}' in class '{}'\n", node.name, current_class->name);
        return;
    }
    field_names.insert(node.name);

    auto field = std::make_unique<structures::variable_symbol>(node.name, program_type_table.getUnknown());
    current_class->fields.push_back(field.get());
    current_class->class_scope->add(std::move(field));
}

void class_body_collector::visit(ast::method_declaration& node) {
    if (method_names.contains(node.name)) {
        error_message += std::format("Duplicate method '{}' in class '{}'\n", node.name, current_class->name);
        return;
    }
    method_names.insert(node.name);

    std::optional<const structures::type*> return_type =
        node.return_type.transform([this](const std::string& type_name) { return program_type_table.resolveType(type_name); });
    std::vector<const structures::type *> param_types;
    for (auto& param : node.parameters) {
        param_types.push_back(program_type_table.resolveType(param->type_name));
    }

    auto method = std::make_unique<structures::method_symbol>(node.name, current_class->class_scope.get(), return_type, std::move(param_types));

    for (auto& param : node.parameters) {
        auto param_sym = std::make_unique<structures::variable_symbol>(param->name, program_type_table.resolveType(param->type_name));
        method->method_scope->add(std::move(param_sym));
    }
    current_class->methods.push_back(method.get());
    current_class->class_scope->add(std::move(method));
}

void class_body_collector::visit(ast::constructor_declaration& node) {
    std::string signature = make_constructor_signature(node.parameters, current_class->name);
    if (constructor_signatures.contains(signature)) {
        error_message += std::format("Duplicate constructor with signature '{}' in class '{}'\n", signature, current_class->name);
        return;
    }
    constructor_signatures.insert(signature);

    std::vector<const structures::type *> param_types;
    for (auto& param : node.parameters) {
        param_types.push_back(program_type_table.resolveType(param->type_name));
    }

    auto ctor = std::make_unique<structures::method_symbol>(signature, current_class->class_scope.get(), program_type_table.resolveType(current_class->name), std::move(param_types));
    for (auto& param : node.parameters) {
        auto param_sym = std::make_unique<structures::variable_symbol>(param->name, program_type_table.resolveType(param->type_name));
        ctor->method_scope->add(std::move(param_sym));
    }

    current_class->constructors.push_back(std::move(ctor));
}

void class_body_collector::visit(ast::parameter_declaration& node) {}
void class_body_collector::visit(ast::block& node) {}
void class_body_collector::visit(ast::assignment_statement& node) {}
void class_body_collector::visit(ast::while_statement& node) {}
void class_body_collector::visit(ast::if_statement& node) {}
void class_body_collector::visit(ast::return_statement& node) {}
void class_body_collector::visit(ast::literal_expression& node) {}
void class_body_collector::visit(ast::this_expression& node) {}
void class_body_collector::visit(ast::identifier_expression& node) {}
void class_body_collector::visit(ast::parameterized_identifier_expression& node) {}
void class_body_collector::visit(ast::member_expression& node) {}
void class_body_collector::visit(ast::call_expression& node) {}
void class_body_collector::visit(ast::grouping_expression& node) {}

} // namespace analysis::semantic::phases::details
