#include "compiler/compilation-structures/ast/codegen/ast.h"

#include <utility>

namespace codegen::ast {

// program
program::program(std::vector<std::unique_ptr<class_declaration>> cls)
    : classes(std::move(cls)) {}
program::~program() = default;
void program::accept(visitor& v) {
    v.visit(*this);
}

// class_declaration
class_declaration::class_declaration(std::string name,
                                     class_declaration* base_class,
                                     std::vector<std::unique_ptr<field_declaration>> fields,
                                     std::vector<std::unique_ptr<method_declaration>> methods,
                                     std::vector<std::unique_ptr<constructor_declaration>> constructors)
    : name(std::move(name)),
      base_class(base_class),
      fields(std::move(fields)),
      methods(std::move(methods)),
      constructors(std::move(constructors)) {}
class_declaration::~class_declaration() = default;
void class_declaration::accept(visitor& v) {
    v.visit(*this);
}

// field_declaration
field_declaration::field_declaration(std::string name, std::unique_ptr<expression> init, class_declaration* type, class_declaration* owner)
    : name(std::move(name)),
      initializer(std::move(init)),
      type(type),
      class_owner(owner) {}
field_declaration::~field_declaration() = default;
void field_declaration::accept(visitor& v) {
    v.visit(*this);
}

// method_declaration
method_declaration::method_declaration(std::string name,
                                       std::vector<std::unique_ptr<parameter_declaration>> params,
                                       class_declaration* ret_type,
                                       std::optional<std::variant<std::unique_ptr<block>, std::unique_ptr<expression>>> body,
                                       class_declaration* owner)
    : name(std::move(name)),
      parameters(std::move(params)),
      return_type(ret_type),
      body(std::move(body)),
      class_owner(owner) {}
method_declaration::~method_declaration() = default;
void method_declaration::accept(visitor& v) {
    v.visit(*this);
}

// constructor_declaration
constructor_declaration::constructor_declaration(std::vector<std::unique_ptr<parameter_declaration>> params, std::unique_ptr<block> body)
    : parameters(std::move(params)),
      body(std::move(body)) {}
constructor_declaration::~constructor_declaration() = default;
void constructor_declaration::accept(visitor& v) {
    v.visit(*this);
}

// parameter_declaration
parameter_declaration::parameter_declaration(std::string n, class_declaration* t)
    : name(std::move(n)),
      type(t) {}
parameter_declaration::~parameter_declaration() = default;
void parameter_declaration::accept(visitor& v) {
    v.visit(*this);
}

// variable_declaration
variable_declaration::variable_declaration(std::string name, std::unique_ptr<expression> init, class_declaration* type)
    : name(std::move(name)),
      initializer(std::move(init)),
      type(type) {}
variable_declaration::~variable_declaration() = default;
void variable_declaration::accept(visitor& v) {
    v.visit(*this);
}

// variable_assignment
variable_assignment::variable_assignment(std::variant<variable_declaration*, parameter_declaration*, field_declaration*> target,
                                         std::unique_ptr<expression> value,
                                         class_declaration* expr_type)
    : target(target),
      value(std::move(value)),
      expression_type(expr_type) {}
variable_assignment::~variable_assignment() = default;
void variable_assignment::accept(visitor& v) {
    v.visit(*this);
}

// field_assignment
field_assignment::field_assignment(std::unique_ptr<member_expression> target, std::unique_ptr<expression> value, class_declaration* expr_type)
    : target(std::move(target)),
      value(std::move(value)),
      expression_type(expr_type) {}
field_assignment::~field_assignment() = default;
void field_assignment::accept(visitor& v) {
    v.visit(*this);
}

// while_statement
while_statement::while_statement(std::unique_ptr<expression> cond, std::unique_ptr<block> body)
    : condition(std::move(cond)),
      body(std::move(body)) {}
while_statement::~while_statement() = default;
void while_statement::accept(visitor& v) {
    v.visit(*this);
}

// if_statement
if_statement::if_statement(std::unique_ptr<expression> cond, std::unique_ptr<block> t, std::unique_ptr<block> f)
    : condition(std::move(cond)),
      true_branch(std::move(t)),
      false_branch(std::move(f)) {}
if_statement::~if_statement() = default;
void if_statement::accept(visitor& v) {
    v.visit(*this);
}

// return_statement
return_statement::return_statement(std::unique_ptr<expression> v, class_declaration* expr_type)
    : value(std::move(v)),
      expression_type(expr_type) {}
return_statement::~return_statement() = default;
void return_statement::accept(visitor& v) {
    v.visit(*this);
}

// literal_expression
literal_expression::literal_expression(int64_t v, class_declaration* type)
    : type(type),
      value(v) {}
literal_expression::literal_expression(double v, class_declaration* type)
    : type(type),
      value(v) {}
literal_expression::literal_expression(bool v, class_declaration* type)
    : type(type),
      value(v) {}
literal_expression::~literal_expression() = default;
void literal_expression::accept(visitor& v) {
    v.visit(*this);
}

// this_expression
this_expression::this_expression(class_declaration* type)
    : type(type) {}
this_expression::~this_expression() = default;
void this_expression::accept(visitor& v) {
    v.visit(*this);
}

// identifier_expression
identifier_expression::identifier_expression(std::variant<variable_declaration*, parameter_declaration*, field_declaration*> target)
    : target(target) {}
identifier_expression::~identifier_expression() = default;
void identifier_expression::accept(visitor& v) {
    v.visit(*this);
}

// method_call_expression
method_call_expression::method_call_expression(std::unique_ptr<expression> obj,
                                               method_declaration* method,
                                               std::vector<std::unique_ptr<expression>> args,
                                               class_declaration* ret_type)
    : object(std::move(obj)),
      method(method),
      arguments(std::move(args)),
      return_type(ret_type) {}
method_call_expression::~method_call_expression() = default;
void method_call_expression::accept(visitor& v) {
    v.visit(*this);
}

// constructor_call_expression
constructor_call_expression::constructor_call_expression(constructor_declaration* ctor, std::vector<std::unique_ptr<expression>> args)
    : constructor(ctor),
      arguments(std::move(args)) {}
constructor_call_expression::~constructor_call_expression() = default;
void constructor_call_expression::accept(visitor& v) {
    v.visit(*this);
}

// member_expression
member_expression::member_expression(std::unique_ptr<expression> obj, field_declaration* member)
    : object(std::move(obj)),
      member(member) {}
member_expression::~member_expression() = default;
void member_expression::accept(visitor& v) {
    v.visit(*this);
}

// grouping_expression
grouping_expression::grouping_expression(std::unique_ptr<expression> expr)
    : inner(std::move(expr)) {}
grouping_expression::~grouping_expression() = default;
void grouping_expression::accept(visitor& v) {
    v.visit(*this);
}

// block
block::block(std::vector<std::unique_ptr<entity>> items)
    : items(std::move(items)) {}
block::~block() = default;
void block::accept(visitor& v) {
    v.visit(*this);
}

} // namespace codegen::ast
