#pragma once

#include "ast_fwd.h"
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ast {

class Entity {
public:
    virtual ~Entity() = default;
    virtual void accept(Visitor& visitor) = 0;
};

class Declaration : public Entity {};

class Statement : public Entity {};

class Expression : public Entity {};

class Block : public Entity {
public:
    std::vector<std::unique_ptr<Entity>> items;

    Block() = default;
    explicit Block(std::vector<std::unique_ptr<Entity>> items)
        : items(std::move(items)) {}

    void accept(Visitor& visitor) override;
};

class Program : public Entity {
public:
    std::vector<std::unique_ptr<ClassDeclaration>> classes;

    Program() = default;
    explicit Program(std::vector<std::unique_ptr<ClassDeclaration>> cls)
        : classes(std::move(cls)) {}

    void accept(Visitor& visitor) override;
};

// |------------|
// |Declarations|
// |------------|
class ClassDeclaration : public Declaration {
public:
    std::string name;
    std::vector<std::string> typeParameters; // for Array[int]??? or better IntArray???
    std::optional<std::string> baseClass;
    std::vector<std::unique_ptr<VariableDeclaration>> fields;
    std::vector<std::unique_ptr<MethodDeclaration>> methods;
    std::vector<std::unique_ptr<ConstructorDeclaration>> constructors;

    ClassDeclaration(std::string name, std::vector<std::string> typeParameters, std::optional<std::string> baseClass,
                     std::vector<std::unique_ptr<VariableDeclaration>> fields, std::vector<std::unique_ptr<MethodDeclaration>> methods,
                     std::vector<std::unique_ptr<ConstructorDeclaration>> constructors)
        : name(std::move(name)),
          typeParameters(std::move(typeParameters)),
          baseClass(std::move(baseClass)),
          fields(std::move(fields)),
          methods(std::move(methods)),
          constructors(std::move(constructors)) {}

    void accept(Visitor& visitor) override;
};

/**
 *   var x : 10
 *   ^^^^^^^^^^
 */
class VariableDeclaration : public Declaration {
public:
    std::string name;
    std::unique_ptr<Expression> initializer;

    VariableDeclaration(std::string name, std::unique_ptr<Expression> init)
        : name(std::move(name)),
          initializer(std::move(init)) {}

    void accept(Visitor& visitor) override;
};

class ParameterDeclaration : public Declaration {
public:
    std::string name;
    std::string typeName; // TODO: type hierarchy, not string

    ParameterDeclaration(std::string n, std::string t)
        : name(std::move(n)),
          typeName(std::move(t)) {}

    void accept(Visitor& visitor) override;
};

class MethodDeclaration : public Declaration {
public:
    std::string name;
    std::vector<std::unique_ptr<ParameterDeclaration>> parameters;
    std::optional<std::string> returnType; // nothing or type. TODO: type hierarchy, not string
    std::optional<std::variant<std::unique_ptr<Block>, std::unique_ptr<Expression>>> body;

    MethodDeclaration(std::string name, std::vector<std::unique_ptr<ParameterDeclaration>> params, std::optional<std::string> ret,
                      std::optional<std::variant<std::unique_ptr<Block>, std::unique_ptr<Expression>>> body)
        : name(std::move(name)),
          parameters(std::move(params)),
          returnType(std::move(ret)),
          body(std::move(body)) {}

    void accept(Visitor& visitor) override;
};

class ConstructorDeclaration : public Declaration {
public:
    std::vector<std::unique_ptr<ParameterDeclaration>> parameters;
    std::unique_ptr<Block> body;

    ConstructorDeclaration(std::vector<std::unique_ptr<ParameterDeclaration>> params, std::unique_ptr<Block> body)
        : parameters(std::move(params)),
          body(std::move(body)) {}

    void accept(Visitor& visitor) override;
};

// |----------|
// |Statements|
// |----------|
/**
 *   x := 55
 *   ^^^^^^^
 */
class AssignmentStatement : public Statement {
public:
    std::string target;
    std::unique_ptr<Expression> value;

    AssignmentStatement(std::string t, std::unique_ptr<Expression> v)
        : target(std::move(t)),
          value(std::move(v)) {}

    void accept(Visitor& visitor) override;
};

class WhileStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> body;

    WhileStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Block> body)
        : condition(std::move(cond)),
          body(std::move(body)) {}

    void accept(Visitor& visitor) override;
};

class IfStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> trueBranch;
    std::unique_ptr<Block> falseBranch;

    IfStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Block> t, std::unique_ptr<Block> f)
        : condition(std::move(cond)),
          trueBranch(std::move(t)),
          falseBranch(std::move(f)) {}

    void accept(Visitor& visitor) override;
};

class ReturnStatement : public Statement {
public:
    std::unique_ptr<Expression> value;

    explicit ReturnStatement(std::unique_ptr<Expression> v)
        : value(std::move(v)) {}

    void accept(Visitor& visitor) override;
};

// |-----------|
// |Expressions|
// |-----------|
class LiteralExpression : public Expression {
public:
    enum class Type { Integer, Real, Boolean } type;
    std::variant<int64_t, double, bool> value;

    LiteralExpression(int64_t v)
        : type(Type::Integer),
          value(v) {}
    LiteralExpression(double v)
        : type(Type::Real),
          value(v) {}
    LiteralExpression(bool v)
        : type(Type::Boolean),
          value(v) {}

    void accept(Visitor& visitor) override;
};

class ThisExpression : public Expression {
public:
    void accept(Visitor& visitor) override;
};

class IdentifierExpression : public Expression {
public:
    std::string name;

    explicit IdentifierExpression(std::string n)
        : name(std::move(n)) {}
    void accept(Visitor& visitor) override;
};

/**
 * Array[Integer]
 * ^^^^^^^^^^^^^^
 */
class ParameterizedIdentifierExpression : public Expression {
public:
    std::string name;
    std::vector<std::string> typeArguments;

    ParameterizedIdentifierExpression(std::string n, std::vector<std::string> args)
        : name(std::move(n)),
          typeArguments(std::move(args)) {}
    void accept(Visitor& visitor) override;
};

/**
 * obj.field
 * ^^^^^^^^^
 */
class MemberExpression : public Expression {
public:
    std::unique_ptr<Expression> object;
    std::string member;

    MemberExpression(std::unique_ptr<Expression> obj, std::string mem)
        : object(std::move(obj)),
          member(std::move(mem)) {}
    void accept(Visitor& visitor) override;
};

class CallExpression : public Expression {
public:
    std::unique_ptr<Expression> callee;
    std::vector<std::unique_ptr<Expression>> arguments;

    CallExpression(std::unique_ptr<Expression> c, std::vector<std::unique_ptr<Expression>> args)
        : callee(std::move(c)),
          arguments(std::move(args)) {}
    void accept(Visitor& visitor) override;
};

class GroupingExpression : public Expression {
public:
    std::unique_ptr<Expression> inner;

    explicit GroupingExpression(std::unique_ptr<Expression> expr)
        : inner(std::move(expr)) {}
    void accept(Visitor& visitor) override;
};

} // namespace ast
