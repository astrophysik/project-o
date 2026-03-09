#include "ast.h"
#include "ast_visitor.h"

namespace ast {

void Program::accept(Visitor& visitor) { visitor.visit(*this); }
void ClassDeclaration::accept(Visitor& visitor) { visitor.visit(*this); }
void VariableDeclaration::accept(Visitor& visitor) { visitor.visit(*this); }
void Parameter::accept(Visitor& visitor) { visitor.visit(*this); }
void MethodDeclaration::accept(Visitor& visitor) { visitor.visit(*this); }
void ConstructorDeclaration::accept(Visitor& visitor) { visitor.visit(*this); }
void Block::accept(Visitor& visitor) { visitor.visit(*this); }
void AssignmentStatement::accept(Visitor& visitor) { visitor.visit(*this); }
void WhileStatement::accept(Visitor& visitor) { visitor.visit(*this); }
void IfStatement::accept(Visitor& visitor) { visitor.visit(*this); }
void ReturnStatement::accept(Visitor& visitor) { visitor.visit(*this); }
void LiteralExpression::accept(Visitor& visitor) { visitor.visit(*this); }
void ThisExpression::accept(Visitor& visitor) { visitor.visit(*this); }
void IdentifierExpression::accept(Visitor& visitor) { visitor.visit(*this); }
void ParameterizedIdentifierExpression::accept(Visitor& visitor) { visitor.visit(*this); }
void MemberExpression::accept(Visitor& visitor) { visitor.visit(*this); }
void CallExpression::accept(Visitor& visitor) { visitor.visit(*this); }
void GroupingExpression::accept(Visitor& visitor) { visitor.visit(*this); }

} // namespace ast