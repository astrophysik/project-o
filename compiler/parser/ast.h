#pragma once

#include "ast_fwd.h"
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ast {

class entity {
public:
    virtual ~entity() = default;
    virtual void accept(visitor& visitor) = 0;
};

class declaration : public entity {};
class statement : public entity {};
class expression : public entity {};

// |------------|
// |Declarations|
// |------------|
class class_declaration : public declaration {
public:
    std::string name;
    std::vector<std::string> type_parameters;
    std::optional<std::string> base_class;
    std::vector<std::unique_ptr<variable_declaration>> fields;
    std::vector<std::unique_ptr<method_declaration>> methods;
    std::vector<std::unique_ptr<constructor_declaration>> constructors;

    class_declaration() = default;
    class_declaration(std::string name,
                      std::vector<std::string> type_parameters,
                      std::optional<std::string> base_class,
                      std::vector<std::unique_ptr<variable_declaration>> fields,
                      std::vector<std::unique_ptr<method_declaration>> methods,
                      std::vector<std::unique_ptr<constructor_declaration>> constructors);
    ~class_declaration() override;

    void accept(visitor& visitor) override;
};

/**
 *   var x : 10
 *   ^^^^^^^^^^
 */
class variable_declaration : public declaration {
public:
    std::string name;
    std::unique_ptr<expression> initializer;

    variable_declaration() = default;
    variable_declaration(std::string name, std::unique_ptr<expression> init);
    ~variable_declaration() override;

    void accept(visitor& visitor) override;
};

class parameter_declaration : public declaration {
public:
    std::string name;
    std::string type_name;

    parameter_declaration() = default;
    parameter_declaration(std::string n, std::string t);
    ~parameter_declaration() override;

    void accept(visitor& visitor) override;
};

class method_declaration : public declaration {
public:
    std::string name;
    std::vector<std::unique_ptr<parameter_declaration>> parameters;
    std::optional<std::string> return_type;
    std::optional<std::variant<std::unique_ptr<block>, std::unique_ptr<expression>>> body;

    method_declaration() = default;
    method_declaration(std::string name,
                       std::vector<std::unique_ptr<parameter_declaration>> params,
                       std::optional<std::string> ret,
                       std::optional<std::variant<std::unique_ptr<block>, std::unique_ptr<expression>>> body);
    ~method_declaration() override;

    void accept(visitor& visitor) override;
};

class constructor_declaration : public declaration {
public:
    std::vector<std::unique_ptr<parameter_declaration>> parameters;
    std::unique_ptr<block> body;

    constructor_declaration() = default;
    constructor_declaration(std::vector<std::unique_ptr<parameter_declaration>> params, std::unique_ptr<block> body);
    ~constructor_declaration() override;

    void accept(visitor& visitor) override;
};

// |----------|
// |Statements|
// |----------|
/**
 *   x := 55
 *   ^^^^^^^
 */
class assignment_statement : public statement {
public:
    std::string target;
    std::unique_ptr<expression> value;

    assignment_statement() = default;
    assignment_statement(std::string t, std::unique_ptr<expression> v);
    ~assignment_statement() override;

    void accept(visitor& visitor) override;
};

class while_statement : public statement {
public:
    std::unique_ptr<expression> condition;
    std::unique_ptr<block> body;

    while_statement() = default;
    while_statement(std::unique_ptr<expression> cond, std::unique_ptr<block> body);
    ~while_statement() override;

    void accept(visitor& visitor) override;
};

class if_statement : public statement {
public:
    std::unique_ptr<expression> condition;
    std::unique_ptr<block> true_branch;
    std::unique_ptr<block> false_branch;

    if_statement() = default;
    if_statement(std::unique_ptr<expression> cond, std::unique_ptr<block> t, std::unique_ptr<block> f);
    ~if_statement() override;

    void accept(visitor& visitor) override;
};

class return_statement : public statement {
public:
    std::unique_ptr<expression> value;

    return_statement() = default;
    explicit return_statement(std::unique_ptr<expression> v);
    ~return_statement() override;

    void accept(visitor& visitor) override;
};

// |-----------|
// |Expressions|
// |-----------|
class literal_expression : public expression {
public:
    enum class type { integer, real, boolean } type;
    std::variant<int64_t, double, bool> value;

    literal_expression() = default;
    literal_expression(int64_t v);
    literal_expression(double v);
    literal_expression(bool v);
    ~literal_expression() override;

    void accept(visitor& visitor) override;
};

class this_expression : public expression {
public:
    this_expression() = default;
    ~this_expression() override;

    void accept(visitor& visitor) override;
};

class identifier_expression : public expression {
public:
    std::string name;

    identifier_expression() = default;
    explicit identifier_expression(std::string n);
    ~identifier_expression() override;

    void accept(visitor& visitor) override;
};

/**
 * Array[Integer]
 * ^^^^^^^^^^^^^^
 */
class parameterized_identifier_expression : public expression {
public:
    std::string name;
    std::vector<std::string> type_arguments;

    parameterized_identifier_expression() = default;
    parameterized_identifier_expression(std::string n, std::vector<std::string> args);
    ~parameterized_identifier_expression() override;

    void accept(visitor& visitor) override;
};

/**
 * obj.field
 * ^^^^^^^^^
 */
class member_expression : public expression {
public:
    std::unique_ptr<expression> object;
    std::string member;

    member_expression() = default;
    member_expression(std::unique_ptr<expression> obj, std::string mem);
    ~member_expression() override;

    void accept(visitor& visitor) override;
};

class call_expression : public expression {
public:
    std::unique_ptr<expression> callee;
    std::vector<std::unique_ptr<expression>> arguments;

    call_expression() = default;
    call_expression(std::unique_ptr<expression> c, std::vector<std::unique_ptr<expression>> args);
    ~call_expression() override;

    void accept(visitor& visitor) override;
};

class grouping_expression : public expression {
public:
    std::unique_ptr<expression> inner;

    grouping_expression() = default;
    explicit grouping_expression(std::unique_ptr<expression> expr);
    ~grouping_expression() override;

    void accept(visitor& visitor) override;
};

// |--------------|
// |Structural    |
// |--------------|
class block : public entity {
public:
    std::vector<std::unique_ptr<entity>> items;

    block() = default;
    explicit block(std::vector<std::unique_ptr<entity>> items);
    ~block() override;

    void accept(visitor& visitor) override;
};

class program : public entity {
public:
    std::vector<std::unique_ptr<class_declaration>> classes;

    program() = default;
    explicit program(std::vector<std::unique_ptr<class_declaration>> cls);
    ~program() override;

    void accept(visitor& visitor) override;
};

} // namespace ast
