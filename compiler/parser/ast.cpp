#include "ast.h"
#include "ast_visitor.h"

namespace ast {

void Program::accept(Visitor& v) { v.visit(*this); }
void Block::accept(Visitor& v) { v.visit(*this); }
void ClassDeclaration::accept(Visitor& v) { v.visit(*this); }
void VariableDeclaration::accept(Visitor& v) { v.visit(*this); }
void ParameterDeclaration::accept(Visitor& v) { v.visit(*this); }
void MethodDeclaration::accept(Visitor& v) { v.visit(*this); }
void ConstructorDeclaration::accept(Visitor& v) { v.visit(*this); }
void AssignmentStatement::accept(Visitor& v) { v.visit(*this); }
void WhileStatement::accept(Visitor& v) { v.visit(*this); }
void IfStatement::accept(Visitor& v) { v.visit(*this); }
void ReturnStatement::accept(Visitor& v) { v.visit(*this); }
void LiteralExpression::accept(Visitor& v) { v.visit(*this); }
void ThisExpression::accept(Visitor& v) { v.visit(*this); }
void IdentifierExpression::accept(Visitor& v) { v.visit(*this); }
void ParameterizedIdentifierExpression::accept(Visitor& v) { v.visit(*this); }
void MemberExpression::accept(Visitor& v) { v.visit(*this); }
void CallExpression::accept(Visitor& v) { v.visit(*this); }
void GroupingExpression::accept(Visitor& v) { v.visit(*this); }

}