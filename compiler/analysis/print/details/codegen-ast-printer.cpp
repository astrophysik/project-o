#include "compiler/analysis/print/details/codegen-ast-printer.h"

#include "compiler/common/variant-helper.h"

namespace analysis::details {

void codegen_ast_printer::print_indent() {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
}

void codegen_ast_printer::visit(codegen::ast::program& node) {
    std::cout << "Program:\n";
    indent++;
    if (!node.internal_classes.empty()) {
        print_indent();
        std::cout << "InternalClasses:\n";
        indent++;
        for (auto& cls : node.internal_classes) {
            print_indent();
            cls->accept(*this);
        }
        indent--;
    }
    for (auto& cls : node.classes) {
        print_indent();
        cls->accept(*this);
    }
    indent--;
}

void codegen_ast_printer::visit(codegen::ast::block& node) {
    std::cout << "Block:\n";
    indent++;
    for (auto& item : node.items) {
        print_indent();
        item->accept(*this);
    }
    indent--;
}

void codegen_ast_printer::visit(codegen::ast::class_declaration& node) {
    std::cout << "ClassDeclaration: name=" << node.name;
    if (node.base_class) {
        std::cout << ", extends=" << node.base_class->name;
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

void codegen_ast_printer::visit(codegen::ast::field_declaration& node) {
    std::cout << "FieldDeclaration: name=" << node.name;
    if (node.type) {
        std::cout << ", type=" << node.type->name;
    }
    std::cout << "\n";
    if (node.initializer) {
        indent++;
        print_indent();
        node.initializer->accept(*this);
        indent--;
    }
}

void codegen_ast_printer::visit(codegen::ast::variable_declaration& node) {
    std::cout << "VariableDeclaration: name=" << node.name;
    if (node.type) {
        std::cout << ", type=" << node.type->name;
    }
    std::cout << "\n";
    if (node.initializer) {
        indent++;
        print_indent();
        node.initializer->accept(*this);
        indent--;
    }
}

void codegen_ast_printer::visit(codegen::ast::parameter_declaration& node) {
    std::cout << "ParameterDeclaration: name=" << node.name;
    if (node.type) {
        std::cout << ", type=" << node.type->name;
    }
    std::cout << "\n";
}

void codegen_ast_printer::visit(codegen::ast::method_declaration& node) {
    std::cout << "MethodDeclaration: name=" << node.name;
    if (node.return_type) {
        std::cout << ", return_type=" << node.return_type->name;
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

void codegen_ast_printer::visit(codegen::ast::constructor_declaration& node) {
    std::cout << "ConstructorDeclaration:\n";
    indent++;
    print_indent();
    std::cout << "Parameters:\n";
    indent++;
    for (auto& param : node.parameters) {
        print_indent();
        param->accept(*this);
    }
    indent--;
    if (node.super_constructor) {
        print_indent();
        std::cout << "Super constructor call:\n";
        indent++;
        print_indent();
        node.super_constructor->accept(*this);
        indent--;
    }
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

void codegen_ast_printer::visit(codegen::ast::variable_assignment& node) {
    std::visit(overloaded{[](codegen::ast::variable_declaration* decl) { std::cout << "VariableAssignment: target=" << decl->name << "\n"; },
                          [](codegen::ast::parameter_declaration* decl) { std::cout << "VariableAssignment: target=" << decl->name << "\n"; },
                          [](codegen::ast::field_declaration* decl) { std::cout << "VariableAssignment: target=" << decl->name << "\n"; }},
               node.target);
    indent++;
    print_indent();
    node.value->accept(*this);
    indent--;
}

void codegen_ast_printer::visit(codegen::ast::field_assignment& node) {
    std::cout << "FieldAssignment: target=" << node.target->member->name << "\n";
    indent++;
    print_indent();
    node.value->accept(*this);
    indent--;
}

void codegen_ast_printer::visit(codegen::ast::while_statement& node) {
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
}

void codegen_ast_printer::visit(codegen::ast::if_statement& node) {
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

void codegen_ast_printer::visit(codegen::ast::return_statement& node) {
    std::cout << "ReturnStatement:\n";
    if (node.value) {
        indent++;
        print_indent();
        node.value->accept(*this);
        indent--;
    }
}

void codegen_ast_printer::visit(codegen::ast::literal_expression& node) {
    std::cout << "LiteralExpression: ";
    std::visit([](auto&& val) { std::cout << val << "\n"; }, node.value);
}

void codegen_ast_printer::visit(codegen::ast::this_expression& node) {
    std::cout << "ThisExpression";
    if (node.type) {
        std::cout << ": type=" << node.type->name;
    }
    std::cout << "\n";
}

void codegen_ast_printer::visit(codegen::ast::identifier_expression& node) {
    std::visit(overloaded{[](codegen::ast::variable_declaration* decl) { std::cout << "IdentifierExpression: variable name=" << decl->name << "\n"; },
                          [](codegen::ast::parameter_declaration* decl) { std::cout << "IdentifierExpression: parameter target=" << decl->name << "\n"; },
                          [](codegen::ast::field_declaration* decl) { std::cout << "IdentifierExpression: field target=" << decl->name << "\n"; }},
               node.target);
}

void codegen_ast_printer::visit(codegen::ast::method_call_expression& node) {
    std::cout << "MethodCallExpression: method=" << node.method->name;
    if (node.return_type) {
        std::cout << ", return_type=" << node.return_type->name;
    }
    std::cout << "\n";
    indent++;
    print_indent();
    std::cout << "Object:\n";
    indent++;
    print_indent();
    node.object->accept(*this);
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

void codegen_ast_printer::visit(codegen::ast::constructor_call_expression& node) {
    std::cout << "ConstructorCallExpression:\n";
    indent++;
    print_indent();
    std::cout << "Class: " << node.constructor->class_owner->name << "\n";
    print_indent();
    std::cout << "Arguments:\n";
    indent++;
    for (auto& arg : node.arguments) {
        print_indent();
        arg->accept(*this);
    }
    indent -= 2;
}

void codegen_ast_printer::visit(codegen::ast::member_expression& node) {
    std::cout << "MemberExpression: member=" << node.member->name << "\n";
    indent++;
    print_indent();
    node.object->accept(*this);
    indent--;
}

void codegen_ast_printer::visit(codegen::ast::grouping_expression& node) {
    std::cout << "GroupingExpression:\n";
    indent++;
    print_indent();
    node.inner->accept(*this);
    indent--;
}

} // namespace analysis::details
