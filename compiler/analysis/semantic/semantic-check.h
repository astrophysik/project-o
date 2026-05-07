#pragma once

#include <memory>
#include <stdexcept>
#include <string_view>

#include "compiler/analysis/semantic/error.h"
#include "compiler/analysis/semantic/phases/class-body-collector.h"
#include "compiler/analysis/semantic/phases/class-collector.h"
#include "compiler/analysis/semantic/phases/class-field-checker.h"
#include "compiler/analysis/semantic/phases/class-method-checker.h"
#include "compiler/compilation-structures/ast/codegen/ast.h"
#include "compiler/compilation-structures/ast/parsing/ast.h"
#include "compiler/analysis/semantic/phases/codegen-ast-collector.h"

namespace analysis::semantic {

inline std::unique_ptr<codegen::ast::program> check_program(const std::unique_ptr<ast::program>& program,
                                                            std::string_view file_name,
                                                            std::string_view source) {
    error_formatter errors{file_name, source};
    auto [program_symbol_table, program_type_table] = phases::collect_program_classes(program, errors);
    phases::process_classes_content(program, *program_symbol_table, *program_type_table, errors);
    phases::check_field_content(program, *program_symbol_table, *program_type_table, errors);
    phases::check_method_content(program, *program_symbol_table, *program_type_table, errors);
    return phases::codegen_ast_collect(program, *program_symbol_table, *program_type_table);
}

} // namespace analysis::semantic
