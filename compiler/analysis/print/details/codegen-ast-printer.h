#pragma once

#include <iostream>
#include <print>

#include "compiler/compilation-structures/ast/codegen/ast-visitor.h"
#include "compiler/compilation-structures/ast/codegen/ast.h"

namespace analysis::details {

class codegen_ast_printer : public codegen::ast::visitor {
public:
    void visit(codegen::ast::program& node) override;
    void visit(codegen::ast::block& node) override;
    void visit(codegen::ast::class_declaration& node) override;
    void visit(codegen::ast::field_declaration& node) override;
    void visit(codegen::ast::variable_declaration& node) override;
    void visit(codegen::ast::parameter_declaration& node) override;
    void visit(codegen::ast::method_declaration& node) override;
    void visit(codegen::ast::constructor_declaration& node) override;
    void visit(codegen::ast::variable_assignment& node) override;
    void visit(codegen::ast::field_assignment& node) override;
    void visit(codegen::ast::while_statement& node) override;
    void visit(codegen::ast::if_statement& node) override;
    void visit(codegen::ast::return_statement& node) override;
    void visit(codegen::ast::literal_expression& node) override;
    void visit(codegen::ast::this_expression& node) override;
    void visit(codegen::ast::identifier_expression& node) override;
    void visit(codegen::ast::method_call_expression& node) override;
    void visit(codegen::ast::constructor_call_expression& node) override;
    void visit(codegen::ast::member_expression& node) override;
    void visit(codegen::ast::grouping_expression& node) override;

private:
    int indent = 0;

    void print_indent();
};

} // namespace analysis::details
