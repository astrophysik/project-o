#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "compiler/analysis/semantic/builtin-classes.h"
#include "compiler/compilation-structures/ast/codegen/ast.h"
#include "compiler/compilation-structures/ast/parsing/ast-visitor.h"
#include "compiler/compilation-structures/ast/parsing/ast.h"
#include "compiler/compilation-structures/symbol-table.h"
#include "compiler/compilation-structures/type-table.h"

namespace analysis::semantic::phases {

namespace details {

class codegen_ast_collector : public ast::visitor {
public:
    explicit codegen_ast_collector(structures::symbol_table& symbol_table, structures::type_table& type_table)
        : program_symbol_table(symbol_table),
          program_type_table(type_table) {}

    std::unique_ptr<codegen::ast::program> result_program;
    std::unique_ptr<codegen::ast::program> get_result() {
        return std::move(result_program);
    }

    void visit(ast::program& node) override;
    void visit(ast::block& node) override;
    void visit(ast::class_declaration& node) override;
    void visit(ast::variable_declaration& node) override;
    void visit(ast::parameter_declaration& node) override;
    void visit(ast::method_declaration& node) override;
    void visit(ast::constructor_declaration& node) override;
    void visit(ast::assignment_statement& node) override;
    void visit(ast::while_statement& node) override;
    void visit(ast::if_statement& node) override;
    void visit(ast::return_statement& node) override;
    void visit(ast::literal_expression& node) override;
    void visit(ast::this_expression& node) override;
    void visit(ast::identifier_expression& node) override;
    void visit(ast::parameterized_identifier_expression& node) override;
    void visit(ast::member_expression& node) override;
    void visit(ast::call_expression& node) override;
    void visit(ast::grouping_expression& node) override;

private:
    structures::symbol_table& program_symbol_table;
    structures::type_table& program_type_table;

    // Current context
    codegen::ast::class_declaration* current_class = nullptr;
    bool inside_function_scope = false;
    codegen::ast::method_declaration* current_method = nullptr;
    structures::symbol_table* current_scope = nullptr;

    // Mappings from parsing AST to codegen AST
    std::unordered_map<ast::class_declaration*, codegen::ast::class_declaration*> class_map;
    std::unordered_map<std::string, codegen::ast::variable_declaration*> variable_map;

    // Intermediate results for transformations
    std::unique_ptr<codegen::ast::expression> last_expression;
    std::unique_ptr<codegen::ast::statement> last_statement;
    std::unique_ptr<codegen::ast::block> last_block;
    std::unique_ptr<codegen::ast::member_expression> last_member_expr;

    // Helper methods
    codegen::ast::class_declaration* resolveType(const std::string& type_name);
    codegen::ast::class_declaration* resolveType(const structures::type* type);
    std::unique_ptr<codegen::ast::expression> transformExpression(ast::expression* expr);
    std::unique_ptr<codegen::ast::block> transformBlock(ast::block* block);
};

} // namespace details

inline std::unique_ptr<codegen::ast::program>
codegen_ast_collect(const std::unique_ptr<ast::program>& program, structures::symbol_table& symbol_table, structures::type_table& type_table) noexcept {
    details::codegen_ast_collector transformer(symbol_table, type_table);
    transformer.result_program = std::make_unique<codegen::ast::program>();
    builtin::add_builtin_classes_to_codegen(*transformer.result_program);
    program->accept(transformer);
    return transformer.get_result();
}

} // namespace analysis::semantic::phases
