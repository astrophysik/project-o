#pragma once

#include <memory>
#include <stdexcept>

#include "compiler/analysis/semantic/phases/class-body-collector.h"
#include "compiler/analysis/semantic/phases/class-collector.h"
#include "compiler/analysis/semantic/phases/class-content-checker.h"
#include "compiler/compilation-structures/ast.h"

namespace analysis::semantic {

void check_program(const std::unique_ptr<ast::program>& program) {
    auto [program_symbol_table, program_type_table] = phases::collect_program_classes(program);
    phases::process_classes_content(program, *program_symbol_table, *program_type_table);
    phases::check_classes_content(program, *program_symbol_table, *program_type_table);
}

} // namespace analysis::semantic
