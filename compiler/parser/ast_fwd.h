#pragma once

namespace ast {

class visitor;

// base hierarchy
class entity;
class declaration;
class statement;
class expression;

// structural nodes
class block;
class program;

// declarations
class class_declaration;
class variable_declaration;
class parameter_declaration;
class method_declaration;
class constructor_declaration;

// statements
class assignment_statement;
class while_statement;
class if_statement;
class return_statement;

// expressions
class literal_expression;
class this_expression;
class identifier_expression;
class parameterized_identifier_expression;
class member_expression;
class call_expression;
class grouping_expression;

} // namespace ast