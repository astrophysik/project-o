#include "compiler/analysis/print/details/ast-printer.h"

#include <print>

#include "compiler/ast/ast.h"

namespace analysis::details {
void ast_printer::print_indent() {
    for (int i = 0; i < indent; ++i) {
        std::print("  ");
    }
}

void ast_printer::visit(ast::program& node) {
    std::println("Program:");
    indent++;
    for (auto& cls : node.classes) {
        print_indent();
        cls->accept(*this);
    }
    indent--;
}

void ast_printer::visit(ast::block& node) {
    std::println("Block:");
    indent++;
    for (auto& item : node.items) {
        print_indent();
        item->accept(*this);
    }
    indent--;
}

void ast_printer::visit(ast::class_declaration& node) {
    std::print("ClassDeclaration: name={}", node.name);
    if (node.base_class) {
        std::print(", extends={}", *node.base_class);
    }
    std::println("");
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
    std::println("VariableDeclaration: name={}", node.name);
    if (node.initializer) {
        indent++;
        print_indent();
        node.initializer->accept(*this);
        indent--;
    }
}

void ast_printer::visit(ast::parameter_declaration& node) {
    std::println("ParameterDeclaration: name={}, type={}", node.name, node.type_name);
}

void ast_printer::visit(ast::method_declaration& node) {
    std::print("MethodDeclaration: name={}", node.name);
    if (node.return_type) {
        std::print(", return_type={}", *node.return_type);
    }
    std::println("");
    indent++;
    print_indent();
    std::println("Parameters:");
    indent++;
    for (auto& param : node.parameters) {
        print_indent();
        param->accept(*this);
    }
    indent--;
    if (node.body) {
        print_indent();
        std::println("Body:");
        indent++;
        print_indent();
        std::visit([this](auto& body) { body->accept(*this); }, *node.body);
        indent--;
    }
    indent--;
}

void ast_printer::visit(ast::constructor_declaration& node) {
    std::println("ConstructorDeclaration:");
    indent++;
    print_indent();
    std::println("Parameters:");
    indent++;
    for (auto& param : node.parameters) {
        print_indent();
        param->accept(*this);
    }
    indent -= 2;
    if (node.body) {
        print_indent();
        std::println("Body:");
        indent++;
        print_indent();
        node.body->accept(*this);
        indent--;
    }
    indent--;
}

void ast_printer::visit(ast::assignment_statement& node) {
    std::println("AssignmentStatement: target={}", node.target);
    indent++;
    print_indent();
    node.value->accept(*this);
    indent--;
}

void ast_printer::visit(ast::while_statement& node) {
    std::println("WhileStatement:");
    indent++;
    print_indent();
    std::println("Condition:");
    indent++;
    print_indent();
    node.condition->accept(*this);
    indent--;
    print_indent();
    std::println("Body:");
    indent++;
    print_indent();
    node.body->accept(*this);
    indent -= 2;
    indent--;
}

void ast_printer::visit(ast::if_statement& node) {
    std::println("IfStatement:");
    indent++;
    print_indent();
    std::println("Condition:");
    indent++;
    print_indent();
    node.condition->accept(*this);
    indent--;
    print_indent();
    std::println("TrueBranch:");
    indent++;
    print_indent();
    node.true_branch->accept(*this);
    indent--;
    if (node.false_branch) {
        print_indent();
        std::println("FalseBranch:");
        indent++;
        print_indent();
        node.false_branch->accept(*this);
        indent--;
    }
    indent--;
}

void ast_printer::visit(ast::return_statement& node) {
    std::println("ReturnStatement:");
    if (node.value) {
        indent++;
        print_indent();
        node.value->accept(*this);
        indent--;
    }
}

void ast_printer::visit(ast::literal_expression& node) {
    std::print("LiteralExpression: ");
    switch (node.type) {
    case ast::literal_expression::type::integer:
        std::println("int={}", std::get<int64_t>(node.value));
        break;
    case ast::literal_expression::type::real:
        std::println("real={}", std::get<double>(node.value));
        break;
    case ast::literal_expression::type::boolean:
        std::println("bool={}", std::get<bool>(node.value));
        break;
    }
}

void ast_printer::visit(ast::this_expression& node) {
    std::println("ThisExpression");
}

void ast_printer::visit(ast::identifier_expression& node) {
    std::println("IdentifierExpression: name={}", node.name);
}

void ast_printer::visit(ast::parameterized_identifier_expression& node) {
    std::print("ParameterizedIdentifierExpression: name={}", node.name);
    std::print(", type_args=[");
    for (size_t i = 0; i < node.type_arguments.size(); ++i) {
        if (i > 0)
            std::print(", ");
        std::print("{}", node.type_arguments[i]);
    }
    std::println("]");
}

void ast_printer::visit(ast::member_expression& node) {
    std::println("MemberExpression: member={}", node.member);
    indent++;
    print_indent();
    node.object->accept(*this);
    indent--;
}

void ast_printer::visit(ast::call_expression& node) {
    std::println("CallExpression:");
    indent++;
    print_indent();
    std::println("Callee:");
    indent++;
    print_indent();
    node.callee->accept(*this);
    indent--;
    print_indent();
    std::println("Arguments:");
    indent++;
    for (auto& arg : node.arguments) {
        print_indent();
        arg->accept(*this);
    }
    indent -= 2;
}

void ast_printer::visit(ast::grouping_expression& node) {
    std::println("GroupingExpression:");
    indent++;
    print_indent();
    node.inner->accept(*this);
    indent--;
}
} // namespace analysis::details
