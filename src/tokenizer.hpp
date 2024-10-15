#pragma once

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
  _return,    // return
  _int,       // integer
  semi,       // ';'
  let,        // let, variable assignment operator
  _if,        // if
  _else,      // else
  var,        // variable ex; x
  open_pren,  // '('
  close_pren, // ')'
  open_box,   // '['
  close_box,  // ']'
  open_curly, // '{'
  close_curly // '}'
};

struct Token {
  TokenType type;
  int line;
  optional<string> value;
};

class Tokenizer {
public:
  /*explicit Tokenizer(string src) : m_src(move(src)) {}*/
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

        if (buffer == "return") {
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
          tokens.push_back({TokenType::var, line_num, buffer});
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
