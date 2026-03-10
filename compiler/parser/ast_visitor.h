#pragma once

#include "ast_fwd.h"

namespace ast {

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual void visit(Program& node) = 0;
    virtual void visit(Block& node) = 0;
    virtual void visit(ClassDeclaration& node) = 0;
    virtual void visit(VariableDeclaration& node) = 0;
    virtual void visit(ParameterDeclaration& node) = 0;
    virtual void visit(MethodDeclaration& node) = 0;
    virtual void visit(ConstructorDeclaration& node) = 0;
    virtual void visit(AssignmentStatement& node) = 0;
    virtual void visit(WhileStatement& node) = 0;
    virtual void visit(IfStatement& node) = 0;
    virtual void visit(ReturnStatement& node) = 0;
    virtual void visit(LiteralExpression& node) = 0;
    virtual void visit(ThisExpression& node) = 0;
    virtual void visit(IdentifierExpression& node) = 0;
    virtual void visit(ParameterizedIdentifierExpression& node) = 0;
    virtual void visit(MemberExpression& node) = 0;
    virtual void visit(CallExpression& node) = 0;
    virtual void visit(GroupingExpression& node) = 0;
};

}