#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "compiler/compilation-structures/ast/codegen/ast-forward-declarations.h"
#include "compiler/compilation-structures/ast/codegen/ast-visitor.h"

namespace codegen::ast {

struct entity {
    virtual ~entity() = default;
    virtual void accept(visitor& visitor) = 0;
};

struct declaration : public entity {};
struct statement : public entity {};
struct expression : public entity {};

struct program : public entity {
    std::vector<std::unique_ptr<class_declaration>> internal_classes;
    std::vector<std::unique_ptr<class_declaration>> classes;

    program() = default;
    explicit program(std::vector<std::unique_ptr<class_declaration>> cls);
    ~program() override;

    void accept(visitor& visitor) override;
};

struct class_declaration : public declaration {
    std::string name;
    class_declaration* base_class;
    std::vector<std::unique_ptr<field_declaration>> fields;
    std::vector<std::unique_ptr<method_declaration>> methods;
    std::vector<std::unique_ptr<constructor_declaration>> constructors;

    class_declaration() = default;
    explicit class_declaration(std::string name,
                               class_declaration* base_class,
                               std::vector<std::unique_ptr<field_declaration>> fields,
                               std::vector<std::unique_ptr<method_declaration>> methods,
                               std::vector<std::unique_ptr<constructor_declaration>> constructors);
    ~class_declaration() override;

    void accept(visitor& visitor) override;
};

struct field_declaration : public statement {
    std::string name;
    std::unique_ptr<expression> initializer;
    class_declaration* type;
    class_declaration* class_owner;

    field_declaration() = default;
    explicit field_declaration(std::string name, std::unique_ptr<expression> init, class_declaration* type, class_declaration* owner);
    ~field_declaration() override;

    void accept(visitor& visitor) override;
};

struct method_declaration : public declaration {
    std::string name;
    std::vector<std::unique_ptr<parameter_declaration>> parameters;
    class_declaration* return_type;
    std::optional<std::variant<std::unique_ptr<block>, std::unique_ptr<expression>>> body;
    class_declaration* class_owner;

    method_declaration() = default;
    explicit method_declaration(std::string name,
                                std::vector<std::unique_ptr<parameter_declaration>> params,
                                class_declaration* ret_type,
                                std::optional<std::variant<std::unique_ptr<block>, std::unique_ptr<expression>>> body,
                                class_declaration* owner);
    ~method_declaration() override;

    void accept(visitor& visitor) override;
};

struct constructor_declaration : public declaration {
    std::vector<std::unique_ptr<parameter_declaration>> parameters;
    std::unique_ptr<constructor_call_expression> super_constructor;
    std::unique_ptr<block> body;
    class_declaration* class_owner;

    constructor_declaration() = default;
    explicit constructor_declaration(std::vector<std::unique_ptr<parameter_declaration>> params, std::unique_ptr<block> body);
    ~constructor_declaration() override;

    void accept(visitor& visitor) override;
};

struct parameter_declaration : public declaration {
    std::string name;
    class_declaration* type;

    parameter_declaration() = default;
    explicit parameter_declaration(std::string n, class_declaration* t);
    ~parameter_declaration() override;

    void accept(visitor& visitor) override;
};

struct variable_declaration : public statement {
    std::string name;
    std::unique_ptr<expression> initializer;
    class_declaration* type;

    variable_declaration() = default;
    explicit variable_declaration(std::string name, std::unique_ptr<expression> init, class_declaration* type);
    ~variable_declaration() override;

    void accept(visitor& visitor) override;
};

struct variable_assignment : public statement {
    std::variant<variable_declaration*, parameter_declaration*, field_declaration*> target;
    std::unique_ptr<expression> value;
    class_declaration* expression_type;

    variable_assignment() = default;
    explicit variable_assignment(std::variant<variable_declaration*, parameter_declaration*, field_declaration*> target,
                                 std::unique_ptr<expression> value,
                                 class_declaration* expr_type);
    ~variable_assignment() override;
    void accept(visitor& visitor) override;
};

struct field_assignment : public statement {
    std::unique_ptr<member_expression> target;
    std::unique_ptr<expression> value;
    class_declaration* expression_type;

