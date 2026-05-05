#include "compiler/analysis/print/details/codegen-ast-printer.h"

#include <print>

#include "compiler/common/variant-helper.h"

namespace analysis::details {

void codegen_ast_printer::print_indent() {
    for (int i = 0; i < indent; ++i) {
        std::print("  ");
    }
}

void codegen_ast_printer::visit(codegen::ast::program& node) {
    std::println("Program:");
    indent++;
    if (!node.internal_classes.empty()) {
        print_indent();
        std::println("InternalClasses:");
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
    std::println("Block:");
    indent++;
    for (auto& item : node.items) {
        print_indent();
        item->accept(*this);
    }
    indent--;
}

void codegen_ast_printer::visit(codegen::ast::class_declaration& node) {
    std::print("ClassDeclaration: name={}", node.name);
    if (node.base_class) {
        std::print(", extends={}", node.base_class->name);
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

void codegen_ast_printer::visit(codegen::ast::field_declaration& node) {
    std::print("FieldDeclaration: name={}", node.name);
    if (node.type) {
        std::print(", type={}", node.type->name);
    }
    std::println("");
    if (node.initializer) {
        indent++;
        print_indent();
        node.initializer->accept(*this);
        indent--;
    }
}

void codegen_ast_printer::visit(codegen::ast::variable_declaration& node) {
    std::print("VariableDeclaration: name={}", node.name);
    if (node.type) {
        std::print(", type={}", node.type->name);
    }
    std::println("");
    if (node.initializer) {
        indent++;
        print_indent();
        node.initializer->accept(*this);
        indent--;
    }
}

void codegen_ast_printer::visit(codegen::ast::parameter_declaration& node) {
    std::print("ParameterDeclaration: name={}", node.name);
    if (node.type) {
        std::print(", type={}", node.type->name);
    }
    std::println("");
}

void codegen_ast_printer::visit(codegen::ast::method_declaration& node) {
    std::print("MethodDeclaration: name={}", node.name);
    if (node.return_type) {
        std::print(", return_type={}", node.return_type->name);
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

void codegen_ast_printer::visit(codegen::ast::constructor_declaration& node) {
    std::println("ConstructorDeclaration:");
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
        node.body->accept(*this);
        indent--;
    }
    indent--;
}

void codegen_ast_printer::visit(codegen::ast::variable_assignment& node) {
    std::visit(overloaded{[](codegen::ast::variable_declaration* decl) { std::println("VariableAssignment: target={}", decl->name); },
                          [](codegen::ast::parameter_declaration* decl) { std::println("VariableAssignment: target={}", decl->name); }},
               node.target);
    indent++;
    print_indent();
    node.value->accept(*this);
    indent--;
}

void codegen_ast_printer::visit(codegen::ast::field_assignment& node) {
    std::println("FieldAssignment: target={}", node.target->member->name);
    indent++;
    print_indent();
    node.value->accept(*this);
    indent--;
}

void codegen_ast_printer::visit(codegen::ast::while_statement& node) {
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
}

void codegen_ast_printer::visit(codegen::ast::if_statement& node) {
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

void codegen_ast_printer::visit(codegen::ast::return_statement& node) {
    std::println("ReturnStatement:");
    if (node.value) {
        indent++;
        print_indent();
        node.value->accept(*this);
        indent--;
    }
}

void codegen_ast_printer::visit(codegen::ast::literal_expression& node) {
    std::print("LiteralExpression: ");
    std::visit([](auto&& val) { std::println("{}", val); }, node.value);
}

void codegen_ast_printer::visit(codegen::ast::this_expression& node) {
    std::print("ThisExpression");
    if (node.type) {
        std::print(": type={}", node.type->name);
    }
    std::println("");
}

void codegen_ast_printer::visit(codegen::ast::identifier_expression& node) {
    std::visit(overloaded{[](codegen::ast::variable_declaration* decl) { std::println("IdentifierExpression: variable name={}", decl->name); },
                          [](codegen::ast::parameter_declaration* decl) { std::println("IdentifierExpression: parameter target={}", decl->name); }},
               node.target);
}

void codegen_ast_printer::visit(codegen::ast::method_call_expression& node) {
    std::print("MethodCallExpression: method={}", node.method->name);
    if (node.return_type) {
        std::print(", return_type={}", node.return_type->name);
    }
    std::println("");
    indent++;
    print_indent();
    std::println("Object:");
    indent++;
    print_indent();
    node.object->accept(*this);
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

void codegen_ast_printer::visit(codegen::ast::constructor_call_expression& node) {
    std::println("ConstructorCallExpression:");
    indent++;
    print_indent();
    std::println("Class: {}", node.constructor->class_owner->name);
    print_indent();
    std::println("Arguments:");
    indent++;
    for (auto& arg : node.arguments) {
        print_indent();
        arg->accept(*this);
    }
    indent -= 2;
}

void codegen_ast_printer::visit(codegen::ast::member_expression& node) {
    std::println("MemberExpression: member={}", node.member->name);
    indent++;
    print_indent();
    node.object->accept(*this);
    indent--;
}

void codegen_ast_printer::visit(codegen::ast::grouping_expression& node) {
    std::println("GroupingExpression:");
    indent++;
    print_indent();
    node.inner->accept(*this);
    indent--;
}

} // namespace analysis::details
