#pragma once

#include <memory>

#include "compiler/analysis/print/details/codegen-ast-printer.h"
#include "compiler/compilation-structures/ast/codegen/ast.h"

namespace analysis {

void print_codegen_ast(std::unique_ptr<codegen::ast::program>& program) noexcept {
    details::codegen_ast_printer printer;
    program->accept(printer);
}

} // namespace analysis
