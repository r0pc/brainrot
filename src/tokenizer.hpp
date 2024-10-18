#pragma once

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using namespace std;

enum class TokenType {
  _return, // return
  _int,    // integer
  semi,    // ';'
  let,     // let, variable assignment operator
  eq,      // '='
  _if,     // if
  _else,   // else
  ident,   // variable ex; x

  open_pren,   // '('
  close_pren,  // ')'
  open_box,    // '['
  close_box,   // ']'
  open_curly,  // '{'
  close_curly, // '}'

  add, // '+'
  sub, // '-'
  mul, // '*'
  div  // '/'
};

inline string token_to_string(const TokenType type) {
  switch (type) {
  case TokenType::_return:
    return "'return'";
  case TokenType::let:
    return "'let'";
  case TokenType::_int:
    return "'int'";
  case TokenType::semi:
    return "';'";
  case TokenType::_if:
    return "'if'";
  case TokenType::_else:
    return "'else'";
  case TokenType::ident:
    return "'identifier'";
  case TokenType::open_pren:
    return "'('";
  case TokenType::close_pren:
    return "')'";
  case TokenType::eq:
    return "'='";

  case TokenType::add:
    return "'+'";
  case TokenType::mul:
    return "'*'";
  case TokenType::div:
    return "'/'";
  case TokenType::sub:
    return "'-'";
  case TokenType::open_box:
    return "'['";
  case TokenType::open_curly:
    return "'{'";
  case TokenType::close_box:
    return "']'";
  case TokenType::close_curly:
    return "'}'";
  }
  assert(false);
}

inline optional<int> bin_hier(const TokenType type) {
  switch (type) {
  case TokenType::add:
  case TokenType::sub:
    return 0;

  case TokenType::div:
  case TokenType::mul:
    return 1;

  default:
    return {};
  }
}

struct Token {
  TokenType type;
  int line;
  optional<string> value{};
};

class Tokenizer {
public:
  explicit Tokenizer(string src) : m_src(std::move(src)) {}

  vector<Token> tokenize() {
    string buffer;
    vector<Token> tokens;
    int line_num = 1;
    while (next()) {
      if (isalpha(next().value())) {

        buffer.push_back(consume());

        while (next() && isalnum(next().value())) {
          buffer.push_back(consume());
        }

        if (buffer == "exit") {
          tokens.push_back({TokenType::_return, line_num});
          buffer.clear();
        } else if (buffer == "let") {
          tokens.push_back({TokenType::let, line_num});
          buffer.clear();
        } else if (buffer == "let") {
          tokens.push_back({TokenType::let, line_num});
          buffer.clear();
        } else if (buffer == "if") {
          tokens.push_back({TokenType::_if, line_num});
          buffer.clear();
        } else if (buffer == "else") {
          tokens.push_back({TokenType::_else, line_num});
          buffer.clear();
        } else {
          tokens.push_back({TokenType::ident, line_num, buffer});
          buffer.clear();
        }

      } else if (isdigit(next().value())) {

        buffer.push_back(consume());

        while (next().value() && isdigit(next().value())) {
          buffer.push_back(consume());
        }

        tokens.push_back({TokenType::_int, line_num, buffer});
        buffer.clear();
      }

      else if (next().value() == '(') {
        consume();
        tokens.push_back({TokenType::open_pren, line_num});
      } else if (next().value() == ')') {
        consume();
        tokens.push_back({TokenType::close_pren, line_num});
      } else if (next().value() == '=') {
        consume();
        tokens.push_back({TokenType::eq, line_num});
      } else if (next().value() == '+') {
        consume();
        tokens.push_back({TokenType::add, line_num});
      } else if (next().value() == '+') {
        consume();
        tokens.push_back({TokenType::sub, line_num});
      } else if (next().value() == '/') {
        consume();
        tokens.push_back({TokenType::div, line_num});
      } else if (next().value() == '*') {
        consume();
        tokens.push_back({TokenType::mul, line_num});
      } else if (next().value() == '{') {
        consume();
        tokens.push_back({TokenType::open_curly, line_num});
      } else if (next().value() == '}') {
        consume();
        tokens.push_back({TokenType::close_curly, line_num});
      } else if (next().value() == '[') {
        consume();
        tokens.push_back({TokenType::open_box, line_num});
      } else if (next().value() == ']') {
        consume();
        tokens.push_back({TokenType::close_box, line_num});
      } else if (next().value() == ';') {
        consume();
        tokens.push_back({TokenType::semi, line_num});
      } else if (next().value() == '\n') {
        consume();
        line_num++;
      } else if (isspace(next().value())) {
        consume();
      }

      else {
        cerr << "Invalid Token" << endl;
        exit(EXIT_FAILURE);
      }
    }
    m_index = 0;
    return tokens;
  }

private:
  [[nodiscard]] optional<char> next(size_t offset = 0) {
    if (m_index + offset >= m_src.length()) {
      return {};
    }
    return m_src[m_index + offset];
  }

  char consume() { return m_src.at(m_index++); }

  const string m_src;
  size_t m_index = 0;
};
