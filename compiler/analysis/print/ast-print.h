#pragma once

#include <memory>

#include "compiler/analysis/print/details/ast-printer.h"
#include "compiler/compilation-structures/ast.h"

namespace analysis {

void print_program_ast(std::unique_ptr<ast::program>& program) noexcept {
    details::ast_printer printer;
    program->accept(printer);
}
} // namespace analysis
