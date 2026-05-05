#include "compiler/compilation-structures/type-table.h"

#include <cassert>
#include <format>
#include <stdexcept>

#include "compiler/compilation-structures/ast/parsing/ast.h"
#include "compiler/compilation-structures/symbol-table.h"

namespace {

std::string format_argument_types(const std::vector<const structures::type*>& types) {
    std::string result = "(";
    for (size_t i = 0; i < types.size(); ++i) {
        if (i != types.size() - 1) {
            result += ", ";
        }
        result += types[i]->toString();
    }
    result += ")";
    return result;
}

structures::method_symbol*
find_method_in_hierarchy(structures::class_symbol* class_symbol, const std::string& name, const std::vector<const structures::type*>& argument_types) {
    auto* current = class_symbol;
    structures::method_symbol* best_match = nullptr;

    while (current != nullptr) {
        for (auto* method : current->methods) {
            if (method->original_name != name)
                continue;
            if (method->parameter_types.size() != argument_types.size())
                continue;

            bool matches = true;
            for (size_t i = 0; i < argument_types.size(); ++i) {
                if (!structures::type::isSubtype(argument_types[i], method->parameter_types[i])) {
                    matches = false;
                    break;
                }
            }

            if (matches) {
                if (best_match != nullptr) {
                    throw std::runtime_error{std::format("Ambiguous method call '{}' - multiple overloads match\n", name)};
                }
                best_match = method;
            }
        }
        current = current->base_class;
    }
    return best_match;
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

const structures::type* infer_expression(const ast::expression* expression, structures::type::infer_context context);

void resolve_constructor_call(structures::class_symbol* target_class, const std::vector<const structures::type*>& argument_types) {
    if (target_class->constructors.empty()) {
        throw std::runtime_error{std::format("Class '{}' has no constructors defined\n", target_class->name)};
    }

    structures::method_symbol* matching_constructor = nullptr;
    for (const auto& ctor : target_class->constructors) {
        if (argument_types.size() != ctor->parameter_types.size()) {
            continue;
        }
        bool types_match = true;
        for (size_t i = 0; i < argument_types.size(); ++i) {
            if (!structures::type::isSubtype(argument_types[i], ctor->parameter_types[i])) {
                types_match = false;
                break;
            }
        }
        if (types_match) {
            if (matching_constructor != nullptr) {
                throw std::runtime_error{
                    std::format("Ambiguous constructor call for class '{}': multiple overloads match the argument types\n", target_class->name)};
            }
            matching_constructor = ctor.get();
        }
    }

    if (matching_constructor == nullptr) {
        std::string error_msg = std::format("No matching constructor for '{}'\n", target_class->name);
        error_msg += std::format("  Argument types: {}\n", format_argument_types(argument_types));
        error_msg += "  Available constructors:\n";
        for (const auto& ctor : target_class->constructors) {
            error_msg += std::format("    {}{}\n", target_class->name, format_argument_types(ctor->parameter_types));
        }
        throw std::runtime_error{error_msg};
    }
}

void validate_method_call(structures::method_symbol* method, const std::vector<const structures::type*>& argument_types) {
    if (argument_types.size() != method->parameter_types.size()) {
        throw std::runtime_error{
            std::format("Method '{}': expected {} argument(s), got {}\n", method->original_name, method->parameter_types.size(), argument_types.size())};
    }

    for (size_t i = 0; i < argument_types.size(); ++i) {
        if (!structures::type::isSubtype(argument_types[i], method->parameter_types[i])) {
            throw std::runtime_error{std::format("Method '{}': argument {} type mismatch - expected '{}', found '{}'\n",
                                                 method->original_name,
                                                 i + 1,
                                                 method->parameter_types[i]->toString(),
                                                 argument_types[i]->toString())};
        }
    }
}

const structures::type* infer_call_expression(const ast::call_expression* call_expr, structures::type::infer_context context) {
    std::vector<const structures::type*> argument_types;
    for (const auto& arg : call_expr->arguments) {
        argument_types.push_back(infer_expression(arg.get(), context));
    }

    if (auto* ident_expr = dynamic_cast<ast::identifier_expression*>(call_expr->callee.get())) {
        if (auto* target_class = context.symbol_table->typed_lookup<structures::class_symbol>(ident_expr->name)) {
            resolve_constructor_call(target_class, argument_types);
            return context.type_table->resolveType(target_class->name);
        }
        throw std::runtime_error{std::format("undefined constructor or function: {}\n", ident_expr->name)};
    }

    if (auto* member_expr = dynamic_cast<ast::member_expression*>(call_expr->callee.get())) {
        if (auto* obj_ident = dynamic_cast<ast::identifier_expression*>(member_expr->object.get()); obj_ident && obj_ident->name == member_expr->member) {
            // Constructor call: ClassName(args)
            auto* target_class = context.symbol_table->typed_lookup<structures::class_symbol>(obj_ident->name);
            assert(target_class != nullptr);
            resolve_constructor_call(target_class, argument_types);
            return context.type_table->resolveType(target_class->name);
        }

        auto object_type = infer_expression(member_expr->object.get(), context);

        const auto* class_type = dynamic_cast<const structures::class_type*>(object_type);
        if (class_type == nullptr) {
            throw std::runtime_error{"cannot call method on non-class type\n"};
        }

        auto* object_class = context.symbol_table->typed_lookup<structures::class_symbol>(class_type->name);
        assert(object_class != nullptr);

        if (object_class->name == member_expr->member) {
            resolve_constructor_call(object_class, argument_types);
            return class_type;
        }

        auto* method = find_method_in_hierarchy(object_class, member_expr->member, argument_types);
        if (method == nullptr) {
            throw std::runtime_error{std::format("method '{}' with arguments '{}' not found in class '{}'\n",
                                                 member_expr->member,
                                                 format_argument_types(argument_types),
                                                 class_type->name)};
        }

        if (method->return_type.has_value()) {
            return method->return_type.value();
        }
        return context.type_table->resolveType("Unit");
    }

    throw std::runtime_error{"unsupported call expression type\n"};
}

const structures::type* infer_expression(const ast::expression* expression, structures::type::infer_context context) {
    if (auto* literal = dynamic_cast<const ast::literal_expression*>(expression)) {
        switch (literal->type) {
        case ast::literal_expression::type::integer:
            return context.type_table->resolveType("Integer");
        case ast::literal_expression::type::real:
            return context.type_table->resolveType("Real");
        case ast::literal_expression::type::boolean:
            return context.type_table->resolveType("Boolean");
        default:
            throw std::runtime_error{"unknown literal type\n"};
        }
    }

    if (dynamic_cast<const ast::this_expression*>(expression)) {
        return context.type_table->resolveType(context.class_symbol->name);
    }

    if (auto* ident = dynamic_cast<const ast::identifier_expression*>(expression)) {
        if (auto* var = context.symbol_table->typed_lookup<structures::variable_symbol>(ident->name)) {
            return var->type;
        }
        if (auto* cls = context.symbol_table->typed_lookup<structures::class_symbol>(ident->name)) {
            throw std::runtime_error{std::format("{} is class name and can't be used in expression\n", ident->name)};
            ;
        }
        throw std::runtime_error{std::format("undefined variable symbol {}\n", ident->name)};
    }

    if (auto* member = dynamic_cast<const ast::member_expression*>(expression)) {
        auto object_type = infer_expression(member->object.get(), context);
        if (const auto* cls_type = dynamic_cast<const structures::class_type*>(object_type)) {
            auto* cls = context.symbol_table->typed_lookup<structures::class_symbol>(cls_type->name);
            assert(cls != nullptr);
            auto* field = find_field_in_hierarchy(cls, member->member);
            if (field == nullptr) {
                throw std::runtime_error{std::format("cannot find field {} in class {}\n", member->member, cls_type->name)};
            }
            return field->type;
        }
        throw std::runtime_error{"accessing to fields on internal types unsupported\n"};
    }

    if (auto* call = dynamic_cast<const ast::call_expression*>(expression)) {
        return infer_call_expression(call, context);
    }

    if (auto* group = dynamic_cast<const ast::grouping_expression*>(expression)) {
        return infer_expression(group->inner.get(), context);
    }

    throw std::runtime_error{"unsupported expression kind for type infer"};
}

} // namespace

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

const type* type::inferExpressionType(const ast::expression* expression, type::infer_context context) {
    return infer_expression(expression, context);
}

type_table::type_table() {
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
    auto it = class_types_.find(name);
    if (it != class_types_.end()) {
        return it->second;
    }

    throw std::runtime_error{std::format("unknown type {}\n", name)};
}

bool type_table::isPrimitiveTypeName(const std::string& name) {
    return name == "Integer" || name == "Boolean" || name == "Real" || name == "Unit";
}

} // namespace structures
