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

class block : public entity {
public:
    std::vector<std::unique_ptr<entity>> items;

    block() = default;
    explicit block(std::vector<std::unique_ptr<entity>> items)
        : items(std::move(items)) {}

    void accept(visitor& visitor) override;
};

class program : public entity {
public:
    std::vector<std::unique_ptr<class_declaration>> classes;

    program() = default;
    explicit program(std::vector<std::unique_ptr<class_declaration>> cls)
        : classes(std::move(cls)) {}

    void accept(visitor& visitor) override;
};

// |------------|
// |declarations|
// |------------|
class class_declaration : public declaration {
public:
    std::string name;
    std::vector<std::string> type_parameters; // for Array[int]??? or better IntArray???
    std::optional<std::string> base_class;
    std::vector<std::unique_ptr<variable_declaration>> fields;
    std::vector<std::unique_ptr<method_declaration>> methods;
    std::vector<std::unique_ptr<constructor_declaration>> constructors;

    class_declaration(std::string name, std::vector<std::string> type_parameters, std::optional<std::string> base_class,
                     std::vector<std::unique_ptr<variable_declaration>> fields, std::vector<std::unique_ptr<method_declaration>> methods,
                     std::vector<std::unique_ptr<constructor_declaration>> constructors)
        : name(std::move(name)),
          type_parameters(std::move(type_parameters)),
          base_class(std::move(base_class)),
          fields(std::move(fields)),
          methods(std::move(methods)),
          constructors(std::move(constructors)) {}

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

    variable_declaration(std::string name, std::unique_ptr<expression> init)
        : name(std::move(name)),
          initializer(std::move(init)) {}

    void accept(visitor& visitor) override;
};

class parameter_declaration : public declaration {
public:
    std::string name;
    std::string type_name; // TODO: type hierarchy, not string

    parameter_declaration(std::string n, std::string t)
        : name(std::move(n)),
          type_name(std::move(t)) {}

    void accept(visitor& visitor) override;
};

class method_declaration : public declaration {
public:
    std::string name;
    std::vector<std::unique_ptr<parameter_declaration>> parameters;
    std::optional<std::string> return_type; // nothing or type. TODO: type hierarchy, not string
    std::optional<std::variant<std::unique_ptr<block>, std::unique_ptr<expression>>> body;

    method_declaration(std::string name, std::vector<std::unique_ptr<parameter_declaration>> params, std::optional<std::string> ret,
                      std::optional<std::variant<std::unique_ptr<block>, std::unique_ptr<expression>>> body)
        : name(std::move(name)),
          parameters(std::move(params)),
          return_type(std::move(ret)),
          body(std::move(body)) {}

    void accept(visitor& visitor) override;
};

class constructor_declaration : public declaration {
public:
    std::vector<std::unique_ptr<parameter_declaration>> parameters;
    std::unique_ptr<block> body;

    constructor_declaration(std::vector<std::unique_ptr<parameter_declaration>> params, std::unique_ptr<block> body)
        : parameters(std::move(params)),
          body(std::move(body)) {}

    void accept(visitor& visitor) override;
};

// |----------|
// |statements|
// |----------|
/**
 *   x := 55
 *   ^^^^^^^
 */
class assignment_statement : public statement {
public:
    std::string target;
    std::unique_ptr<expression> value;

    assignment_statement(std::string t, std::unique_ptr<expression> v)
        : target(std::move(t)),
          value(std::move(v)) {}

    void accept(visitor& visitor) override;
};

class while_statement : public statement {
public:
    std::unique_ptr<expression> condition;
    std::unique_ptr<block> body;

    while_statement(std::unique_ptr<expression> cond, std::unique_ptr<block> body)
        : condition(std::move(cond)),
          body(std::move(body)) {}

    void accept(visitor& visitor) override;
};

class if_statement : public statement {
public:
    std::unique_ptr<expression> condition;
    std::unique_ptr<block> true_branch;
    std::unique_ptr<block> false_branch;

    if_statement(std::unique_ptr<expression> cond, std::unique_ptr<block> t, std::unique_ptr<block> f)
        : condition(std::move(cond)),
          true_branch(std::move(t)),
          false_branch(std::move(f)) {}

    void accept(visitor& visitor) override;
};

class return_statement : public statement {
public:
    std::unique_ptr<expression> value;

    explicit return_statement(std::unique_ptr<expression> v)
        : value(std::move(v)) {}

    void accept(visitor& visitor) override;
};

// |-----------|
// |expressions|
// |-----------|
class literal_expression : public expression {
public:
    enum class type { integer, real, boolean } type;
    std::variant<int64_t, double, bool> value;

    literal_expression(int64_t v)
        : type(type::integer),
          value(v) {}
    literal_expression(double v)
        : type(type::real),
          value(v) {}
    literal_expression(bool v)
        : type(type::boolean),
          value(v) {}

    void accept(visitor& visitor) override;
};

class this_expression : public expression {
public:
    void accept(visitor& visitor) override;
};

class identifier_expression : public expression {
public:
    std::string name;

    explicit identifier_expression(std::string n)
        : name(std::move(n)) {}
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

    parameterized_identifier_expression(std::string n, std::vector<std::string> args)
        : name(std::move(n)),
          type_arguments(std::move(args)) {}
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

    member_expression(std::unique_ptr<expression> obj, std::string mem)
        : object(std::move(obj)),
          member(std::move(mem)) {}
    void accept(visitor& visitor) override;
};

class call_expression : public expression {
public:
    std::unique_ptr<expression> callee;
    std::vector<std::unique_ptr<expression>> arguments;

    call_expression(std::unique_ptr<expression> c, std::vector<std::unique_ptr<expression>> args)
        : callee(std::move(c)),
          arguments(std::move(args)) {}
    void accept(visitor& visitor) override;
};

class grouping_expression : public expression {
public:
    std::unique_ptr<expression> inner;

    explicit grouping_expression(std::unique_ptr<expression> expr)
        : inner(std::move(expr)) {}
    void accept(visitor& visitor) override;
};

} // namespace ast