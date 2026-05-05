#include "compiler/analysis/semantic/phases/codegen-ast-collector.h"

#include <algorithm>
#include <cassert>
#include <format>
#include <variant>

#include "compiler/compilation-structures/ast/codegen/ast.h"
#include "compiler/compilation-structures/ast/parsing/ast.h"

namespace analysis::semantic::phases::details {

codegen::ast::class_declaration* codegen_ast_collector::resolveType(const std::string& type_name) {
    for (auto& cls : result_program->internal_classes) {
        if (cls->name == type_name) {
            return cls.get();
        }
    }
    for (auto& [parsing_cls, codegen_cls] : class_map) {
        if (parsing_cls->name == type_name) {
            return codegen_cls;
        }
    }
    return nullptr;
}

codegen::ast::class_declaration* codegen_ast_collector::resolveType(const structures::type* type) {
    if (!type || type->kind == structures::type_kind::Unknown) {
        return nullptr;
    }

    if (type->kind == structures::type_kind::Class) {
        auto* class_type = dynamic_cast<const structures::class_type*>(type);
        return resolveType(class_type->name);
    }

    std::string type_name;
    switch (type->kind) {
    case structures::type_kind::Int:
        type_name = "Integer";
        break;
    case structures::type_kind::Bool:
        type_name = "Boolean";
        break;
    case structures::type_kind::Real:
        type_name = "Real";
        break;
    case structures::type_kind::Unit:
        type_name = "Unit";
        break;
    default:
        return nullptr;
    }
    return resolveType(type_name);
}

void codegen_ast_collector::visit(ast::program& node) {
    // Pass 1: Create all class declarations first (for forward references)
    for (auto& cls : node.classes) {
        auto codegen_cls = std::make_unique<codegen::ast::class_declaration>();
        codegen_cls->name = cls->name;
        class_map[cls.get()] = codegen_cls.get();
        result_program->classes.push_back(std::move(codegen_cls));
    }

    // Pass 2: collect declarations
    for (auto& cls : node.classes) {
        current_class = class_map[cls.get()];
        auto* class_sym = program_symbol_table.typed_lookup<structures::class_symbol>(cls->name);
        assert(class_sym != nullptr);
        current_scope = class_sym ? class_sym->class_scope.get() : nullptr;

        if (cls->base_class.has_value()) {
            current_class->base_class = resolveType(*cls->base_class);
        }

        for (auto& method : cls->methods) {
            auto method_decl = std::make_unique<codegen::ast::method_declaration>();
            method_decl->name = method->name;
            current_class->methods.push_back(std::move(method_decl));
        }

        for (auto& field : cls->fields) {
            field->accept(*this);
        }

        for (auto& ctor : cls->constructors) {
            ctor->accept(*this);
        }

        for (auto& method : cls->methods) {
            method->accept(*this);
        }
    }

    current_class = nullptr;
    current_scope = nullptr;
}

void codegen_ast_collector::visit(ast::class_declaration& node) {}

void codegen_ast_collector::visit(ast::variable_declaration& node) {
    if (current_class && !inside_function_scope) {
        auto field = std::make_unique<codegen::ast::field_declaration>();
        field->name = node.name;
        field->class_owner = current_class;

        if (node.initializer) {
            field->initializer = transformExpression(node.initializer.get());
        }

        // Infer type from initializer or symbol table
        auto* class_sym = program_symbol_table.typed_lookup<structures::class_symbol>(current_class->name);
        if (node.initializer) {
            auto* expr_type = structures::type::inferExpressionType(node.initializer.get(), {&program_type_table, class_sym, current_scope});
            field->type = resolveType(expr_type);
        }
        variable_map[node.name] = field.get();
        current_class->fields.push_back(std::move(field));
    } else {
        auto var = std::make_unique<codegen::ast::variable_declaration>();
        var->name = node.name;

        if (node.initializer) {
            var->initializer = transformExpression(node.initializer.get());
        }

        // Infer type
        auto* class_sym = current_class ? program_symbol_table.typed_lookup<structures::class_symbol>(current_class->name) : nullptr;
        if (node.initializer) {
            auto* expr_type = structures::type::inferExpressionType(node.initializer.get(), {&program_type_table, class_sym, current_scope});
            var->type = resolveType(expr_type);
        }

        variable_map[node.name] = var.get();
        last_statement = std::move(var);
    }
}

void codegen_ast_collector::visit(ast::parameter_declaration& node) {
    auto param = std::make_unique<codegen::ast::parameter_declaration>();
    param->name = node.name;
    param->type = resolveType(node.type_name);

    variable_map[node.name] = param.get();

    if (current_method) {
        current_method->parameters.push_back(std::move(param));
    }
}

void codegen_ast_collector::visit(ast::method_declaration& node) {
    auto it = std::find_if(current_class->methods.begin(), current_class->methods.end(), [&node](const auto& method) { return method->name == node.name; });
    assert(it != current_class->methods.end());
    codegen::ast::method_declaration* method = it->get();

    std::vector<const structures::type *> params;
    for (auto& param : node.parameters) {
        params.push_back(program_type_table.resolveType(param->type_name));
    }
    std::string mangled = structures::mangle_method_name(node.name, params);
    current_scope = current_scope->typed_lookup<structures::method_symbol>(mangled)->method_scope.get();
    assert(current_scope != nullptr);

    method->name = node.name;
    method->class_owner = current_class;
    current_method = method;

    if (node.return_type.has_value()) {
        method->return_type = resolveType(*node.return_type);
    }

    for (auto& param : node.parameters) {
        param->accept(*this);
    }

    inside_function_scope = true;
    if (node.body.has_value()) {
        std::visit(
            [&](auto& body) {
                using T = std::decay_t<decltype(body)>;
                if constexpr (std::is_same_v<T, std::unique_ptr<ast::block>>) {
                    method->body = transformBlock(body.get());
                } else if constexpr (std::is_same_v<T, std::unique_ptr<ast::expression>>) {
                    method->body = transformExpression(body.get());
                }
            },
            *node.body);
    }
    inside_function_scope = false;

    current_method = nullptr;
}

void codegen_ast_collector::visit(ast::constructor_declaration& node) {
    std::vector<const structures::type *> params;
    for (auto& param : node.parameters) {
        params.push_back(program_type_table.resolveType(param->type_name));
    }
    std::string mangled = structures::mangle_method_name(current_class->name, params);
    current_scope = current_scope->typed_lookup<structures::method_symbol>(mangled)->method_scope.get();
    assert(current_scope != nullptr);

    auto ctor = std::make_unique<codegen::ast::constructor_declaration>();
    ctor->class_owner = current_class;

    for (auto& param : node.parameters) {
        auto codegen_param = std::make_unique<codegen::ast::parameter_declaration>();
        codegen_param->name = param->name;
        codegen_param->type = resolveType(param->type_name);
        variable_map[param->name] = codegen_param.get();

        ctor->parameters.push_back(std::move(codegen_param));
    }

    inside_function_scope = true;
    if (node.body) {
        ctor->body = transformBlock(node.body.get());
    }
    inside_function_scope = false;

    current_class->constructors.push_back(std::move(ctor));
}

void codegen_ast_collector::visit(ast::assignment_statement& node) {
    auto* var_sym = current_scope ? current_scope->typed_lookup<structures::variable_symbol>(node.target) : nullptr;

    bool is_field = false;
    if (var_sym && current_class) {
        auto* class_sym = program_symbol_table.typed_lookup<structures::class_symbol>(current_class->name);
        if (class_sym && class_sym->class_scope->lookup(node.target)) {
            is_field = true;
        }
    }

    if (is_field) {
        auto field_assign = std::make_unique<codegen::ast::field_assignment>();
        auto this_expr = std::make_unique<codegen::ast::this_expression>(current_class);
        auto member = std::make_unique<codegen::ast::member_expression>();

        auto* field_owner_class = current_class;
        while (field_owner_class != nullptr) {
            for (auto& f : field_owner_class->fields) {
                if (f->name == node.target) {
                    member->member = f.get();
                    break;
                }
            }
            field_owner_class = field_owner_class->base_class;
        }
        assert(member->member != nullptr);

        member->object = std::move(this_expr);
        field_assign->target = std::move(member);

        if (node.value) {
            field_assign->value = transformExpression(node.value.get());
            auto* class_sym = program_symbol_table.typed_lookup<structures::class_symbol>(current_class->name);
            auto* expr_type = structures::type::inferExpressionType(node.value.get(), {&program_type_table, class_sym, current_scope});
            field_assign->expression_type = resolveType(expr_type);
        }

        last_statement = std::move(field_assign);
    } else {
        auto var_assign = std::make_unique<codegen::ast::variable_assignment>();
        var_assign->target = variable_map[node.target];

        if (node.value) {
            var_assign->value = transformExpression(node.value.get());
            auto* class_sym = current_class ? program_symbol_table.typed_lookup<structures::class_symbol>(current_class->name) : nullptr;
            auto* expr_type = structures::type::inferExpressionType(node.value.get(), {&program_type_table, class_sym, current_scope});
            var_assign->expression_type = resolveType(expr_type);
        }

        last_statement = std::move(var_assign);
    }
}

void codegen_ast_collector::visit(ast::while_statement& node) {
    auto while_stmt = std::make_unique<codegen::ast::while_statement>();

    if (node.condition) {
        while_stmt->condition = transformExpression(node.condition.get());
    }
    if (node.body) {
        while_stmt->body = transformBlock(node.body.get());
    }

    last_statement = std::move(while_stmt);
}

void codegen_ast_collector::visit(ast::if_statement& node) {
    auto if_stmt = std::make_unique<codegen::ast::if_statement>();

    if (node.condition) {
        if_stmt->condition = transformExpression(node.condition.get());
    }
    if (node.true_branch) {
        if_stmt->true_branch = transformBlock(node.true_branch.get());
    }
    if (node.false_branch) {
        if_stmt->false_branch = transformBlock(node.false_branch.get());
    }

    last_statement = std::move(if_stmt);
}

void codegen_ast_collector::visit(ast::return_statement& node) {
    auto ret_stmt = std::make_unique<codegen::ast::return_statement>();

    if (node.value) {
        ret_stmt->value = transformExpression(node.value.get());
        auto* class_sym = current_class ? program_symbol_table.typed_lookup<structures::class_symbol>(current_class->name) : nullptr;
        auto* expr_type = structures::type::inferExpressionType(node.value.get(), {&program_type_table, class_sym, current_scope});
        ret_stmt->expression_type = resolveType(expr_type);
    }

    last_statement = std::move(ret_stmt);
}

void codegen_ast_collector::visit(ast::literal_expression& node) {
    codegen::ast::class_declaration* type = nullptr;

    switch (node.type) {
    case ast::literal_expression::type::integer:
        type = resolveType("Integer");
        last_expression = std::make_unique<codegen::ast::literal_expression>(std::get<int64_t>(node.value), type);
        break;
    case ast::literal_expression::type::real:
        type = resolveType("Real");
        last_expression = std::make_unique<codegen::ast::literal_expression>(std::get<double>(node.value), type);
        break;
    case ast::literal_expression::type::boolean:
        type = resolveType("Boolean");
        last_expression = std::make_unique<codegen::ast::literal_expression>(std::get<bool>(node.value), type);
        break;
    }
}

void codegen_ast_collector::visit(ast::this_expression& node) {
    last_expression = std::make_unique<codegen::ast::this_expression>(current_class);
}

void codegen_ast_collector::visit(ast::identifier_expression& node) {
    last_expression = std::make_unique<codegen::ast::identifier_expression>(variable_map[node.name]);
}

void codegen_ast_collector::visit(ast::parameterized_identifier_expression& node) {
    auto* target_class = resolveType(node.name);
    if (target_class) {
        auto ctor_call = std::make_unique<codegen::ast::constructor_call_expression>();

        for (auto& ctor : target_class->constructors) {
            if (ctor->parameters.empty()) {
                ctor_call->constructor = ctor.get();
                break;
            }
        }

        last_expression = std::move(ctor_call);
    }
}

void codegen_ast_collector::visit(ast::member_expression& node) {
    auto member_expr = std::make_unique<codegen::ast::member_expression>();

    if (node.object) {
        member_expr->object = transformExpression(node.object.get());
    }

    if (node.object) {
        auto* class_sym = current_class ? program_symbol_table.typed_lookup<structures::class_symbol>(current_class->name) : nullptr;
        auto* obj_type = structures::type::inferExpressionType(node.object.get(), {&program_type_table, class_sym, current_scope});

        if (obj_type && obj_type->kind == structures::type_kind::Class) {
            auto* class_type = dynamic_cast<const structures::class_type*>(obj_type);
            auto* target_class = resolveType(class_type->name);
            while (target_class != nullptr) {
                for (auto& field : target_class->fields) {
                    if (field->name == node.member) {
                        member_expr->member = field.get();
                        break;
                    }
                }
                target_class = target_class->base_class;
            }
        }
    }

    last_expression = std::move(member_expr);
}

void codegen_ast_collector::visit(ast::call_expression& node) {
    auto* member_expr = dynamic_cast<ast::member_expression*>(node.callee.get());
    assert(member_expr != nullptr);

    std::vector<const structures::type*> arg_types;
    auto* class_sym = current_class ? program_symbol_table.typed_lookup<structures::class_symbol>(current_class->name) : nullptr;
    for (auto& arg : node.arguments) {
        arg_types.push_back(structures::type::inferExpressionType(arg.get(), {&program_type_table, class_sym, current_scope}));
    }

    // Constructor call: ClassName(args)
    auto* obj_ident = dynamic_cast<ast::identifier_expression*>(member_expr->object.get());
    if (obj_ident && obj_ident->name == member_expr->member) {
        auto* target_class = resolveType(obj_ident->name);
        assert(target_class != nullptr);
        auto ctor_call = std::make_unique<codegen::ast::constructor_call_expression>();

        auto* target_class_sym = program_symbol_table.typed_lookup<structures::class_symbol>(obj_ident->name);
        if (target_class_sym) {
            for (size_t i = 0; i < target_class_sym->constructors.size(); ++i) {
                auto& ctor = target_class_sym->constructors[i];
                if (ctor->parameter_types.size() != arg_types.size())
                    continue;

                bool matches = true;
                for (size_t j = 0; j < arg_types.size(); ++j) {
                    if (!structures::type::isSubtype(arg_types[j], ctor->parameter_types[j])) {
                        matches = false;
                        break;
                    }
                }

                if (matches) {
                    ctor_call->constructor = target_class->constructors[i].get();
                    break;
                }
            }
        }

        for (auto& arg : node.arguments) {
            ctor_call->arguments.push_back(transformExpression(arg.get()));
        }

        last_expression = std::move(ctor_call);
        return;
    }

    // Method call: object.method(args)
    auto method_call = std::make_unique<codegen::ast::method_call_expression>();

    if (member_expr->object) {
        method_call->object = transformExpression(member_expr->object.get());
    }

    if (member_expr->object) {
        auto* obj_type = structures::type::inferExpressionType(member_expr->object.get(), {&program_type_table, class_sym, current_scope});

        if (obj_type && obj_type->kind == structures::type_kind::Class) {
            auto* class_type = dynamic_cast<const structures::class_type*>(obj_type);
            auto* target_class = resolveType(class_type->name);

            if (target_class) {
                auto* target_class_sym = program_symbol_table.typed_lookup<structures::class_symbol>(class_type->name);
                if (target_class_sym) {
                    structures::method_symbol* best_match = nullptr;
                    size_t best_match_index = 0;
                    auto* current = target_class_sym;

                    while (current != nullptr) {
                        for (size_t i = 0; i < current->methods.size(); ++i) {
                            auto* method = current->methods[i];
                            if (method->original_name != member_expr->member)
                                continue;
                            if (method->parameter_types.size() != arg_types.size())
                                continue;

                            bool matches = true;
                            for (size_t j = 0; j < arg_types.size(); ++j) {
                                if (!structures::type::isSubtype(arg_types[j], method->parameter_types[j])) {
                                    matches = false;
                                    break;
                                }
                            }

                            if (matches) {
                                if (best_match != nullptr) {
                                    // Ambiguous - keep first match
                                } else {
                                    best_match = method;
                                    best_match_index = i;
                                }
                            }
                        }
                        current = current->base_class;
                    }

                    if (best_match) {
                        method_call->method = target_class->methods[best_match_index].get();
                        if (best_match->return_type.has_value()) {
                            method_call->return_type = resolveType(*best_match->return_type);
                        }
                    }
                }
            }
        }
    }

    for (auto& arg : node.arguments) {
        method_call->arguments.push_back(transformExpression(arg.get()));
    }

    last_expression = std::move(method_call);
}

void codegen_ast_collector::visit(ast::grouping_expression& node) {
    if (node.inner) {
        last_expression = transformExpression(node.inner.get());
    }
}

void codegen_ast_collector::visit(ast::block& node) {
    last_block = transformBlock(&node);
}

std::unique_ptr<codegen::ast::expression> codegen_ast_collector::transformExpression(ast::expression* expr) {
    if (!expr)
        return nullptr;

    expr->accept(*this);
    return std::move(last_expression);
}

std::unique_ptr<codegen::ast::block> codegen_ast_collector::transformBlock(ast::block* blk) {
    if (!blk)
        return nullptr;

    auto result = std::make_unique<codegen::ast::block>();

    for (auto& item : blk->items) {
        item->accept(*this);
        if (last_statement) {
            result->items.push_back(std::move(last_statement));
        }
    }

    return result;
}

} // namespace analysis::semantic::phases::details
