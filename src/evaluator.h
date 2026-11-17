#pragma once
#include "ast.h"
#include "symbol.h"
#include "token.h"
#include "value.h"
#include <memory>
#include <optional>

class Evaluator { 
public:
    Evaluator();
    std::optional<Value> Run(SyntaxTree &tree);

private:
    std::optional<Value> Run(std::unique_ptr<ASTNode> &node);
    std::optional<Value> RunVarDeclaration(VarDeclaration &node);
    std::optional<Value> RunBlock(Block &node);
    Value EvaluateExpression(std::unique_ptr<ASTNode> &node);

    Value EvaluateBinary(BinaryOperation &node);
    Value EvaluateUnary(UnaryOperation &node);
    Value EvaluateIdentifier(Identifier &node);

    Value EvaluateReassignment(VarReassignment &node);

    Value EvaluateLogical(BinaryOperation &node);
    Value EvaluateComparison(parse::TokenKind operator_, const Value &lhs, const Value &rhs);
    Value EvaluateArithmetic(parse::TokenKind operator_, const Value &lhs, const Value &rhs);

    ScopeStack symbols;
};
