#pragma once

#include <cstddef>
#include <format>

namespace common {

struct span {
    size_t line_num{};
    size_t start_pos{};
    size_t end_pos{};
};

} // namespace common

template<>
struct std::formatter<common::span> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const noexcept {
        return ctx.begin();
    }

    template<typename FmtContext>
    auto format(const common::span& span, FmtContext& ctx) const noexcept {
        return std::format_to(ctx.out(), "{}:{}-{}", span.line_num, span.start_pos, span.end_pos);
    }
};
