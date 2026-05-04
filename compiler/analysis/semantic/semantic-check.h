#pragma once

#include <memory>
#include <stdexcept>

#include "compiler/analysis/semantic/phases/class-body-collector.h"
#include "compiler/analysis/semantic/phases/class-collector.h"
#include "compiler/analysis/semantic/phases/class-field-checker.h"
#include "compiler/analysis/semantic/phases/class-method-checker.h"
#include "compiler/compilation-structures/ast/codegen/codegen-ast.h"
#include "compiler/compilation-structures/ast/parsing/ast.h"

namespace analysis::semantic {

void check_program(const std::unique_ptr<ast::program>& program) {
    auto [program_symbol_table, program_type_table] = phases::collect_program_classes(program);
    phases::process_classes_content(program, *program_symbol_table, *program_type_table);
    phases::check_field_content(program, *program_symbol_table, *program_type_table);
    phases::check_method_content(program, *program_symbol_table, *program_type_table);
}

} // namespace analysis::semantic
