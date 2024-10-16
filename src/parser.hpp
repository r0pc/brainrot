#pragma once

#include <cstddef>
#include <cstdlib>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "arena.hpp"
#include "tokenizer.hpp"

using namespace std;

struct NodeTermInt {
  Token _int;
};

struct NodeTermIdent {
  Token ident;
};

struct NodeExpr;

struct NodeTermParen {
  NodeExpr *expr;
};

struct NodeBinExprAdd {
  NodeExpr *lhs;
  NodeExpr *rhs;
};

struct NodeBinExprMulti {
  NodeExpr *lhs;
  NodeExpr *rhs;
};

struct NodeBinExprSub {
  NodeExpr *lhs;
  NodeExpr *rhs;
};

struct NodeBinExprDiv {
  NodeExpr *lhs;
  NodeExpr *rhs;
};

struct NodeBinExpr {
  std::variant<NodeBinExprAdd *, NodeBinExprMulti *, NodeBinExprSub *,
               NodeBinExprDiv *>
      var;
};

struct NodeTerm {
  std::variant<NodeTermInt *, NodeTermIdent *, NodeTermParen *> var;
};

struct NodeExpr {
  std::variant<NodeTerm *, NodeBinExpr *> var;
};

struct NodeStmtExit {
  NodeExpr *expr;
};

struct NodeStmtLet {
  Token ident;
  NodeExpr *expr{};
};

struct NodeStmt;

struct NodeScope {
  std::vector<NodeStmt *> stmts;
};

struct NodeIfPred;

struct NodeIfPredElif {
  NodeExpr *expr{};
  NodeScope *scope{};
  std::optional<NodeIfPred *> pred;
};

struct NodeIfPredElse {
  NodeScope *scope;
};

struct NodeIfPred {
  std::variant<NodeIfPredElif *, NodeIfPredElse *> var;
};

struct NodeStmtIf {
  NodeExpr *expr{};
  NodeScope *scope{};
  std::optional<NodeIfPred *> pred;
};

struct NodeStmtAssign {
  Token ident;
  NodeExpr *expr{};
};

struct NodeStmt {
  std::variant<NodeStmtExit *, NodeStmtLet *, NodeScope *, NodeStmtIf *,
               NodeStmtAssign *>
      var;
};

struct NodeProg {
  std::vector<NodeStmt *> stmts;
};
class Parser {
public:
  explicit Parser(vector<Token> tokens)
      : m_tokens(std::move(tokens)), m_allocator(1024 * 1024 * 5) {
  } // Allocates 5 mb

  void error_found(const string &err_msg) {
    cerr << "Error In Parsing, Expected " << err_msg << " at line "
         << next(-1).value().line << endl;
    exit(EXIT_FAILURE);
  }

  optional<NodeTerm *> parse_term() {
    if (auto _int = try_consume(TokenType::_int)) {
      auto term_int = m_allocator.emplace<NodeTermInt>(_int.value());
      auto term = m_allocator.emplace<NodeTerm>(term_int);
      return term;
    }

    if (auto ident = try_consume(TokenType::ident)) {
      auto term_ident = m_allocator.emplace<NodeTermIdent>(ident.value());
      auto term = m_allocator.emplace<NodeTerm>(term_ident);
      return term;
    }
  }

  optional<NodeExpr *> parse_expr() {
    optional<NodeTerm *> term_lhs = parse_term();
    if (!term_lhs) {
      return {};
    }
  }

  optional<NodeStmt *> parse_stmt() {
    if (next() && next()->type == TokenType::_return && next(1) &&
        next(1)->type == TokenType::open_pren) {
      consume();
      consume();
      auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
      auto node_expr = parse_expr();
      if (node_expr) {
        stmt_exit->expr = node_expr.value();
      } else {
        error_found("expression");
      }
    }
  }

  optional<NodeProg> parse_prog() {
    NodeProg prog;
    while (next()) {
      auto stmt = parse_stmt();
      if (stmt) {
        prog.stmts.push_back(stmt.value());
      } else {
        error_found("Stmt");
      }
    }
    return prog;
  }

private:
  [[nodiscard]] optional<Token> next(size_t offset = 0) {
    if (m_index + offset >= m_tokens.size()) {
      return {};
    }
    return m_tokens[m_index + offset];
  }

  Token consume() { return m_tokens.at(m_index++); }

  optional<Token> try_consume(const TokenType type) {
    if (next() && next().value().type == type) {
      return consume();
    }
    return {};
  }

  Token try_consume_err(const TokenType type) {
    if (next() && next().value().type == type) {
      return consume();
    }
    error_found(to_string(type));
    return {};
  }

  const vector<Token> m_tokens;
  size_t m_index = 0;
  ArenaAllocator m_allocator;
};
