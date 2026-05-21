#include "compiler/analysis/print/details/ast-printer.h"

#include "compiler/compilation-structures/ast/parsing/ast.h"

namespace analysis::details {
void ast_printer::print_indent() {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
}

void ast_printer::visit(ast::program& node) {
    std::cout << "Program:\n";
    indent++;
    for (auto& cls : node.classes) {
        print_indent();
        cls->accept(*this);
    }
    indent--;
}

void ast_printer::visit(ast::block& node) {
    std::cout << "Block:\n";
    indent++;
    for (auto& item : node.items) {
        print_indent();
        item->accept(*this);
    }
    indent--;
}

void ast_printer::visit(ast::class_declaration& node) {
    std::cout << "ClassDeclaration: name=" << node.name;
    if (node.base_class) {
        std::cout << ", extends=" << *node.base_class;
    }
    std::cout << "\n";
    indent++;
    for (auto& field : node.fields) {
        print_indent();
        field->accept(*this);
    }
    for (auto& method : node.methods) {
        print_indent();
        method->accept(*this);
    }
    for (auto& ctor : node.constructors) {
        print_indent();
        ctor->accept(*this);
    }
    indent--;
}

void ast_printer::visit(ast::variable_declaration& node) {
    std::cout << "VariableDeclaration: name=" << node.name << "\n";
    if (node.initializer) {
        indent++;
        print_indent();
        node.initializer->accept(*this);
        indent--;
    }
}

void ast_printer::visit(ast::parameter_declaration& node) {
    std::cout << "ParameterDeclaration: name=" << node.name << ", type=" << node.type_name << "\n";
}

void ast_printer::visit(ast::method_declaration& node) {
    std::cout << "MethodDeclaration: name=" << node.name;
    if (node.return_type) {
        std::cout << ", return_type=" << *node.return_type;
    }
    std::cout << "\n";
    indent++;
    print_indent();
    std::cout << "Parameters:\n";
    indent++;
    for (auto& param : node.parameters) {
        print_indent();
        param->accept(*this);
    }
    indent--;
    if (node.body) {
        print_indent();
        std::cout << "Body:\n";
        indent++;
        print_indent();
        std::visit([this](auto& body) { body->accept(*this); }, *node.body);
        indent--;
    }
    indent--;
}

void ast_printer::visit(ast::constructor_declaration& node) {
    std::cout << "ConstructorDeclaration:\n";
    indent++;
    print_indent();
    std::cout << "Parameters:\n";
    indent++;
    for (auto& param : node.parameters) {
        print_indent();
        param->accept(*this);
    }
    indent -= 2;
    if (node.body) {
        print_indent();
        std::cout << "Body:\n";
        indent++;
        print_indent();
        node.body->accept(*this);
        indent--;
    }
    indent--;
}

void ast_printer::visit(ast::assignment_statement& node) {
    std::cout << "AssignmentStatement: target=" << node.target << "\n";
    indent++;
    print_indent();
    node.value->accept(*this);
    indent--;
}

void ast_printer::visit(ast::while_statement& node) {
    std::cout << "WhileStatement:\n";
    indent++;
    print_indent();
    std::cout << "Condition:\n";
    indent++;
    print_indent();
    node.condition->accept(*this);
    indent--;
    print_indent();
    std::cout << "Body:\n";
    indent++;
    print_indent();
    node.body->accept(*this);
    indent -= 2;
    indent--;
}

void ast_printer::visit(ast::if_statement& node) {
    std::cout << "IfStatement:\n";
    indent++;
    print_indent();
    std::cout << "Condition:\n";
    indent++;
    print_indent();
    node.condition->accept(*this);
    indent--;
    print_indent();
    std::cout << "TrueBranch:\n";
    indent++;
    print_indent();
    node.true_branch->accept(*this);
    indent--;
    if (node.false_branch) {
        print_indent();
        std::cout << "FalseBranch:\n";
        indent++;
        print_indent();
        node.false_branch->accept(*this);
        indent--;
    }
    indent--;
}

void ast_printer::visit(ast::return_statement& node) {
    std::cout << "ReturnStatement:\n";
    if (node.value) {
        indent++;
        print_indent();
        node.value->accept(*this);
        indent--;
    }
}

void ast_printer::visit(ast::literal_expression& node) {
    std::cout << "LiteralExpression: ";
    switch (node.type) {
    case ast::literal_expression::type::integer:
        std::cout << "int=" << std::get<int64_t>(node.value) << "\n";
        break;
    case ast::literal_expression::type::real:
        std::cout << "real=" << std::get<double>(node.value) << "\n";
        break;
    case ast::literal_expression::type::boolean:
        std::cout << "bool=" << std::get<bool>(node.value) << "\n";
        break;
    }
}

void ast_printer::visit(ast::this_expression& node) {
    std::cout << "ThisExpression\n";
}

void ast_printer::visit(ast::identifier_expression& node) {
    std::cout << "IdentifierExpression: name=" << node.name << "\n";
}

void ast_printer::visit(ast::member_expression& node) {
    std::cout << "MemberExpression: member=" << node.member << "\n";
    indent++;
    print_indent();
    node.object->accept(*this);
    indent--;
}

void ast_printer::visit(ast::call_expression& node) {
    std::cout << "CallExpression:\n";
    indent++;
    print_indent();
    std::cout << "Callee:\n";
    indent++;
    print_indent();
    node.callee->accept(*this);
    indent--;
    print_indent();
    std::cout << "Arguments:\n";
    indent++;
    for (auto& arg : node.arguments) {
        print_indent();
        arg->accept(*this);
    }
    indent -= 2;
}

void ast_printer::visit(ast::grouping_expression& node) {
    std::cout << "GroupingExpression:\n";
    indent++;
    print_indent();
    node.inner->accept(*this);
    indent--;
}
} // namespace analysis::details
