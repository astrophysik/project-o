#pragma once

#include <format>
#include <string>
#include <string_view>
#include <utility>

#include "compiler/compilation-structures/common.h"

namespace analysis::semantic {

class error_formatter {
public:
    error_formatter(std::string_view file_name, std::string_view source);

    template<typename... Args>
    std::string format_error(common::span span, std::format_string<Args...> fmt, Args&&... args) const {
        return format_with_location(span, std::format(fmt, std::forward<Args>(args)...));
    }

private:
    std::string_view file_name_;
    std::string_view source_;

    std::string format_with_location(common::span span, std::string_view description) const;
    std::string_view get_line(size_t line_num) const;
};

} // namespace analysis::semantic
