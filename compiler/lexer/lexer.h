#pragma once

#include <cctype>
#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "compiler/lexer/token.h"

namespace lexer {

namespace impl_ {

struct lexeme_parser {

  lexeme_parser(std::string_view text) noexcept
      : text(text),
        line_num{},
        column_num{} {}

  std::expected<std::optional<token>, std::string> take_next_token() noexcept {
    skip_whitespace();
    skip_comment();

    auto symbol_opt{take_next_symbol()};

    if (!symbol_opt.has_value()) {
      return std::nullopt;
    }

    char symbol{*symbol_opt};

    switch (symbol) {
    case '(':
      return token{.type = token_type::tok_open_par, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = "("};
      break;
    case ')':
      return token{.type = token_type::tok_close_par, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = ")"};
      break;
    case ':':
      if (auto next_symbol{peek_next_symbol().value_or(' ')}; next_symbol == '=') {
        std::ignore = take_next_symbol();
        return token{.type = token_type::tok_assignment, .span{.line_num = line_num, .start_pos = column_num - 1, .end_pos = column_num + 1}, .value = ":="};
      } else {
        return token{.type = token_type::tok_colon, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = ":"};
      }
      break;
    case '.':
      return token{.type = token_type::tok_dot, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = "."};
      break;
    case ',':
      return token{.type = token_type::tok_comma, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = ","};
      break;
    case '=':
      if (auto next_symbol{peek_next_symbol().value_or(' ')}; next_symbol == '>') {
        std::ignore = take_next_symbol();
        return token{.type = token_type::tok_fat_arrow, .span{.line_num = line_num, .start_pos = column_num - 1, .end_pos = column_num + 1}, .value = "=>"};
      } else {
        return std::unexpected{std::format("unknown token \'{}\' at line : {}, column : {}", symbol, line_num, column_num)};
      }
      break;
    case '\n':
      return token{.type = token_type::tok_new_line, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = "\\n"};
      break;
      // todo:
      // - parse tok_int, tok_real (int.int) , tok_bool
      // - parse name => after check is name is keyword and which by swich
      // case for keywords
      //
    default:
      return std::unexpected{std::format("unknown token \'{}\' at line : {}, column : {}", symbol, line_num, column_num)};
    }

    return std::nullopt;
  }

private:
  void skip_whitespace() noexcept {
    for (auto next_symbol{peek_next_symbol()}; next_symbol.has_value() && std::isspace(*next_symbol) && *next_symbol != '\n';
         next_symbol = peek_next_symbol()) {
      std::ignore = take_next_symbol();
    }
  }

  void skip_comment() noexcept {
    if (auto next_symbol{peek_next_symbol()}; next_symbol.has_value() && *next_symbol == '/') {
      if (auto next_next_symbol{peek_next_symbol(1)}; next_next_symbol.has_value() && *next_next_symbol == '/') {
        take_next_symbol();
        take_next_symbol();
        while (true) {
          auto current_symbol{peek_next_symbol()};
          if (!current_symbol.has_value() || *current_symbol == '\n') {
            break;
          }
          take_next_symbol();
        }
      }
    }
  }

  std::optional<char> take_next_symbol() noexcept {
    if (text.empty()) {
      return std::nullopt;
    }

    char symbol{text[0]};

    if (symbol == '\n') {
      line_num++;
      column_num = 0;
    } else {
      column_num++;
    }

    text.remove_prefix(1);
    return symbol;
  }

  std::optional<char> peek_next_symbol(size_t pos = 0) noexcept {
    if (pos >= text.size()) {
      return std::nullopt;
    }

    return text[pos];
  }

  std::string_view text;
  size_t line_num;
  size_t column_num;
};

} // namespace impl_

inline std::expected<std::vector<token>, std::string> tokenize_text(std::string_view text) noexcept {
  auto parser{impl_::lexeme_parser(text)};

  // reserve?
  std::vector<token> tokens;

  while (true) {
    auto token_res{parser.take_next_token()};
    if (!token_res.has_value()) {
      return std::unexpected{std::move(token_res.error())};
    }
    auto token_opt{*token_res};
    if (!token_opt.has_value()) {
      return tokens;
    }
    tokens.push_back(*token_opt);
  }
}

} // namespace lexer
