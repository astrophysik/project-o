#pragma once
#include <expected>
#include <string>
#include "compiler/analysis/semantic/symbol-table.h"
#include "compiler/ast/ast.h"

namespace analysis::semantic {

class type_inferrer {
public:
    type_inferrer(symbol_table& program_table,
                  const std::string& current_class_name)
        : program_table_(program_table),
          current_class_name_(current_class_name) {}

    std::expected<type, std::string> infer(ast::expression* expr,
                                           symbol_table& scope);

private:
    symbol_table&      program_table_;
    const std::string& current_class_name_;  // ссылка — обновляется автоматически
};

} // namespace analysis::semantic