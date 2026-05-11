#include "compiler/analysis/semantic/error.h"

namespace analysis::semantic {

error_formatter::error_formatter(std::string_view file_name, std::string_view source)
    : file_name_(file_name), source_(source) {}

std::string_view error_formatter::get_line(size_t line_num) const {
    if (line_num == 0) {
        return {};
    }
    size_t current_line = 1;
    size_t pos = 0;
    while (current_line < line_num && pos < source_.size()) {
        if (source_[pos] == '\n') {
            ++current_line;
        }
        ++pos;
    }
    if (current_line != line_num || pos > source_.size()) {
        return {};
    }
    size_t end = pos;
    while (end < source_.size() && source_[end] != '\n') {
        ++end;
    }
    return source_.substr(pos, end - pos);
}

std::string error_formatter::format_with_location(common::span span, std::string_view description) const {
    static constexpr std::string_view indent = "    ";

    std::string result;
    result += std::format("{}:{}:{}: error: {}\n",
                          file_name_.empty() ? std::string_view{"<source_file>"} : file_name_,
                          span.line_num,
                          span.start_pos,
                          description);

    auto line = get_line(span.line_num);
    if (line.empty() && span.line_num == 0) {
        return result;
    }

    result += indent;
    result += line;
    result += '\n';

    result += indent;
    size_t caret_start = span.start_pos > 0 ? span.start_pos - 1 : 0;
    for (size_t i = 0; i < caret_start; ++i) {
        char c = i < line.size() ? line[i] : ' ';
        result += (c == '\t') ? '\t' : ' ';
    }
    size_t caret_count = span.end_pos > span.start_pos ? span.end_pos - span.start_pos : 1;
    for (size_t i = 0; i < caret_count; ++i) {
        result += '^';
    }
    result += '\n';

    return result;
}

} // namespace analysis::semantic
