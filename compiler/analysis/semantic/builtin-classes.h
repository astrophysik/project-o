#pragma once

#include "compiler/compilation-structures/ast/codegen/ast.h"
#include "compiler/compilation-structures/symbol-table.h"
#include "compiler/compilation-structures/type-table.h"

namespace analysis::semantic::builtin {

inline void add_method(structures::class_symbol* cls,
                       const std::string& name,
                       const structures::type* return_type,
                       std::vector<const structures::type*> param_types,
                       structures::type_table& type_table) {
    std::string mangled = structures::mangle_method_name(name, param_types);
    auto method = std::make_unique<structures::method_symbol>(mangled, name, cls->class_scope.get(), return_type, std::move(param_types));
    cls->methods.push_back(method.get());
    cls->class_scope->add(std::move(method));
}

inline void add_constructor(structures::class_symbol* cls, std::vector<const structures::type*> param_types, structures::type_table& type_table) {
    std::string mangled = structures::mangle_method_name(cls->name, param_types);
    auto ctor =
        std::make_unique<structures::method_symbol>(mangled, cls->name, cls->class_scope.get(), type_table.resolveType(cls->name), std::move(param_types));
    cls->constructors.push_back(ctor.get());
    cls->class_scope->add(std::move(ctor));
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

    auto anyvalue_sym = std::make_unique<structures::class_symbol>("AnyValue", class_ptr->class_scope.get(), class_ptr);
    type_table.addClass("AnyValue", nullptr);
    auto* anyvalue_ptr = anyvalue_sym.get();
    sym_table.add(std::move(anyvalue_sym));

    auto anyref_sym = std::make_unique<structures::class_symbol>("AnyRef", class_ptr->class_scope.get(), class_ptr);
    type_table.addClass("AnyRef", nullptr);
    auto* anyref_ptr = anyref_sym.get();
    sym_table.add(std::move(anyref_sym));

    auto integer_sym = std::make_unique<structures::class_symbol>("Integer", anyvalue_ptr->class_scope.get(), anyvalue_ptr);
    type_table.addClass("Integer", nullptr);
    auto* integer_ptr = integer_sym.get();
    sym_table.add(std::move(integer_sym));

    auto real_sym = std::make_unique<structures::class_symbol>("Real", anyvalue_ptr->class_scope.get(), anyvalue_ptr);
    type_table.addClass("Real", nullptr);
    auto* real_ptr = real_sym.get();
    sym_table.add(std::move(real_sym));

    auto boolean_sym = std::make_unique<structures::class_symbol>("Boolean", anyvalue_ptr->class_scope.get(), anyvalue_ptr);
    type_table.addClass("Boolean", nullptr);
    auto* boolean_ptr = boolean_sym.get();
    sym_table.add(std::move(boolean_sym));

    auto unit_sym = std::make_unique<structures::class_symbol>("Unit", anyvalue_ptr->class_scope.get(), anyvalue_ptr);
    type_table.addClass("Unit", nullptr);
    auto* unit_ptr = unit_sym.get();
    sym_table.add(std::move(unit_sym));

    auto array_integer_sym = std::make_unique<structures::class_symbol>("ArrayInteger", anyref_ptr->class_scope.get(), anyref_ptr);
    type_table.addClass("ArrayInteger", nullptr);
    auto *array_integer_ptr = array_integer_sym.get();
    sym_table.add(std::move(array_integer_sym));

    auto io_sym = std::make_unique<structures::class_symbol>("IO", anyvalue_ptr->class_scope.get(), anyvalue_ptr);
    type_table.addClass("IO", nullptr);
    auto *io_ptr = io_sym.get();
    sym_table.add(std::move(io_sym));

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

    add_constructor(unit_ptr, {}, type_table);

    add_constructor(array_integer_ptr, {type_table.resolveType("Integer")}, type_table);
    add_method(array_integer_ptr, "Len", type_table.resolveType("Integer"), {}, type_table);
    add_method(array_integer_ptr, "Get", type_table.resolveType("Integer"), {type_table.resolveType("Integer")}, type_table);
    add_method(array_integer_ptr, "Set", type_table.resolveType("Unit"), {type_table.resolveType("Integer"), type_table.resolveType("Integer")}, type_table);

    add_constructor(io_ptr, {}, type_table);
    add_method(io_ptr, "Print", type_table.resolveType("Unit"), {type_table.resolveType("Integer")}, type_table);
    add_method(io_ptr, "Print", type_table.resolveType("Unit"), {type_table.resolveType("Real")}, type_table);
    add_method(io_ptr, "Print", type_table.resolveType("Unit"), {type_table.resolveType("Boolean")}, type_table);
}

inline void add_codegen_field(codegen::ast::class_declaration* cls, const std::string& name, codegen::ast::class_declaration* field_type) {
    auto field = std::make_unique<codegen::ast::field_declaration>();
    field->name = name;
    field->type = field_type;
    field->class_owner = cls;
    cls->fields.push_back(std::move(field));
}

inline void add_codegen_method(codegen::ast::class_declaration* cls,
                               const std::string& name,
                               codegen::ast::class_declaration* return_type,
                               std::vector<std::pair<std::string, codegen::ast::class_declaration*>> params) {
    auto method = std::make_unique<codegen::ast::method_declaration>();
    method->name = name;
    method->return_type = return_type;
    method->class_owner = cls;

    for (auto& [param_name, param_type] : params) {
        auto param = std::make_unique<codegen::ast::parameter_declaration>();
        param->name = param_name;
        param->type = param_type;
        method->parameters.push_back(std::move(param));
    }

    cls->methods.push_back(std::move(method));
}

inline void add_codegen_constructor(codegen::ast::class_declaration* cls, std::vector<std::pair<std::string, codegen::ast::class_declaration*>> params) {
    auto ctor = std::make_unique<codegen::ast::constructor_declaration>();
    ctor->class_owner = cls;

    for (auto& [param_name, param_type] : params) {
        auto param = std::make_unique<codegen::ast::parameter_declaration>();
        param->name = param_name;
        param->type = param_type;
        ctor->parameters.push_back(std::move(param));
    }

    ctor->body = std::make_unique<codegen::ast::block>();

    cls->constructors.push_back(std::move(ctor));
}

inline void add_builtin_classes_to_codegen(codegen::ast::program& program) {
    auto create_builtin_class = [](const std::string& name, codegen::ast::class_declaration* base = nullptr) {
        auto cls = std::make_unique<codegen::ast::class_declaration>();
        cls->name = name;
        cls->base_class = base;
        return cls;
    };

    auto class_cls = create_builtin_class("Class");
    auto* class_ptr = class_cls.get();
    program.internal_classes.push_back(std::move(class_cls));

    auto anyvalue_cls = create_builtin_class("AnyValue", class_ptr);
    auto* anyvalue_ptr = anyvalue_cls.get();
    program.internal_classes.push_back(std::move(anyvalue_cls));

    auto anyref_cls = create_builtin_class("AnyRef", class_ptr);
    auto* anyref_ptr = anyref_cls.get();
    program.internal_classes.push_back(std::move(anyref_cls));

    auto integer_cls = create_builtin_class("Integer", anyvalue_ptr);
    auto* integer_ptr = integer_cls.get();
    program.internal_classes.push_back(std::move(integer_cls));

    auto real_cls = create_builtin_class("Real", anyvalue_ptr);
    auto* real_ptr = real_cls.get();
    program.internal_classes.push_back(std::move(real_cls));

    auto boolean_cls = create_builtin_class("Boolean", anyvalue_ptr);
    auto* boolean_ptr = boolean_cls.get();
    program.internal_classes.push_back(std::move(boolean_cls));

    auto unit_cls = create_builtin_class("Unit", anyvalue_ptr);
    auto* unit_ptr = unit_cls.get();
    program.internal_classes.push_back(std::move(unit_cls));

    auto array_integer_cls = create_builtin_class("ArrayInteger", anyref_ptr);
    auto *array_integer_ptr = array_integer_cls.get();
    program.internal_classes.push_back(std::move(array_integer_cls));

    auto io_cls = create_builtin_class("IO", anyvalue_ptr);
    auto *io_ptr = io_cls.get();
    program.internal_classes.push_back(std::move(io_cls));

    // Add Inreger members
    add_codegen_constructor(integer_ptr, {{"p", integer_ptr}});
    add_codegen_constructor(integer_ptr, {{"p", real_ptr}});
    add_codegen_field(integer_ptr, "Min", integer_ptr);
    add_codegen_field(integer_ptr, "Max", integer_ptr);
    add_codegen_method(integer_ptr, "toReal", real_ptr, {});
    add_codegen_method(integer_ptr, "toBoolean", boolean_ptr, {});
    add_codegen_method(integer_ptr, "UnaryMinus", integer_ptr, {});
    add_codegen_method(integer_ptr, "Plus", integer_ptr, {{"p", integer_ptr}});
    add_codegen_method(integer_ptr, "Plus", real_ptr, {{"p", real_ptr}});
    add_codegen_method(integer_ptr, "Minus", integer_ptr, {{"p", integer_ptr}});
    add_codegen_method(integer_ptr, "Minus", real_ptr, {{"p", real_ptr}});
    add_codegen_method(integer_ptr, "Mult", integer_ptr, {{"p", integer_ptr}});
    add_codegen_method(integer_ptr, "Mult", real_ptr, {{"p", real_ptr}});
    add_codegen_method(integer_ptr, "Div", integer_ptr, {{"p", integer_ptr}});
    add_codegen_method(integer_ptr, "Div", real_ptr, {{"p", real_ptr}});
    add_codegen_method(integer_ptr, "Rem", integer_ptr, {{"p", integer_ptr}});
    add_codegen_method(integer_ptr, "Less", boolean_ptr, {{"p", integer_ptr}});
    add_codegen_method(integer_ptr, "Less", boolean_ptr, {{"p", real_ptr}});
    add_codegen_method(integer_ptr, "LessEqual", boolean_ptr, {{"p", integer_ptr}});
    add_codegen_method(integer_ptr, "LessEqual", boolean_ptr, {{"p", real_ptr}});
    add_codegen_method(integer_ptr, "Greater", boolean_ptr, {{"p", integer_ptr}});
    add_codegen_method(integer_ptr, "Greater", boolean_ptr, {{"p", real_ptr}});
    add_codegen_method(integer_ptr, "GreaterEqual", boolean_ptr, {{"p", integer_ptr}});
    add_codegen_method(integer_ptr, "GreaterEqual", boolean_ptr, {{"p", real_ptr}});
    add_codegen_method(integer_ptr, "Equal", boolean_ptr, {{"p", integer_ptr}});
    add_codegen_method(integer_ptr, "Equal", boolean_ptr, {{"p", real_ptr}});

    // Add Real members
    add_codegen_constructor(real_ptr, {{"p", real_ptr}});
    add_codegen_constructor(real_ptr, {{"p", integer_ptr}});
    add_codegen_field(real_ptr, "Min", real_ptr);
    add_codegen_field(real_ptr, "Max", real_ptr);
    add_codegen_field(real_ptr, "Epsilon", real_ptr);
    add_codegen_method(real_ptr, "toInteger", integer_ptr, {});
    add_codegen_method(real_ptr, "UnaryMinus", real_ptr, {});
    add_codegen_method(real_ptr, "Plus", real_ptr, {{"p", real_ptr}});
    add_codegen_method(real_ptr, "Plus", real_ptr, {{"p", integer_ptr}});
    add_codegen_method(real_ptr, "Minus", real_ptr, {{"p", real_ptr}});
    add_codegen_method(real_ptr, "Minus", real_ptr, {{"p", integer_ptr}});
    add_codegen_method(real_ptr, "Mult", real_ptr, {{"p", real_ptr}});
    add_codegen_method(real_ptr, "Mult", real_ptr, {{"p", integer_ptr}});
    add_codegen_method(real_ptr, "Div", real_ptr, {{"p", real_ptr}});
    add_codegen_method(real_ptr, "Div", real_ptr, {{"p", integer_ptr}});
    add_codegen_method(real_ptr, "Rem", real_ptr, {{"p", integer_ptr}});
    add_codegen_method(real_ptr, "Less", boolean_ptr, {{"p", real_ptr}});
    add_codegen_method(real_ptr, "Less", boolean_ptr, {{"p", integer_ptr}});
    add_codegen_method(real_ptr, "LessEqual", boolean_ptr, {{"p", real_ptr}});
    add_codegen_method(real_ptr, "LessEqual", boolean_ptr, {{"p", integer_ptr}});
    add_codegen_method(real_ptr, "Greater", boolean_ptr, {{"p", real_ptr}});
    add_codegen_method(real_ptr, "Greater", boolean_ptr, {{"p", integer_ptr}});
    add_codegen_method(real_ptr, "GreaterEqual", boolean_ptr, {{"p", real_ptr}});
    add_codegen_method(real_ptr, "GreaterEqual", boolean_ptr, {{"p", integer_ptr}});
    add_codegen_method(real_ptr, "Equal", boolean_ptr, {{"p", real_ptr}});
    add_codegen_method(real_ptr, "Equal", boolean_ptr, {{"p", integer_ptr}});

    // Add Boolean members
    add_codegen_constructor(boolean_ptr, {{"p", boolean_ptr}});
    add_codegen_method(boolean_ptr, "toInteger", integer_ptr, {});
    add_codegen_method(boolean_ptr, "Or", boolean_ptr, {{"p", boolean_ptr}});
    add_codegen_method(boolean_ptr, "And", boolean_ptr, {{"p", boolean_ptr}});
    add_codegen_method(boolean_ptr, "Xor", boolean_ptr, {{"p", boolean_ptr}});
    add_codegen_method(boolean_ptr, "Not", boolean_ptr, {});

    // Add Unit constructor
    add_codegen_constructor(unit_ptr, {});

    // Add Array Integer members
    add_codegen_constructor(array_integer_ptr, {{"size", integer_ptr}});
    add_codegen_method(array_integer_ptr, "Len", integer_ptr, {});
    add_codegen_method(array_integer_ptr, "Get", integer_ptr, {{"i", integer_ptr}});
    add_codegen_method(array_integer_ptr, "Set", unit_ptr, {{"i", integer_ptr}, {"v", integer_ptr}});

    // Add IO members
    add_codegen_constructor(io_ptr, {});
    add_codegen_method(io_ptr, "Print", unit_ptr, {{"p", integer_ptr}});
    add_codegen_method(io_ptr, "Print", unit_ptr, {{"p", real_ptr}});
    add_codegen_method(io_ptr, "Print", unit_ptr, {{"p", boolean_ptr}});
}

} // namespace analysis::semantic::builtin
