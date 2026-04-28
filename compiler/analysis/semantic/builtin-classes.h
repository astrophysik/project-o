#pragma once

#include "compiler/compilation-structures/symbol-table.h"
#include "compiler/compilation-structures/type-table.h"

namespace analysis::semantic::builtin {

inline void add_method(structures::class_symbol* cls,
                       const std::string& name,
                       const structures::type* return_type,
                       std::vector<const structures::type*> param_types,
                       structures::type_table& type_table) {
    auto method = std::make_unique<structures::method_symbol>(name, cls->class_scope.get(), return_type, std::move(param_types));
    cls->methods.push_back(method.get());
    cls->class_scope->add(std::move(method));
}

inline void add_constructor(structures::class_symbol* cls, std::vector<const structures::type*> param_types, structures::type_table& type_table) {
    auto ctor = std::make_unique<structures::method_symbol>(cls->name, cls->class_scope.get(), type_table.resolveType(cls->name), std::move(param_types));
    cls->constructors.push_back(std::move(ctor));
}

inline void add_field(structures::class_symbol* cls, const std::string& name, const structures::type* field_type) {
    auto field = std::make_unique<structures::variable_symbol>(name, field_type);
    cls->fields.push_back(field.get());
    cls->class_scope->add(std::move(field));
}

inline void add_builtin_classes(structures::symbol_table& sym_table, structures::type_table& type_table) {
    auto class_sym = std::make_unique<structures::class_symbol>("Class", &sym_table, nullptr);
    type_table.addClass("Class", nullptr);
    auto* class_ptr = class_sym.get();
    sym_table.add(std::move(class_sym));

    auto anyvalue_sym = std::make_unique<structures::class_symbol>("AnyValue", &sym_table, class_ptr);
    type_table.addClass("AnyValue", nullptr);
    auto* anyvalue_ptr = anyvalue_sym.get();
    sym_table.add(std::move(anyvalue_sym));

    auto anyref_sym = std::make_unique<structures::class_symbol>("AnyRef", &sym_table, class_ptr);
    type_table.addClass("AnyRef", nullptr);
    sym_table.add(std::move(anyref_sym));

    auto integer_sym = std::make_unique<structures::class_symbol>("Integer", &sym_table, anyvalue_ptr);
    type_table.addClass("Integer", nullptr);
    auto* integer_ptr = integer_sym.get();
    sym_table.add(std::move(integer_sym));

    auto real_sym = std::make_unique<structures::class_symbol>("Real", &sym_table, anyvalue_ptr);
    type_table.addClass("Real", nullptr);
    auto* real_ptr = real_sym.get();
    sym_table.add(std::move(real_sym));

    auto boolean_sym = std::make_unique<structures::class_symbol>("Boolean", &sym_table, anyvalue_ptr);
    type_table.addClass("Boolean", nullptr);
    auto* boolean_ptr = boolean_sym.get();
    sym_table.add(std::move(boolean_sym));

    auto unit_sym = std::make_unique<structures::class_symbol>("Unit", &sym_table, anyvalue_ptr);
    type_table.addClass("Unit", nullptr);
    auto* unit_ptr = unit_sym.get();
    sym_table.add(std::move(unit_sym));

    add_constructor(integer_ptr, {type_table.resolveType("Integer")}, type_table);
    add_constructor(integer_ptr, {type_table.resolveType("Real")}, type_table);

    add_field(integer_ptr, "Min", type_table.resolveType("Integer"));
    add_field(integer_ptr, "Max", type_table.resolveType("Integer"));

    add_method(integer_ptr, "toReal", type_table.resolveType("Real"), {}, type_table);
    add_method(integer_ptr, "toBoolean", type_table.resolveType("Boolean"), {}, type_table);
    add_method(integer_ptr, "UnaryMinus", type_table.resolveType("Integer"), {}, type_table);
    add_method(integer_ptr, "Plus", type_table.resolveType("Integer"), {type_table.resolveType("Integer")}, type_table);
    add_method(integer_ptr, "Plus", type_table.resolveType("Real"), {type_table.resolveType("Real")}, type_table);
    add_method(integer_ptr, "Minus", type_table.resolveType("Integer"), {type_table.resolveType("Integer")}, type_table);
    add_method(integer_ptr, "Minus", type_table.resolveType("Real"), {type_table.resolveType("Real")}, type_table);
    add_method(integer_ptr, "Mult", type_table.resolveType("Integer"), {type_table.resolveType("Integer")}, type_table);
    add_method(integer_ptr, "Mult", type_table.resolveType("Real"), {type_table.resolveType("Real")}, type_table);
    add_method(integer_ptr, "Div", type_table.resolveType("Integer"), {type_table.resolveType("Integer")}, type_table);
    add_method(integer_ptr, "Div", type_table.resolveType("Real"), {type_table.resolveType("Real")}, type_table);
    add_method(integer_ptr, "Rem", type_table.resolveType("Integer"), {type_table.resolveType("Integer")}, type_table);
    add_method(integer_ptr, "Less", type_table.resolveType("Boolean"), {type_table.resolveType("Integer")}, type_table);
    add_method(integer_ptr, "Less", type_table.resolveType("Boolean"), {type_table.resolveType("Real")}, type_table);
    add_method(integer_ptr, "LessEqual", type_table.resolveType("Boolean"), {type_table.resolveType("Integer")}, type_table);
    add_method(integer_ptr, "LessEqual", type_table.resolveType("Boolean"), {type_table.resolveType("Real")}, type_table);
    add_method(integer_ptr, "Greater", type_table.resolveType("Boolean"), {type_table.resolveType("Integer")}, type_table);
    add_method(integer_ptr, "Greater", type_table.resolveType("Boolean"), {type_table.resolveType("Real")}, type_table);
    add_method(integer_ptr, "GreaterEqual", type_table.resolveType("Boolean"), {type_table.resolveType("Integer")}, type_table);
    add_method(integer_ptr, "GreaterEqual", type_table.resolveType("Boolean"), {type_table.resolveType("Real")}, type_table);
    add_method(integer_ptr, "Equal", type_table.resolveType("Boolean"), {type_table.resolveType("Integer")}, type_table);
    add_method(integer_ptr, "Equal", type_table.resolveType("Boolean"), {type_table.resolveType("Real")}, type_table);

    add_constructor(real_ptr, {type_table.resolveType("Real")}, type_table);
    add_constructor(real_ptr, {type_table.resolveType("Integer")}, type_table);

    add_field(real_ptr, "Min", type_table.resolveType("Real"));
    add_field(real_ptr, "Max", type_table.resolveType("Real"));
    add_field(real_ptr, "Epsilon", type_table.resolveType("Real"));

    add_method(real_ptr, "toInteger", type_table.resolveType("Integer"), {}, type_table);
    add_method(real_ptr, "UnaryMinus", type_table.resolveType("Real"), {}, type_table);
    add_method(real_ptr, "Plus", type_table.resolveType("Real"), {type_table.resolveType("Real")}, type_table);
    add_method(real_ptr, "Plus", type_table.resolveType("Real"), {type_table.resolveType("Integer")}, type_table);
    add_method(real_ptr, "Minus", type_table.resolveType("Real"), {type_table.resolveType("Real")}, type_table);
    add_method(real_ptr, "Minus", type_table.resolveType("Real"), {type_table.resolveType("Integer")}, type_table);
    add_method(real_ptr, "Mult", type_table.resolveType("Real"), {type_table.resolveType("Real")}, type_table);
    add_method(real_ptr, "Mult", type_table.resolveType("Real"), {type_table.resolveType("Integer")}, type_table);
    add_method(real_ptr, "Div", type_table.resolveType("Real"), {type_table.resolveType("Real")}, type_table);
    add_method(real_ptr, "Div", type_table.resolveType("Real"), {type_table.resolveType("Integer")}, type_table);
    add_method(real_ptr, "Rem", type_table.resolveType("Real"), {type_table.resolveType("Integer")}, type_table);
    add_method(real_ptr, "Less", type_table.resolveType("Boolean"), {type_table.resolveType("Real")}, type_table);
    add_method(real_ptr, "Less", type_table.resolveType("Boolean"), {type_table.resolveType("Integer")}, type_table);
    add_method(real_ptr, "LessEqual", type_table.resolveType("Boolean"), {type_table.resolveType("Real")}, type_table);
    add_method(real_ptr, "LessEqual", type_table.resolveType("Boolean"), {type_table.resolveType("Integer")}, type_table);
    add_method(real_ptr, "Greater", type_table.resolveType("Boolean"), {type_table.resolveType("Real")}, type_table);
    add_method(real_ptr, "Greater", type_table.resolveType("Boolean"), {type_table.resolveType("Integer")}, type_table);
    add_method(real_ptr, "GreaterEqual", type_table.resolveType("Boolean"), {type_table.resolveType("Real")}, type_table);
    add_method(real_ptr, "GreaterEqual", type_table.resolveType("Boolean"), {type_table.resolveType("Integer")}, type_table);
    add_method(real_ptr, "Equal", type_table.resolveType("Boolean"), {type_table.resolveType("Real")}, type_table);
    add_method(real_ptr, "Equal", type_table.resolveType("Boolean"), {type_table.resolveType("Integer")}, type_table);

    add_constructor(boolean_ptr, {type_table.resolveType("Boolean")}, type_table);

    add_method(boolean_ptr, "toInteger", type_table.resolveType("Integer"), {}, type_table);
    add_method(boolean_ptr, "Or", type_table.resolveType("Boolean"), {type_table.resolveType("Boolean")}, type_table);
    add_method(boolean_ptr, "And", type_table.resolveType("Boolean"), {type_table.resolveType("Boolean")}, type_table);
    add_method(boolean_ptr, "Xor", type_table.resolveType("Boolean"), {type_table.resolveType("Boolean")}, type_table);
    add_method(boolean_ptr, "Not", type_table.resolveType("Boolean"), {}, type_table);
}

} // namespace analysis::semantic::builtin
