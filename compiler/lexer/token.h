#pragma once

#include <cstddef>
#include <format>
#include <variant>
#include <string>

namespace lexer {

struct span {
  size_t line_num{};
  size_t start_pos{};
  size_t end_pos{};
};

enum class token_type : uint8_t {
  tok_assignment, // :=
  tok_fat_arrow,  // =>
  tok_new_line,   // \n
  tok_colon,      // :
  tok_dot,        // .
  tok_comma,      // ,
  tok_open_par,   // (
  tok_close_par,  // )
  tok_keyword,    // class, extends, is, var, method, if, then, else, while, loop, return, end, this, true, false
  tok_identifier, // var/class/method names
  tok_eof,        // EOF
  tok_unknown
};

struct assignment_value {};
struct fat_arrow_value {};
struct new_line_value {};
struct colon_value {};
struct dot_value {};
struct comma_value {};
struct open_par_value {};
struct close_par_value {};
struct keyword_value {
  enum class type : uint8_t {
    kw_class,
    kw_extends,
    kw_is,
    kw_var,
    kw_method,
    kw_if,
    kw_then,
    kw_else,
    kw_while,
    kw_loop,
    kw_return,
    kw_end,
    kw_this,
    kw_true,
    kw_false
  } kind;
};
struct identifier_value {
  std::string name;
};
struct eof_value {};
struct unknown_value {};

using token_value = std::variant<
  assignment_value,
  fat_arrow_value,
  new_line_value,
  colon_value,
  dot_value,
  comma_value,
  open_par_value,
  close_par_value,
  keyword_value,
  identifier_value,
  eof_value,
  unknown_value
>;

struct token {
  token_type type;
  span span;
  token_value value;

  template<typename T>
  bool is() const noexcept {
    return std::holds_alternative<T>(value);
  }

  template<typename T>
  const T& as() const noexcept {
    return std::get<T>(value);
  }
};

namespace impl_ {
inline constexpr std::string_view token_type_to_string(token_type type) noexcept {
  switch (type) {
  case token_type::tok_assignment:
    return "tok_assignment";
  case token_type::tok_fat_arrow:
    return "tok_fat_arrow";
  case token_type::tok_new_line:
    return "tok_new_line";
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
  case token_type::tok_keyword:
    return "tok_keyword";
  case token_type::tok_identifier:
    return "tok_identifier";
  case token_type::tok_eof:
    return "tok_eof";
  case token_type::tok_unknown:
    return "tok_unknown";
  }
}

inline std::string_view keyword_to_string(keyword_value::type kw) noexcept {
  switch (kw) {
    case keyword_value::type::kw_class:
      return "class";
    case keyword_value::type::kw_extends: 
      return "extends";
    case keyword_value::type::kw_is: 
      return "is";
    case keyword_value::type::kw_var: 
      return "var";
    case keyword_value::type::kw_method: 
      return "method";
    case keyword_value::type::kw_if: 
      return "if";
    case keyword_value::type::kw_then: 
      return "then";
    case keyword_value::type::kw_else: 
      return "else";
    case keyword_value::type::kw_while: 
      return "while";
    case keyword_value::type::kw_loop: 
      return "loop";
    case keyword_value::type::kw_return: 
      return "return";
    case keyword_value::type::kw_end: 
      return "end";
    case keyword_value::type::kw_this: 
      return "this";
    case keyword_value::type::kw_true: 
      return "true";
    case keyword_value::type::kw_false: 
      return "false";
  }

  return "unknown";
}

} // namespace impl_

} // namespace lexer

template<>
struct std::formatter<lexer::span> {
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx) const noexcept {
    return ctx.begin();
  }

  template<typename FmtContext>
  auto format(const lexer::span& span, FmtContext& ctx) const noexcept {
    return std::format_to(ctx.out(), "{}:{}-{}", span.line_num, span.start_pos, span.end_pos);
  }
};

template<>
struct std::formatter<lexer::token> {
  template<typename ParseContext>
  constexpr auto parse(ParseContext& ctx) const noexcept {
    return ctx.begin();
  }

  template<typename FmtContext>
  auto format(const lexer::token& token, FmtContext& ctx) const noexcept {
    auto token_type = lexer::impl_::token_type_to_string(token.type);
    
    return std::visit([&](const auto& val) -> decltype(auto) {
      using T = std::decay_t<decltype(val)>;
      
      if constexpr (std::is_same_v<T, lexer::keyword_value>) {
        return std::format_to(ctx.out(), "{{type = {}, span = {}, value = \"{}\"}}\n", 
                              token_type, token.span, lexer::impl_::keyword_to_string(val.kind));
      } else if constexpr (std::is_same_v<T, lexer::identifier_value>) {
        return std::format_to(ctx.out(), "{{type = {}, span = {}, value = \"{}\"}}\n", 
                              token_type, token.span, val.name);
      } else {
        return std::format_to(ctx.out(), "{{type = {}, span = {}}}\n", token_type, token.span);
      }
    }, token.value);
  }
};
