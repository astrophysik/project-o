#pragma once

namespace codegen::ast {

class visitor;

class entity;
class declaration;
class statement;
class expression;

class block;
class program;

class class_declaration;
class field_declaration;
class variable_declaration;
class parameter_declaration;
class method_declaration;
class constructor_declaration;

// statements
class variable_assignment;
class field_assignment;
class while_statement;
class if_statement;
class return_statement;

// expressions
class literal_expression;
class this_expression;
class identifier_expression;
class method_call_expression;
class constructor_call_expression;
class member_expression;
class grouping_expression;

} // namespace codegen::ast
