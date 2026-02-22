#pragma once

#include <cstddef>
#include <format>
#include <string>

namespace lexer {

struct span {
  size_t line_num{};
  size_t start_pos{};
  size_t end_pos{};
};

enum class token_type {

  tok_assignment, // :=
  tok_fat_arrow, // =>

  tok_new_line, // \n
  tok_colon, // :
  tok_dot, // .
  tok_comma, // ,

  tok_open_par, // (
  tok_close_par, // )

  tok_end
};

struct token {
  token_type type;
  span span;
  std::string value;
};

namespace impl_ {
inline constexpr std::string_view
token_type_to_string(token_type type) noexcept {
  switch (type) {
  case token_type::tok_assignment:
    return "tok_assignment";
  case token_type::tok_fat_arrow:
    return "tok_fat_arrow";

  case token_type::tok_colon:
    return "tok_colon";
  case token_type::tok_dot:
    return "tok_dot";
  case token_type::tok_comma:
    return "tok_comma";

  case token_type::tok_open_par:
    return "tok_oppar";
  case token_type::tok_close_par:
    return "tok_clpar";

  case token_type::tok_end:
    return "tok_end";

  case token_type::tok_new_line:
    return "tok_end";
  }
  return "tok_unknown";
}

} // namespace impl_

} // namespace lexer

template <> struct std::formatter<lexer::span> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx) const noexcept {
    return ctx.begin();
  }

  template <typename FmtContext>
  auto format(const lexer::span &span, FmtContext &ctx) const noexcept {
    return std::format_to(ctx.out(), "{}:{}-{}", span.line_num, span.start_pos,
                          span.end_pos);
  }
};

template <> struct std::formatter<lexer::token> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx) const noexcept {
    return ctx.begin();
  }

  template <typename FmtContext>
  auto format(const lexer::token &token, FmtContext &ctx) const noexcept {
    return std::format_to(ctx.out(), "type = {}, span = {}, value = \"{}\"",
                          lexer::impl_::token_type_to_string(token.type),
                          token.span, token.value);
  }
};
