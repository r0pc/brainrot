#pragma once

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

using namespace std;

enum class TokenType { _return, _int, semi, let, _if, _else, var };

struct Token {
  TokenType type;
  int line;
  optional<string> value;
};

class Tokenizer {
public:
  /*explicit Tokenizer(string src) : m_src(std::move(src)) {}*/
  explicit Tokenizer(std::string src) : m_src(std::move(src)) {}

  vector<Token> tokenize() {
    string buffer;
    vector<Token> tokens;
    int line_num = 1;
    while (peek()) {
      if (isalpha(peek().value())) {
        
        buffer.push_back(consume());

        while (peek().has_value() && isalnum(peek().value())) {
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

      }
      else if(isdigit(peek().value())){
        
        buffer.push_back(consume());
        
        while(peek().value()&&isdigit(peek().value())){
          buffer.push_back(consume());
        }
        
        tokens.push_back({TokenType::_int, line_num, buffer});
        buffer.clear();
      }

      else if(isspace(peek().value())){
        consume();
      }

      else if(peek().value()=='\n'){
        consume();
        line_num++;
      }

      else{
        cerr<<"Invalid Token"<<endl;
        exit(EXIT_FAILURE);
      }
    }
    m_index = 0;
    return tokens;
  }

private:
  [[nodiscard]] inline optional<char> peek(size_t offset = 0) {
    if (m_index + offset >= m_src.length()) {
      return NULL;
    }
    return m_src[m_index + offset];
  }

  inline char consume() { return m_src[m_index++]; }

  const string m_src;
  size_t m_index = 0;
};
