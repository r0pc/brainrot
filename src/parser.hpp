#pragma once

#include <cassert>
#include <cmath>
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
  variant<NodeBinExprAdd *, NodeBinExprMulti *, NodeBinExprSub *,
          NodeBinExprDiv *>
      var;
};

struct NodeTerm {
  variant<NodeTermInt *, NodeTermIdent *, NodeTermParen *> var;
};

struct NodeExpr {
  variant<NodeTerm *, NodeBinExpr *> var;
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
  vector<NodeStmt *> stmts;
};

struct NodeIfPred;

struct NodeIfPredElif {
  NodeExpr *expr{};
  NodeScope *scope{};
  optional<NodeIfPred *> pred;
};

struct NodeIfPredElse {
  NodeScope *scope;
};

struct NodeIfPred {
  variant<NodeIfPredElif *, NodeIfPredElse *> var;
};

struct NodeStmtIf {
  NodeExpr *expr{};
  NodeScope *scope{};
  optional<NodeIfPred *> pred;
};

struct NodeStmtAssign {
  Token ident;
  NodeExpr *expr{};
};

struct NodeStmt {
  variant<NodeStmtExit *, NodeStmtLet *, NodeScope *, NodeStmtIf *,
          NodeStmtAssign *>
      var;
};

struct NodeProg {
  vector<NodeStmt *> stmts;
};
class Parser {
public:
  explicit Parser(vector<Token> tokens)
      : m_tokens(std::move(tokens)), m_allocator(1024 * 1024 * 5) {
  } // Allocates 5 mb

  void error_found(const string &err_msg) {
    cerr << "Syntax Error, Expected " << err_msg << " at line "
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

    if (auto open_pren = try_consume(TokenType::open_pren)) {
      auto expr = parse_expr();
      if (!expr) {
        error_found("expression");
      }

      try_consume_err(TokenType::close_pren);
      auto term_pren = m_allocator.emplace<NodeTermParen>(expr.value());
      auto term = m_allocator.emplace<NodeTerm>(term_pren);
    }
    return {};
  }

  optional<NodeExpr *> parse_expr(const int min_hier = 0) {
    optional<NodeTerm *> term_lhs = parse_term();
    if (!term_lhs) {
      return {};
    }
    auto expr_lhs = m_allocator.emplace<NodeExpr>(term_lhs.value());

    while (true) {
      optional<Token> current = next();
      optional<int> hier;

      if (current) {
        hier = bin_hier(current->type);
        if (!hier || hier < min_hier) {
          break;
        }
      } else {
        break;
      }

      const auto [type, line, value] = consume();
      const int next_min_hier = hier.value() + 1;
      auto expr_rhs = parse_expr(next_min_hier);

      if (!expr_rhs) {
        error_found("expression");
      }

      auto expr = m_allocator.emplace<NodeBinExpr>();
      auto expr_lhs_2 = m_allocator.emplace<NodeExpr>();

      if (type == TokenType::add) {
        expr_lhs_2->var = expr_lhs->var;
        auto add =
            m_allocator.emplace<NodeBinExprAdd>(expr_lhs_2, expr_rhs.value());
        expr->var = add;
      }

      else if (type == TokenType::sub) {
        expr_lhs_2->var = expr_lhs->var;
        auto sub =
            m_allocator.emplace<NodeBinExprSub>(expr_lhs_2, expr_rhs.value());
        expr->var = sub;
      }

      else if (type == TokenType::mul) {
        expr_lhs_2->var = expr_lhs->var;
        auto mul =
            m_allocator.emplace<NodeBinExprMulti>(expr_lhs_2, expr_rhs.value());
        expr->var = mul;
      }

      else if (type == TokenType::div) {
        expr_lhs_2->var = expr_lhs->var;
        auto div =
            m_allocator.emplace<NodeBinExprDiv>(expr_lhs_2, expr_rhs.value());
        expr->var = div;
      }

      else {
        assert(false);
      }

      expr_lhs->var = expr;
    }
    return expr_lhs;
  }

  optional<NodeStmt *> parse_stmt() {
    if (next().has_value() && next().value().type == TokenType::_return &&
        next(1).has_value() && next(1).value().type == TokenType::open_pren) {
      consume();
      consume();

      auto stmt_exit = m_allocator.emplace<NodeStmtExit>();
      auto node_expr = parse_expr();

      if (node_expr) {
        stmt_exit->expr = node_expr.value();
      } else {
        error_found("Expression");
      }

      try_consume_err(TokenType::close_pren);
      try_consume_err(TokenType::semi);

      auto stmt = m_allocator.emplace<NodeStmt>();
      stmt->var = stmt_exit;

      return stmt;
    }

    if (next().has_value() && next().value().type == TokenType::let &&
        next(1).has_value() && next(1).value().type == TokenType::ident &&
        next(2).has_value() && next(2).value().type == TokenType::eq) {

      consume();

      auto stmt_let = m_allocator.emplace<NodeStmtLet>();
      stmt_let->ident = consume();
      consume();

      auto expr = parse_expr();

      if (expr) {
        stmt_let->expr = expr.value();
      } else {
        error_found("Expression");
      }

      try_consume_err(TokenType::semi);

      auto stmt = m_allocator.emplace<NodeStmt>();
      stmt->var = stmt_let;

      return stmt;
    }

    if (next() && next().value().type == TokenType::ident && next(1) &&
        next(1).value().type == TokenType::eq) {

      auto assign = m_allocator.emplace<NodeStmtAssign>();
      assign->ident = consume();
      consume();

      auto expr = parse_expr();

      if (expr) {
        assign->expr = expr.value();
      } else {
        error_found("Expression");
      }

      try_consume_err(TokenType::semi);

      auto stmt = m_allocator.emplace<NodeStmt>(assign);

      return stmt;
    }

    return {};
  }

  optional<NodeProg> parse_prog() {
    NodeProg prog;

    while (next()) {

      if (auto stmt = parse_stmt()) {
        prog.stmts.push_back(stmt.value());
      } else {
        error_found("statement");
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

    error_found(token_to_string(type));
    return {};
  }

  const vector<Token> m_tokens;
  size_t m_index = 0;
  ArenaAllocator m_allocator;
};
