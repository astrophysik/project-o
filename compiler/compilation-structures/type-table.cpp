#include "compiler/compilation-structures/type-table.h"

#include <format>
#include <cassert>

#include "compiler/compilation-structures/ast.h"
#include "compiler/compilation-structures/symbol-table.h"

namespace {
structures::method_symbol* find_method_in_hierarchy(structures::class_symbol* class_symbol, const std::string& name) {
    auto* current = class_symbol;
    while (current != nullptr) {
        if (auto* method = current->class_scope->typed_lookup<structures::method_symbol>(name)) {
            return method;
        }
        current = current->base_class;
    }
    return nullptr;
}

structures::variable_symbol* find_field_in_hierarchy(structures::class_symbol* class_symbol, const std::string& name) {
    auto* current = class_symbol;
    while (current != nullptr) {
        if (auto* field = current->class_scope->typed_lookup<structures::variable_symbol>(name)) {
            return field;
        }
        current = current->base_class;
    }
    return nullptr;
}
}

namespace structures {

bool type::isSubtype(const type* sub, const type* super) {
    if (sub == super) {
        return true;
    }
    if (sub->isError() || super->isError()) {
        return true;
    }

    if (auto* sub_cls = dynamic_cast<const class_type*>(sub)) {
        if (auto* super_cls = dynamic_cast<const class_type*>(super)) {
            ast::class_declaration* current = sub_cls->declaration;
            while (current && current->base_class) {
                if (*current->base_class == super_cls->name) {
                    return true;
                }
                break;
            }
        }
    }
    return false;
}

bool type::typesEqual(const type* t1, const type* t2) {
    return t1 == t2;
}

std::expected<const type*, std::string> type::inferExpressionType(const ast::expression* expression, type::infer_context context) {
    if (auto* literal = dynamic_cast<const ast::literal_expression*>(expression)) {
        switch (literal->type) {
        case ast::literal_expression::type::integer:
            return context.type_table->resolveType("Integer");
        case ast::literal_expression::type::real:
            return context.type_table->resolveType("Real");
        case ast::literal_expression::type::boolean:
            return context.type_table->resolveType("Bool");
        default:
            return std::unexpected{"unknown literal type\n"};
        }
    } else if (const auto* this_expr = dynamic_cast<const ast::this_expression*>(expression)) {
        return context.type_table->resolveType(context.class_symbol->name); // todo check args
    } else if (const auto* ident_expr = dynamic_cast<const ast::identifier_expression*>(expression)) {
        if (auto* variable_symbol = context.class_symbol->class_scope->typed_lookup<structures::variable_symbol>(ident_expr->name)) {
            return variable_symbol->type;
        } else if (auto* class_symbol = context.class_symbol->class_scope->typed_lookup<structures::class_symbol>(ident_expr->name)) {
            return context.type_table->resolveType(class_symbol->name);
        } else {
            return std::unexpected{std::format("cannot use non variable symbol for field initialization {}\n", ident_expr->name)};
        }
    } else if (auto* member_expr = dynamic_cast<const ast::member_expression*>(expression)) {
        auto object_type = inferExpressionType(member_expr->object.get(), context);
        if (!object_type.has_value()) {
            return object_type;
        }
        if (const auto* class_type = dynamic_cast<const structures::class_type*>(*object_type)) {
            auto* class_symbol = context.class_symbol->class_scope->typed_lookup<structures::class_symbol>(class_type->name);
            assert(class_symbol != nullptr);
            auto* field_symbol = find_field_in_hierarchy(class_symbol, member_expr->member);
            if (field_symbol == nullptr) {
                return std::unexpected{std::format("cannot find field {} in class {}\n", member_expr->member, class_type->name)};
            }
            return field_symbol->type;
        } else {
            std::unexpected{"accessing to fields on internal types unsupported\n"};
            return nullptr;
        }
    } else if (auto* call_expr = dynamic_cast<const ast::call_expression*>(expression)) {
        if (auto* ident_expr = dynamic_cast<ast::identifier_expression*>(call_expr->callee.get())) {
            if (auto* class_symbol = context.class_symbol->class_scope->typed_lookup<structures::class_symbol>(ident_expr->name)) {
                return context.type_table->resolveType(class_symbol->name);
            }
            return std::unexpected{std::format("undefined constructor or function: {}\n", ident_expr->name)};
        } else if (auto* member_expr = dynamic_cast<ast::member_expression*>(call_expr->callee.get())) {
            auto object_type = inferExpressionType(member_expr->object.get(), context);
            if (!object_type.has_value()) {
                return object_type;
            }

            const auto* class_type = dynamic_cast<const structures::class_type*>(*object_type);
            if (class_type == nullptr) {
                return std::unexpected{"cannot call method on non-class type\n"};
            }

            auto* class_symbol = context.class_symbol->class_scope->typed_lookup<structures::class_symbol>(class_type->name);
            assert(class_symbol != nullptr);

            if (class_symbol->name == member_expr->member) {
                return class_type;
            }


            auto* method_symbol = find_method_in_hierarchy(class_symbol, member_expr->member);
            if (method_symbol == nullptr) {
                return std::unexpected{std::format("method {} not found in class {}\n", member_expr->member, class_type->name)};
            }

            if (method_symbol->return_type.has_value()) {
                return method_symbol->return_type.value();
            } else {
                return context.type_table->resolveType("Unit");
            }
        } else {
            return std::unexpected{"unsupported call expression type\n"};
        }
    } else if (auto* group_expr = dynamic_cast<const ast::grouping_expression*>(expression)) {
        return inferExpressionType(group_expr->inner.get(), context);
    }
    return std::unexpected{"unsupported expression kind for type infer"};
}

type_table::type_table() {
    owned_types_.push_back(std::make_unique<primitive_type>(type_kind::Unit));
    unit_type_ = owned_types_.back().get();

    owned_types_.push_back(std::make_unique<primitive_type>(type_kind::Int));
    int_type_ = owned_types_.back().get();

    owned_types_.push_back(std::make_unique<primitive_type>(type_kind::Bool));
    bool_type_ = owned_types_.back().get();

    owned_types_.push_back(std::make_unique<primitive_type>(type_kind::Real));
    real_type_ = owned_types_.back().get();

    owned_types_.push_back(std::make_unique<primitive_type>(type_kind::Error));
    error_type_ = owned_types_.back().get();

    owned_types_.push_back(std::make_unique<primitive_type>(type_kind::Unknown));
    unknown_type_ = owned_types_.back().get();
}

const class_type* type_table::getClass(const std::string& name) const {
    auto it = class_types_.find(name);
    if (it != class_types_.end()) {
        return it->second;
    }
    return nullptr;
}

const class_type* type_table::addClass(const std::string& name, ast::class_declaration* decl) {
    if (class_types_.find(name) != class_types_.end()) {
        return nullptr;
    }

    auto class_type = std::make_unique<structures::class_type>(name, decl);
    const structures::class_type* ptr = class_type.get();
    owned_types_.push_back(std::move(class_type));
    class_types_[name] = ptr;
    return ptr;
}

const type* type_table::resolveType(const std::string& name) const {
    if (name == "Integer") {
        return int_type_;
    }
    if (name == "Bool") {
        return bool_type_;
    }
    if (name == "Unit") {
        return unit_type_;
    }
    if (name == "Real") {
        return real_type_;
    }

    auto it = class_types_.find(name);
    if (it != class_types_.end()) {
        return it->second;
    }

    return unknown_type_;
}

bool type_table::isPrimitiveTypeName(const std::string& name) {
    return name == "Integer" || name == "Bool" || name == "Void" || name == "Real";
}

} // namespace structures
