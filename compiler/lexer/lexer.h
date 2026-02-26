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

inline const std::unordered_map<std::string_view, token_type> keywords = {
  {"class", token_type::tok_kw_class},
  {"extends", token_type::tok_kw_extends},
  {"is", token_type::tok_kw_is},
  {"var", token_type::tok_kw_var},
  {"method", token_type::tok_kw_method},
  {"if", token_type::tok_kw_if},
  {"then", token_type::tok_kw_then},
  {"else", token_type::tok_kw_else},
  {"while", token_type::tok_kw_while},
  {"loop", token_type::tok_kw_loop},
  {"return", token_type::tok_kw_return},
  {"end", token_type::tok_kw_end},
  {"this", token_type::tok_kw_this},
  {"true", token_type::tok_kw_true},
  {"false", token_type::tok_kw_false}
};

inline bool is_identifier_char(char c, bool first_char = false) noexcept {
  if (first_char) {
    return std::isalpha(c) != 0 || c == '_';
  }
  return std::isalnum(c) != 0 || c == '_';
}

inline bool is_digit(char c) noexcept {
  return std::isdigit(static_cast<unsigned char>(c));
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
      return *ident_token;
    }

    if (auto ident_token{try_number()}; ident_token.has_value()) {
      return *ident_token;
    }

    auto symbol_opt{take_next_symbol()};

    if (!symbol_opt.has_value()) {
      return std::nullopt;
    }

    char symbol{*symbol_opt};

    switch (symbol) {
    case '(':
      return token{.type = token_type::tok_open_par, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = "("};
    case ')':
      return token{.type = token_type::tok_close_par, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = ")"};
    case ':':
      if (auto next_symbol{peek_next_symbol().value_or(' ')}; next_symbol == '=') {
        std::ignore = take_next_symbol();
        return token{.type = token_type::tok_assignment, .span{.line_num = line_num, .start_pos = column_num - 1, .end_pos = column_num + 1}, .value = ":="};
      } else {
        return token{.type = token_type::tok_colon, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = ":"};
      }
    case '.':
      return token{.type = token_type::tok_dot, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = "."};
    case ',':
      return token{.type = token_type::tok_comma, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = ","};
    case '=':
      if (auto next_symbol{peek_next_symbol().value_or(' ')}; next_symbol == '>') {
        std::ignore = take_next_symbol();
        return token{.type = token_type::tok_fat_arrow, .span{.line_num = line_num, .start_pos = column_num - 1, .end_pos = column_num + 1}, .value = "=>"};
      } else {
        return std::unexpected{std::format("unknown token \'{}\' at line : {}, column : {}", symbol, line_num, column_num)};
      }
    case '\n':
      return token{.type = token_type::tok_new_line, .span{.line_num = line_num, .start_pos = column_num, .end_pos = column_num + 1}, .value = "\\n"};
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
          .type = it->second,
          .span = {.line_num = line_num, .start_pos = start_col, .end_pos = column_num + 1},
          .value = std::string(ident)
        };
      } else {
        return token{
          .type = token_type::tok_identifier,
          .span = {.line_num = line_num, .start_pos = start_col, .end_pos = column_num + 1},
          .value = std::string(ident)
        };
      }
    }

    return std::unexpected{std::format("didn't found keyword or identifier")};
  }

  std::expected<std::optional<token>, std::string> try_number() {
    auto symbol_opt{peek_next_symbol()};

    if (!symbol_opt.has_value()) {
      return std::nullopt;
    }

    char symbol{*symbol_opt};

    size_t start_col = column_num + 1;
    if (is_digit(symbol)) {
      size_t length = 1;

      auto next_symbol{peek_next_symbol(length).value_or(' ')};
      while (is_digit(next_symbol)) {
        length++;
        next_symbol = peek_next_symbol(length).value_or(' ');
      }

      auto separator{peek_next_symbol(length).value_or(' ')};
      auto after_separator{peek_next_symbol(length+1).value_or(' ')};
      bool is_real(separator == '.' && is_digit(after_separator));

      if (is_real) {
        length++;
        next_symbol = peek_next_symbol(length).value_or(' ');
        while (is_digit(next_symbol)) {
          length++;
          next_symbol = peek_next_symbol(length).value_or(' ');
        }
      }

      auto num_opt = take_sequence(length);
      if (not num_opt.has_value()) {
        return std::unexpected{std::format("tried to get sequence out of text at line : {}, from : {} to : {}", line_num, column_num, column_num + length)};
      }
      auto num = *num_opt;

      if (is_real) {
    return token{
        .type = token_type::tok_real,
        .span = {.line_num = line_num, .start_pos = start_col, .end_pos = column_num + 1},
        .value = std::stod(std::string(num))
    };
    } else {
        return token{
            .type = token_type::tok_int,
            .span = {.line_num = line_num, .start_pos = start_col, .end_pos = column_num + 1},
            .value = std::stoi(std::string(num))
        };
}
    }

    return std::unexpected{std::format("didn't found number")};
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
