#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "parser.hpp"

using namespace std;

class Generator {
public:
  explicit Generator(NodeProg prog) : m_prog(std::move(prog)) {};

  void gen_term(const NodeTerm *term) {
    struct TermVisitor {
      Generator &gen;

      void operator()(const NodeTermInt *term_int) const {
        gen.m_output << "\tmov rax, " << term_int->_int.value.value() << '\n';
        gen.push("rax");
      }

      void operator()(const NodeTermIdent *term_ident) const {
        const auto i = find_if(
            gen.m_vars.cbegin(), gen.m_vars.cend(), [&](const Var &var) {
              return var.name == term_ident->ident.value.value();
            });

        if (i == gen.m_vars.cend()) {
          cerr << "Undeclared Identifier: " << term_ident->ident.value.value()
               << endl;
          exit(EXIT_FAILURE);
        }

        stringstream offset;
        offset << "QWORD [rsp + " << (gen.m_stack_size - i->stack_loc - 1) * 8
               << "]";
        gen.push(offset.str());
      }

      void operator()(const NodeTermParen *term_pren) const {
        gen.gen_expr(term_pren->expr);
      }
    };

    TermVisitor visitor({.gen = *this});
    visit(visitor, term->var);
  }

  void gen_bin_expr(const NodeBinExpr *bin_expr) {
    struct BinExprVisitor {
      Generator &gen;

      void operator()(const NodeBinExprAdd *add) const {
        gen.gen_expr(add->rhs);
        gen.gen_expr(add->lhs);
        gen.pop("rax");
        gen.pop("rbx");
        gen.m_output << "\tadd rax, rbx\n";
        gen.push("rax");
      }

      void operator()(const NodeBinExprSub *sub) const {
        gen.gen_expr(sub->rhs);
        gen.gen_expr(sub->lhs);
        gen.pop("rax");
        gen.pop("rbx");
        gen.m_output << "\tsub rax, rbx\n";
        gen.push("rax");
      }

      void operator()(const NodeBinExprMulti *mul) const {
        gen.gen_expr(mul->rhs);
        gen.gen_expr(mul->lhs);
        gen.pop("rax");
        gen.pop("rbx");
        gen.m_output << "\tmul rbx\n";
        gen.push("rax");
      }

      void operator()(const NodeBinExprDiv *div) const {
        gen.gen_expr(div->rhs);
        gen.gen_expr(div->lhs);
        gen.pop("rax");
        gen.pop("rbx");
        gen.m_output << "\tdiv rbx\n";
        gen.push("rax");
      }
    };

    BinExprVisitor visitor{.gen = *this};
    visit(visitor, bin_expr->var);
  }

  void gen_expr(const NodeExpr *expr) {
    struct ExprVistor {
      Generator &gen;
      void operator()(const NodeTerm *term) const { gen.gen_term(term); }

      void operator()(const NodeBinExpr *bin_expr) const {
        gen.gen_bin_expr(bin_expr);
      }
    };

    ExprVistor visitor{.gen = *this};
    visit(visitor, expr->var);
  }

  void gen_stmt(const NodeStmt *stmt) {
    struct StmtVisitor {
      Generator &gen;

      void operator()(const NodeStmtExit *stmt_exit) const {
        gen.m_output << "\t;; exit\n";
        gen.gen_expr(stmt_exit->expr);
        gen.m_output << "\tmov rax, 60\n";
        gen.pop("rdi");
        gen.m_output << "\tsyscall\n";
        gen.m_output << "\t;; /exit\n";
      }

      void operator()(const NodeStmtLet *stmt_let) const {
        gen.m_output << "\t;; let\n";

        const auto i = find_if(
            gen.m_vars.cbegin(), gen.m_vars.cend(), [&](const Var &var) {
              return var.name == stmt_let->ident.value.value();
            });

        if (i != gen.m_vars.cend()) {
          cerr << "Identifier already exists '" << stmt_let->ident.value.value()
               << "'" << endl;
          exit(EXIT_FAILURE);
        }

        gen.m_vars.push_back({.name = stmt_let->ident.value.value(),
                              .stack_loc = gen.m_stack_size});
        gen.gen_expr(stmt_let->expr);
        gen.m_output << "\t;; /let\n";
      }

      void operator()(const NodeStmtAssign *stmt_assign) const {

        const auto i = find_if(
            gen.m_vars.cbegin(), gen.m_vars.cend(), [&](const Var &var) {
              return var.name == stmt_assign->ident.value.value();
            });

        if (i == gen.m_vars.end()) {
          cerr << "Undeclared Identifier: " << stmt_assign->ident.value.value()
               << endl;
          exit(EXIT_FAILURE);
        }
        gen.gen_expr(stmt_assign->expr);
        gen.pop("rax");
        gen.m_output << "\tmov [rsp + "
                     << (gen.m_stack_size - i->stack_loc - 1) * 8 << "], rax\n";
      }

      void operator()(const NodeScope *scope) const {}
      void operator()(const NodeStmtIf *stmt_if) const {}
    };
    StmtVisitor visitor{.gen = *this};
    visit(visitor, stmt->var);
  }

  [[nodiscard]] string gen_prog() {
    m_output << "global _start\n_start :\n ";

    for (const NodeStmt *stmt : m_prog.stmts) {
      gen_stmt(stmt);
    }

    m_output << "\tmov rax, 60\n";
    m_output << "\tmov rdi, 0\n";
    m_output << "\tsyscall\n";

    return m_output.str();
  }

private:
  void push(const string &reg) {
    m_output << "\tpush " << reg << '\n';
    m_stack_size++;
  }

  void pop(const string &reg) {
    m_output << "\tpop " << reg << '\n';
    m_stack_size--;
  }

  struct Var {
    string name;
    size_t stack_loc;
  };

  string create_label() {
    stringstream tmp;
    tmp << "label" << m_label_count++;
    return tmp.str();
  }

  const NodeProg m_prog;
  stringstream m_output;
  size_t m_stack_size = 0;
  int m_label_count = 0;
  vector<Var> m_vars{};
};
