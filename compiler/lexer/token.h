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
  tok_identifier, // var/class/method names
  tok_int,        //integer literal
  tok_real,       //float literal
  tok_eof,        // EOF

  // keywords
  tok_kw_class,
  tok_kw_extends,
  tok_kw_is,
  tok_kw_var,
  tok_kw_method,
  tok_kw_if,
  tok_kw_then,
  tok_kw_else,
  tok_kw_while,
  tok_kw_loop,
  tok_kw_return,
  tok_kw_end,
  tok_kw_this,
  tok_kw_true,
  tok_kw_false
};

using token_value = std::variant<int, double, std::string>;

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
    case token_type::tok_identifier:
      return "tok_identifier";
    case token_type::tok_eof:
      return "tok_eof";
    case token_type::tok_kw_class:
      return "tok_kw_class";
    case token_type::tok_kw_extends: 
      return "tok_kw_extends";
    case token_type::tok_kw_is: 
      return "tok_kw_is";
    case token_type::tok_kw_var: 
      return "tok_kw_var";
    case token_type::tok_kw_method: 
      return "tok_kw_method";
    case token_type::tok_kw_if: 
      return "tok_kw_if";
    case token_type::tok_kw_then: 
      return "tok_kw_then";
    case token_type::tok_kw_else: 
      return "tok_kw_else";
    case token_type::tok_kw_while: 
      return "tok_kw_while";
    case token_type::tok_kw_loop: 
      return "tok_kw_loop";
    case token_type::tok_kw_return: 
      return "tok_kw_return";
    case token_type::tok_kw_end: 
      return "tok_kw_end";
    case token_type::tok_kw_this: 
      return "tok_kw_this";
    case token_type::tok_kw_true: 
      return "tok_kw_true";
    case token_type::tok_kw_false: 
      return "tok_kw_false";
  }

  return "tok_unknown";
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
      using ValueType = std::decay_t<decltype(val)>;
      
      if constexpr (std::is_same_v<ValueType, int>) {
        return std::format_to(ctx.out(), "{{type = {}, span = {}, value = \"{}\"}}\n", 
                              token_type, token.span, val);
      } else if constexpr (std::is_same_v<ValueType, double>) {
        return std::format_to(ctx.out(), "{{type = {}, span = {}, value = \"{}\"}}\n", 
                              token_type, token.span, val);
      } else if constexpr (std::is_same_v<ValueType, std::string>) {
        return std::format_to(ctx.out(), "{{type = {}, span = {}, value = \"{}\"}}\n", 
                              token_type, token.span, val);
      } else {
        return std::format_to(ctx.out(), "{{type = {}, span = {}, value = \"unknown_value\"}}\n", token_type, token.span);
      }
    }, token.value);
  }
};
