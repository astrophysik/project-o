#include "ast.h"
#include "ast_visitor.h"

namespace ast {

// class_declaration
class_declaration::class_declaration(std::string name,
                                     std::vector<std::string> type_parameters,
                                     std::optional<std::string> base_class,
                                     std::vector<std::unique_ptr<variable_declaration>> fields,
                                     std::vector<std::unique_ptr<method_declaration>> methods,
                                     std::vector<std::unique_ptr<constructor_declaration>> constructors)
    : name(std::move(name)),
      type_parameters(std::move(type_parameters)),
      base_class(std::move(base_class)),
      fields(std::move(fields)),
      methods(std::move(methods)),
      constructors(std::move(constructors)) {}
class_declaration::~class_declaration() = default;
void class_declaration::accept(visitor& v) {
    v.visit(*this);
}

// variable_declaration
variable_declaration::variable_declaration(std::string name, std::unique_ptr<expression> init)
    : name(std::move(name)),
      initializer(std::move(init)) {}
variable_declaration::~variable_declaration() = default;
void variable_declaration::accept(visitor& v) {
    v.visit(*this);
}

// parameter_declaration
parameter_declaration::parameter_declaration(std::string n, std::string t)
    : name(std::move(n)),
      type_name(std::move(t)) {}
parameter_declaration::~parameter_declaration() = default;
void parameter_declaration::accept(visitor& v) {
    v.visit(*this);
}

// method_declaration
method_declaration::method_declaration(std::string name,
                                       std::vector<std::unique_ptr<parameter_declaration>> params,
                                       std::optional<std::string> ret,
                                       std::optional<std::variant<std::unique_ptr<block>, std::unique_ptr<expression>>> body)
    : name(std::move(name)),
      parameters(std::move(params)),
      return_type(std::move(ret)),
      body(std::move(body)) {}
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

// assignment_statement
assignment_statement::assignment_statement(std::string t, std::unique_ptr<expression> v)
    : target(std::move(t)),
      value(std::move(v)) {}
assignment_statement::~assignment_statement() = default;
void assignment_statement::accept(visitor& v) {
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
return_statement::return_statement(std::unique_ptr<expression> v)
    : value(std::move(v)) {}
return_statement::~return_statement() = default;
void return_statement::accept(visitor& v) {
    v.visit(*this);
}

// literal_expression
literal_expression::literal_expression(int64_t v)
    : type(type::integer),
      value(v) {}
literal_expression::literal_expression(double v)
    : type(type::real),
      value(v) {}
literal_expression::literal_expression(bool v)
    : type(type::boolean),
      value(v) {}
literal_expression::~literal_expression() = default;
void literal_expression::accept(visitor& v) {
    v.visit(*this);
}

// this_expression
this_expression::~this_expression() = default;
void this_expression::accept(visitor& v) {
    v.visit(*this);
}

// identifier_expression
identifier_expression::identifier_expression(std::string n)
    : name(std::move(n)) {}
identifier_expression::~identifier_expression() = default;
void identifier_expression::accept(visitor& v) {
    v.visit(*this);
}

// parameterized_identifier_expression
parameterized_identifier_expression::parameterized_identifier_expression(std::string n, std::vector<std::string> args)
    : name(std::move(n)),
      type_arguments(std::move(args)) {}
parameterized_identifier_expression::~parameterized_identifier_expression() = default;
void parameterized_identifier_expression::accept(visitor& v) {
    v.visit(*this);
}

// member_expression
member_expression::member_expression(std::unique_ptr<expression> obj, std::string mem)
    : object(std::move(obj)),
      member(std::move(mem)) {}
member_expression::~member_expression() = default;
void member_expression::accept(visitor& v) {
    v.visit(*this);
}

// call_expression
call_expression::call_expression(std::unique_ptr<expression> c, std::vector<std::unique_ptr<expression>> args)
    : callee(std::move(c)),
      arguments(std::move(args)) {}
call_expression::~call_expression() = default;
void call_expression::accept(visitor& v) {
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

// program
program::program(std::vector<std::unique_ptr<class_declaration>> cls)
    : classes(std::move(cls)) {}
program::~program() = default;
void program::accept(visitor& v) {
    v.visit(*this);
}

} // namespace ast
