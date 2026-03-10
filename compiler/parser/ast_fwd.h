#pragma once

namespace ast {

class Visitor;

// base hierarchy
class Entity;
class Declaration;
class Statement;
class Expression;

// structural nodes
class Block;
class Program;

// declarations
class ClassDeclaration;
class VariableDeclaration;
class ParameterDeclaration;
class MethodDeclaration;
class ConstructorDeclaration;

// statements
class AssignmentStatement;
class WhileStatement;
class IfStatement;
class ReturnStatement;

// expressions
class LiteralExpression;
class ThisExpression;
class IdentifierExpression;
class ParameterizedIdentifierExpression;
class MemberExpression;
class CallExpression;
class GroupingExpression;

} // namespace ast