    field_assignment() = default;
    explicit field_assignment(std::unique_ptr<member_expression> target, std::unique_ptr<expression> value, class_declaration* expr_type);
    ~field_assignment() override;
    void accept(visitor& visitor) override;
};

struct while_statement : public statement {
public:
    std::unique_ptr<expression> condition;
    std::unique_ptr<block> body;

    while_statement() = default;
    while_statement(std::unique_ptr<expression> cond, std::unique_ptr<block> body);
    ~while_statement() override;

    void accept(visitor& visitor) override;
};

struct if_statement : public statement {
public:
    std::unique_ptr<expression> condition;
    std::unique_ptr<block> true_branch;
    std::unique_ptr<block> false_branch;

    if_statement() = default;
    if_statement(std::unique_ptr<expression> cond, std::unique_ptr<block> t, std::unique_ptr<block> f);
    ~if_statement() override;

    void accept(visitor& visitor) override;
};

struct return_statement : public statement {
    std::unique_ptr<expression> value;
    class_declaration* expression_type;

    return_statement() = default;
    explicit return_statement(std::unique_ptr<expression> v, class_declaration* expr_type);
    ~return_statement() override;

    void accept(visitor& visitor) override;
};

struct literal_expression : public expression {
    class_declaration* type;
    std::variant<int64_t, double, bool> value;

    literal_expression() = default;
    literal_expression(int64_t v, class_declaration* type);
    literal_expression(double v, class_declaration* type);
    literal_expression(bool v, class_declaration* type);
    ~literal_expression() override;

    void accept(visitor& visitor) override;
};

struct this_expression : public expression {
    class_declaration* type;

    this_expression() = default;
    explicit this_expression(class_declaration* type);
    ~this_expression() override;

    void accept(visitor& visitor) override;
};

struct identifier_expression : public expression {
    std::variant<variable_declaration*, parameter_declaration*, field_declaration*> target;

    identifier_expression() = default;
    explicit identifier_expression(std::variant<variable_declaration*, parameter_declaration*, field_declaration*> target);
    ~identifier_expression() override;

    void accept(visitor& visitor) override;
};

struct method_call_expression : public expression {
    std::unique_ptr<expression> object;
    method_declaration* method;
    std::vector<std::unique_ptr<expression>> arguments;
    class_declaration* return_type;

    method_call_expression() = default;
    explicit method_call_expression(std::unique_ptr<expression> obj,
                                    method_declaration* method,
                                    std::vector<std::unique_ptr<expression>> args,
                                    class_declaration* ret_type);
    ~method_call_expression() override;

    void accept(visitor& visitor) override;
};

struct constructor_call_expression : public expression {
    constructor_declaration* constructor;
    std::vector<std::unique_ptr<expression>> arguments;

    constructor_call_expression() = default;
    explicit constructor_call_expression(constructor_declaration* ctor, std::vector<std::unique_ptr<expression>> args);
    ~constructor_call_expression() override;

    void accept(visitor& visitor) override;
};

struct member_expression : public expression {
    std::unique_ptr<expression> object;
    field_declaration* member;

    member_expression() = default;
    member_expression(std::unique_ptr<expression> obj, field_declaration* member);
    ~member_expression() override;

    void accept(visitor& visitor) override;
};

struct grouping_expression : public expression {
public:
    std::unique_ptr<expression> inner;

    grouping_expression() = default;
    explicit grouping_expression(std::unique_ptr<expression> expr);
    ~grouping_expression() override;

    void accept(visitor& visitor) override;
};

struct block : public entity {
public:
    std::vector<std::unique_ptr<entity>> items;

    block() = default;
    explicit block(std::vector<std::unique_ptr<entity>> items);
    ~block() override;

    void accept(visitor& visitor) override;
};
} // namespace codegen::ast
