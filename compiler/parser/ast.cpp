#include "ast.h"
#include "ast_visitor.h"

namespace ast {

void program::accept(visitor& v) { v.visit(*this); }
void block::accept(visitor& v) { v.visit(*this); }
void class_declaration::accept(visitor& v) { v.visit(*this); }
void variable_declaration::accept(visitor& v) { v.visit(*this); }
void parameter_declaration::accept(visitor& v) { v.visit(*this); }
void method_declaration::accept(visitor& v) { v.visit(*this); }
void constructor_declaration::accept(visitor& v) { v.visit(*this); }
void assignment_statement::accept(visitor& v) { v.visit(*this); }
void while_statement::accept(visitor& v) { v.visit(*this); }
void if_statement::accept(visitor& v) { v.visit(*this); }
void return_statement::accept(visitor& v) { v.visit(*this); }
void literal_expression::accept(visitor& v) { v.visit(*this); }
void this_expression::accept(visitor& v) { v.visit(*this); }
void identifier_expression::accept(visitor& v) { v.visit(*this); }
void parameterized_identifier_expression::accept(visitor& v) { v.visit(*this); }
void member_expression::accept(visitor& v) { v.visit(*this); }
void call_expression::accept(visitor& v) { v.visit(*this); }
void grouping_expression::accept(visitor& v) { v.visit(*this); }

} // namespace ast