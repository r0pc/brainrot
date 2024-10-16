#pragma once

#include <variant>

#include "tokenizer.hpp"
#include "arena.hpp"

using namespace std;

struct NodeTermInt{
  Token _int; 
};

struct NodeTermIdent{
  Token ident;
};

struct NodeExpr;

struct NodeTermParen{
NodeExpr* expr;
};

struct NodeTerm{
  variant<NodeTermInt*, NodeTermIdent*, NodeTermParen*> var;
};

struct NodeExpr{
  variant<NodeTerm*> var;
};

struct NodeStmtExit{
  NodeExpr* expr;
};

struct NodeStmtLet{
  Token ident;
  NodeExpr* expr{};
};
