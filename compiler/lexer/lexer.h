#pragma once

#include <cctype>
#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

#include "compiler/lexer/token.h"

namespace lexer {

namespace impl_ {

struct lexeme_parser {

  lexeme_parser(std::string_view text) noexcept
      : text(text), line_num{}, column_num{} {}

  std::optional<token> take_next_token() noexcept {
    skip_whitespace();
    // skip comment
    auto symbol_opt{take_next_symbol()};

    if (!symbol_opt.has_value()) {
      return std::nullopt;
    }

    char symbol{*symbol_opt};

    switch (symbol) {
    case '(':
      return token{.type = token_type::tok_open_par,
                   .span{.line_num = line_num,
                         .start_pos = column_num,
                         .end_pos = column_num + 1},
                   .value = "("};
      break;
    case ')':
      return token{.type = token_type::tok_close_par,
                   .span{.line_num = line_num,
                         .start_pos = column_num,
                         .end_pos = column_num + 1},
                   .value = ")"};
      break;
    case ':':
      if (auto next_symbol{peek_next_symbol().value_or(' ')};
          next_symbol == '=') {
        std::ignore = take_next_symbol();
        return token{.type = token_type::tok_assignment,
                     .span{.line_num = line_num,
                           .start_pos = column_num - 1,
                           .end_pos = column_num + 1},
                     .value = ":="};
      } else {
        return token{.type = token_type::tok_colon,
                     .span{.line_num = line_num,
                           .start_pos = column_num,
                           .end_pos = column_num + 1},
                     .value = ":"};
      }
      break;
    case '.':
      return token{.type = token_type::tok_dot,
                   .span{.line_num = line_num,
                         .start_pos = column_num,
                         .end_pos = column_num + 1},
                   .value = "."};
      break;
    case ',':
      return token{.type = token_type::tok_comma,
                   .span{.line_num = line_num,
                         .start_pos = column_num,
                         .end_pos = column_num + 1},
                   .value = ","};
      break;
    case '=':
      if (auto next_symbol{peek_next_symbol().value_or(' ')};
          next_symbol == '>') {
        std::ignore = take_next_symbol();
        return token{.type = token_type::tok_fat_arrow,
                     .span{.line_num = line_num,
                           .start_pos = column_num - 1,
                           .end_pos = column_num + 1},
                     .value = "=>"};
      } else {
        return std::nullopt; // todo error here
      }
      break;
    case '\n':
      return token{.type = token_type::tok_new_line,
                   .span{.line_num = line_num,
                         .start_pos = column_num,
                         .end_pos = column_num + 1},
                   .value = "\n"};
      break;

      // todo:
      // - parse tok_int, tok_real (int.int) , tok_bool
      // - parse name => after check is name is keyword and which by swich
      // case for keywords
      //
    }

    return std::nullopt;
  }

private:
  void skip_whitespace() noexcept {
    for (auto next_symbol{peek_next_symbol()};
         next_symbol.has_value() && std::isspace(*next_symbol) &&
         *next_symbol != '\n';
         next_symbol = peek_next_symbol()) {
      std::ignore = take_next_symbol();
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

  std::optional<char> peek_next_symbol() noexcept {
    if (text.empty()) {
      return std::nullopt;
    }

    return text[0];
  }

  std::string_view text;
  size_t line_num;
  size_t column_num;
};

} // namespace impl_

inline std::vector<token> tokenize_text(std::string_view text) noexcept {
  auto parser{impl_::lexeme_parser(text)};

  // reserve?
  std::vector<token> tokens;
  for (auto tok{parser.take_next_token()}; tok != std::nullopt;
       tok = parser.take_next_token()) {
    tokens.push_back(std::move(*tok));
  }
  return tokens;
}

} // namespace lexer
