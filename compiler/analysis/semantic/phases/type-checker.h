//#pragma once
//
//#include <expected>
//#include <format>
//
//#include "compiler/analysis/semantic/symbol-table.h"
//#include "compiler/ast/ast-visitor.h"
//#include "compiler/ast/ast.h"
//
//namespace analysis::details {
//
//class type_checker {
//public:
//    explicit type_checker(symbol_table& current_scope)
//        : current_scope(current_scope) {}
//
//    std::expected<type, std::string> type_infer(ast::expression* expr) {
//        if (auto* literal_expr = dynamic_cast<ast::literal_expression*>(expr)) {
//            switch (literal_expr->type) {
//            case ast::literal_expression::type::real:
//                return "real";
//            case ast::literal_expression::type::integer:
//                return "integer";
//            case ast::literal_expression::type::boolean:
//                return "boolean";
//            }
//        } else if (auto* identifier_expr = dynamic_cast<ast::identifier_expression*>(expr)) {
//            auto* sym = current_scope.lookup(identifier_expr->name);
//            if (auto* sym_var = dynamic_cast<variable_symbol*>(sym)) {
//                return sym_var->type_name;
//            } else if (auto* sym_method = dynamic_cast<method_symbol*>(sym)) {
//                return std::unexpected{std::format("infer type of method is unsupported {}", sym_method->name)};
//            } else if (auto* sym_class = dynamic_cast<class_symbol*>(sym)) {
//                return std::unexpected{std::format("infer type of class is unsupported {}", sym_class->name)};
//            }
//        } else if (auto* member_expr = dynamic_cast<ast::member_expression*>(expr)) {
//            auto object_type{type_infer(member_expr->object.get())};
//
//        }
//    }
//
//    bool type_check(ast::expression* expr, std::string type);
//
//private:
//    const symbol_table& current_scope;
//};
//
//} // namespace analysis::details
