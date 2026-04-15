#pragma once

#include <memory>

#include "compiler/analysis/semantic/phases/class-body-collector.h"
#include "compiler/analysis/semantic/phases/class-collector.h"
#include "compiler/analysis/semantic/phases/class-content-checker.h"
#include "compiler/ast/ast.h"

namespace analysis::semantic {

void check_program(const std::unique_ptr<ast::program>& program) {
    auto program_symbol_table_res = phases::collect_program_classes(program);
    if (!program_symbol_table_res.has_value()) {
        throw std::runtime_error{program_symbol_table_res.error()};
    }

    auto program_symbol_table = std::move(*program_symbol_table_res);

    if (auto res = phases::process_classes_content(program, *program_symbol_table); !res.has_value()) {
        throw std::runtime_error{res.error()};
    }

    if (auto res = phases::check_classes_content(program, *program_symbol_table); !res.has_value()) {
        throw std::runtime_error{res.error()};
    }
}

} // namespace analysis::semantic
