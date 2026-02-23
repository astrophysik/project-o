#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "compiler/lexer/token.h"

namespace lexer {

namespace impl_ {

inline const std::unordered_map<std::string_view, keyword_value::type> keywords = {
  {"class", keyword_value::type::kw_class},
  {"extends", keyword_value::type::kw_extends},
  {"is", keyword_value::type::kw_is},
  {"var", keyword_value::type::kw_var},
  {"method", keyword_value::type::kw_method},
  {"if", keyword_value::type::kw_if},
  {"then", keyword_value::type::kw_then},
  {"else", keyword_value::type::kw_else},
  {"while", keyword_value::type::kw_while},
  {"loop", keyword_value::type::kw_loop},
  {"return", keyword_value::type::kw_return},
  {"end", keyword_value::type::kw_end},
  {"this", keyword_value::type::kw_this},
  {"true", keyword_value::type::kw_true},
  {"false", keyword_value::type::kw_false}
};

inline bool is_identifier_char(char c, bool first_char = false) noexcept {
  if (first_char) {
    return std::isalpha(c) != 0 || c == '_';
  }
  return std::isalnum(c) != 0 || c == '_';
}

struct lexeme_parser {

  lexeme_parser(std::string_view text) noexcept
      : text(text),
        line_num{},
        column_num{} {}

  std::expected<std::optional<token>, std::string> take_next_token() noexcept {
    skip_whitespace();
    skip_comment();

    if (auto ident_token{try_identificator_or_keyword()}; ident_token.has_value()) {
      if ((*(*ident_token)).type != token_type::tok_unknown) {
         return ident_token;
      } // else not error and not identifier token
    } else {
      return ident_token;
    }

    auto symbol_opt{take_next_symbol()};

    if (!symbol_opt.has_value()) {
      return std::nullopt;
    }

    char symbol{*symbol_opt};

    switch (symbol) {
    case '(':
      return token{.type = token_type::tok_open_par, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = open_par_value{}};
    case ')':
      return token{.type = token_type::tok_close_par, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = close_par_value{}};
    case ':':
      if (auto next_symbol{peek_next_symbol().value_or(' ')}; next_symbol == '=') {
        std::ignore = take_next_symbol();
        return token{.type = token_type::tok_assignment, .span{.line_num = line_num, .start_pos = column_num - 1, .end_pos = column_num + 1}, .value = assignment_value{}};
      } else {
        return token{.type = token_type::tok_colon, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = colon_value{}};
      }
    case '.':
      return token{.type = token_type::tok_dot, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = dot_value{}};
    case ',':
      return token{.type = token_type::tok_comma, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = comma_value{}};
    case '=':
      if (auto next_symbol{peek_next_symbol().value_or(' ')}; next_symbol == '>') {
        std::ignore = take_next_symbol();
        return token{.type = token_type::tok_fat_arrow, .span{.line_num = line_num, .start_pos = column_num - 1, .end_pos = column_num + 1}, .value = fat_arrow_value{}};
      } else {
        return std::unexpected{std::format("unknown token \'{}\' at line : {}, column : {}", symbol, line_num, column_num)};
      }
    case '\n':
      return token{.type = token_type::tok_new_line, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = new_line_value{}};
      // todo:
      // - parse tok_int, tok_real (int.int)
    }

    return std::unexpected{std::format("unknown token \'{}\' at line : {}, column : {}", symbol, line_num, column_num)};
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

  std::expected<std::optional<token>, std::string> try_identificator_or_keyword() {
    auto symbol_opt{peek_next_symbol()};

    if (!symbol_opt.has_value()) {
      return std::nullopt;
    }

    char symbol{*symbol_opt};

    size_t start_col = column_num + 1;
    if (is_identifier_char(symbol, true)) {
      size_t length = 1;

      auto next_symbol{peek_next_symbol(length).value_or(' ')};
      while (is_identifier_char(next_symbol)) {
        length++;
        next_symbol = peek_next_symbol(length).value_or(' ');
      }

      auto ident_opt = take_sequence(length);
      if (not ident_opt.has_value()) {
        return std::unexpected{std::format("tried to get sequence out of text at line : {}, from : {} to : {}", line_num, column_num, column_num + length)};
      }
      auto ident = *ident_opt;

      if (auto it = keywords.find(ident); it != keywords.end()) {
        return token{
          .type = token_type::tok_keyword,
          .span = {.line_num = line_num, .start_pos = start_col, .end_pos = column_num + 1},
          .value = keyword_value{.kind = it->second}
        };
      } else {
        return token{
          .type = token_type::tok_identifier,
          .span = {.line_num = line_num, .start_pos = start_col, .end_pos = column_num + 1},
          .value = identifier_value{.name = std::string(ident)}
        };
      }
    }

    return token{
      .type = token_type::tok_unknown,
      .span = {.line_num = line_num, .start_pos = start_col, .end_pos = column_num + 1},
      .value = identifier_value{.name = ""}
    };
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

  std::optional<std::string_view> take_sequence(size_t length) noexcept {
    if (text.empty() or length > text.size()) {
      return std::nullopt;
    }

    std::string_view char_seq{text.substr(0, length)};

    auto new_line_symbols{std::ranges::count(char_seq, '\n')};
    if (new_line_symbols > 0) { // multi-line sequence
      auto last_new_line_symbol{char_seq.find_last_of('\n')};
      
      column_num = char_seq.size() - last_new_line_symbol + 1;
      line_num += new_line_symbols;
    } else { // one-line sequence
      column_num += length;
    }

    text.remove_prefix(length);

    return char_seq;
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
