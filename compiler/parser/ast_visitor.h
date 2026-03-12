#pragma once

#include "ast_fwd.h"

namespace ast {

class visitor {
public:
    virtual ~visitor() = default;

    virtual void visit(program& node) = 0;
    virtual void visit(block& node) = 0;
    virtual void visit(class_declaration& node) = 0;
    virtual void visit(variable_declaration& node) = 0;
    virtual void visit(parameter_declaration& node) = 0;
    virtual void visit(method_declaration& node) = 0;
    virtual void visit(constructor_declaration& node) = 0;
    virtual void visit(assignment_statement& node) = 0;
    virtual void visit(while_statement& node) = 0;
    virtual void visit(if_statement& node) = 0;
    virtual void visit(return_statement& node) = 0;
    virtual void visit(literal_expression& node) = 0;
    virtual void visit(this_expression& node) = 0;
    virtual void visit(identifier_expression& node) = 0;
    virtual void visit(parameterized_identifier_expression& node) = 0;
    virtual void visit(member_expression& node) = 0;
    virtual void visit(call_expression& node) = 0;
    virtual void visit(grouping_expression& node) = 0;
};

} // namespace ast
