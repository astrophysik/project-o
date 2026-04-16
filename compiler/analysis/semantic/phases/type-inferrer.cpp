#include "type-inferrer.h"
#include <format>

namespace analysis::semantic {

std::expected<type, std::string>
type_inferrer::infer(ast::expression* expr, symbol_table& scope) {

    // literal
    if (auto* lit = dynamic_cast<ast::literal_expression*>(expr)) {
        switch (lit->type) {
            case ast::literal_expression::type::integer: return "Integer";
            case ast::literal_expression::type::real:    return "Real";
            case ast::literal_expression::type::boolean: return "Boolean";
        }
    }

    // this
    if (dynamic_cast<ast::this_expression*>(expr)) {
        return current_class_name_;
    }

    // identifier
    if (auto* id = dynamic_cast<ast::identifier_expression*>(expr)) {
        auto* sym = scope.lookup(id->name);
        if (!sym)
            return std::unexpected{std::format("Undefined identifier '{}'", id->name)};
        if (auto* var = dynamic_cast<variable_symbol*>(sym))
            return var->type;
        if (auto* cls = dynamic_cast<class_symbol*>(sym))
            return cls->name;
        return std::unexpected{std::format("'{}' is not a variable or class", id->name)};
    }

    // grouping
    if (auto* grp = dynamic_cast<ast::grouping_expression*>(expr)) {
        return infer(grp->inner.get(), scope);
    }

    // member access: obj.field
    if (auto* mem = dynamic_cast<ast::member_expression*>(expr)) {
        auto obj_type = infer(mem->object.get(), scope);
        if (!obj_type) return obj_type;

        auto* cls = program_table_.typed_lookup<class_symbol>(*obj_type);
        if (!cls)
            return std::unexpected{std::format(
                "Type '{}' is not a class", *obj_type)};

        auto* field = cls->class_scope->typed_lookup<variable_symbol>(mem->member);
        if (!field)
            return std::unexpected{std::format(
                "Class '{}' has no field '{}'", *obj_type, mem->member)};
        return field->type;
    }

    // call: obj.method(args)
    if (auto* call = dynamic_cast<ast::call_expression*>(expr)) {
        auto* mem = dynamic_cast<ast::member_expression*>(call->callee.get());
        if (!mem)
            return std::unexpected{"Unsupported call expression form"};

        auto obj_type = infer(mem->object.get(), scope);
        if (!obj_type) return obj_type;

        auto* cls = program_table_.typed_lookup<class_symbol>(*obj_type);
        if (!cls)
            return std::unexpected{std::format(
                "Type '{}' is not a class", *obj_type)};

        auto* method = cls->class_scope->typed_lookup<method_symbol>(mem->member);
        if (!method)
            return std::unexpected{std::format(
                "Class '{}' has no method '{}'", *obj_type, mem->member)};
        if (!method->return_type)
            return std::unexpected{std::format(
                "Method '{}' has no return type", mem->member)};
        return *method->return_type;
    }

    return std::unexpected{"Unsupported expression kind"};
}

} // namespace analysis::semantic