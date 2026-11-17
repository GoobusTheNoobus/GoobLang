#include "evaluator.h"
#include "ast.h"
#include "core.h"
#include "diagnostic.h"
#include "symbol.h"
#include "token.h"
#include "value.h"
#include <optional>
#include <stdexcept>
#include <string>

Evaluator::Evaluator() {
    symbols.PushScope();
    symbols.Define({.name = "GOOB_VERSION", .value = std::string(VERSION), .is_mut = false});
    symbols.Define({.name = "GOOB_IS_COOL", .value = true, .is_mut = false});
    symbols.Define({.name = "FUNNY_NUMBER", .value = static_cast<double>(67), .is_mut = false});
}

std::optional<Value> Evaluator::Run(SyntaxTree &tree) {

    // We store the last value that was evaluated.
    std::optional<Value> last = Value();
    for (auto &node : tree) {
        std::optional<Value> value = Run(node);

        if (!value) {
            last = value;
            continue;
        }

        if (value->type == Type::Error) {
            return std::nullopt;
        }

        last = value;
    }

    // If the caller is the REPL, it will print this value
    return last;
}

std::optional<Value> Evaluator::Run(std::unique_ptr<ASTNode> &node) {
    switch (node->Kind()) {
        case ASTNodeKind::VarDeclaration: 
            return RunVarDeclaration(static_cast<VarDeclaration &>(*node));
        
        case ASTNodeKind::Block: 
            return RunBlock(static_cast<Block &>(*node));

        default: 
            return EvaluateExpression(node);
    }
}

std::optional<Value> Evaluator::RunVarDeclaration(VarDeclaration &node) {

    if (symbols.LookupTopScope(node.name)) {
        diagnostic::ReportError("Symbol '" + node.name + "' has already been declared in the current scope.");
        return Error;
    }

    auto value = EvaluateExpression(node.value);

    if (!value) {
        return std::nullopt;
    }

    symbols.Define({.name = node.name, .value = value});
    return std::nullopt;
}

std::optional<Value> Evaluator::RunBlock(Block &block) {

    symbols.PushScope();

    for (auto &node : block.code) {
        auto run_result = Run(node);

        if(run_result && run_result->type == Type::Error) {
            return Error;
        }
    }

    symbols.PopScope();

    return std::nullopt;
}

Value Evaluator::EvaluateExpression(std::unique_ptr<ASTNode> &node) {

    switch (node->Kind()) {
        case ASTNodeKind::Number:
            return static_cast<Number &>(*node).value;
        case ASTNodeKind::String:
            return static_cast<String &>(*node).value;
        case ASTNodeKind::Boolean:
            return static_cast<Boolean &>(*node).value;
        case ASTNodeKind::Identifier:
            return EvaluateIdentifier(static_cast<Identifier &>(*node));
        case ASTNodeKind::BinaryOperation:
            return EvaluateBinary(static_cast<BinaryOperation &>(*node));
        case ASTNodeKind::UnaryOperation:
            return EvaluateUnary(static_cast<UnaryOperation &>(*node));
        case ASTNodeKind::VarReassignment: // Unlike variable delcarations, reassignments return the value it was assigned
            return EvaluateReassignment(static_cast<VarReassignment &>(*node));
        default: {
            throw std::runtime_error("Unknown ast node type id=" + std::to_string(static_cast<int>(node->Kind())));
        }
    }
}

Value Evaluator::EvaluateReassignment(VarReassignment &node) {

    Symbol *lookup = symbols.LookupAllScopes(node.name);

    if (!lookup) {
        diagnostic::ReportError("Symbol '" + node.name + "' has not yet been defined.");
        return Error;
    }

    if (!lookup->is_mut) {
        diagnostic::ReportError("Cannot assign to '" + node.name + "', it is immutable.");
        return Error;
    }

    Value value = EvaluateExpression(node.value);

    if (!value) {
        return Error;
    }

    symbols.Define({.name = node.name, .value = value});
    return value;
}

Value Evaluator::EvaluateUnary(UnaryOperation &node) {

    Value value = EvaluateExpression(node.operand);

    if (!value) return value;

    if (node.operator_ == parse::TokenKind::Bang) {
        return !value.IsTruthy();
    }

    switch (value.type) {
        case Type::Number: {
            return node.operator_ == parse::TokenKind::Plus ? *value.GetNum() : -(*value.GetNum());
        }

        default: {
            diagnostic::ReportError("Operator '" + parse::OperatorToString(node.operator_) +
                                     "' is not supported for type '" + TypeToString(value.type) + "'.");
            return Error;
        }
    }
}

Value Evaluator::EvaluateBinary(BinaryOperation &node) {

    if (parse::IsLogicalOp(node.operator_))
        return EvaluateLogical(node);

    Value lhs = EvaluateExpression(node.operand1);

    if (!lhs) {
        return lhs;
    }

    Value rhs = EvaluateExpression(node.operand2);

    if (!rhs) {
        return Error;
    }

    if (parse::IsComparisonOp(node.operator_))
        return EvaluateComparison(node.operator_, lhs, rhs);

    return EvaluateArithmetic(node.operator_, lhs, rhs);
}

Value Evaluator::EvaluateLogical(BinaryOperation &node) {

    Value value1 = EvaluateExpression(node.operand1);
    if (!value1) {
        return value1;
    }

    switch (node.operator_) {

        case parse::TokenKind::AndAnd: {
            bool truthiness1 = value1.IsTruthy();
            
            if (!truthiness1) return false;

            Value value2 = EvaluateExpression(node.operand2);
            if (!value2) {
                return value2;
            }
            return value2.IsTruthy();
        }

        case parse::TokenKind::OrOr: {
            bool truthiness1 = value1.IsTruthy();
            
            if (truthiness1) return true;

            Value value2 = EvaluateExpression(node.operand2);
            if (!value2) {
                return value2;
            }
            return value2.IsTruthy();
        }

        default:
            throw std::runtime_error("Unknown operator " + parse::TokenKindToString(node.operator_));
    }
}

Value Evaluator::EvaluateArithmetic(parse::TokenKind operator_, const Value &lhs, const Value &rhs) {
    bool is_both_str = lhs.type == Type::String && rhs.type == Type::String;
    bool is_both_num = lhs.type == Type::Number && rhs.type == Type::Number;

    if (is_both_str && operator_ == parse::TokenKind::Plus) {

        std::string str1 = lhs.ToString();
        std::string str2 = rhs.ToString();

        return str1 + str2;
    }

    if (is_both_num) {

        double num1 = *lhs.GetNum();
        double num2 = *rhs.GetNum();

        switch (operator_) {
            case parse::TokenKind::Plus:
                return num1 + num2;
            case parse::TokenKind::Minus:
                return num1 - num2;
            case parse::TokenKind::Star:
                return num1 * num2;
            case parse::TokenKind::Slash:
                return num1 / num2;
            default:
                throw std::runtime_error("Unknown operator " + parse::TokenKindToString(operator_));
        }
    }

    diagnostic::ReportError("Operator " + parse::OperatorToString(operator_) + " is not supported for types '" +
                             TypeToString(lhs.type) + "' and '" + TypeToString(rhs.type) + "'.");
    return Error;
}

Value Evaluator::EvaluateComparison(parse::TokenKind operator_, const Value &lhs, const Value &rhs) {
    switch (operator_) {
        case parse::TokenKind::EqualEqual:
        case parse::TokenKind::BangEqual: {
            bool is_equal = false;

            if (lhs.type == rhs.type) {
                switch (lhs.type) {
                    case Type::Boolean:
                        is_equal = *lhs.GetBool() == *rhs.GetBool();
                        break;
                    case Type::Number:
                        is_equal = *lhs.GetNum() == *rhs.GetNum();
                        break;
                    case Type::String:
                        is_equal = *lhs.GetStr() == *rhs.GetStr();
                        break;
                    case Type::Void:
                        is_equal = true; // nothing is always nothing :P
                        break;
                    default:
                        diagnostic::ReportError("Operator " + parse::OperatorToString(operator_) +
                                                 " is not supported for types '" + TypeToString(lhs.type) +
                                                 "' and '" + TypeToString(rhs.type) + "'.");
                        return Error;
                }
            }

            return operator_ == parse::TokenKind::EqualEqual ? is_equal : !is_equal;
        }

        case parse::TokenKind::LessThan:
        case parse::TokenKind::LessEqualThan:
        case parse::TokenKind::MoreEqualThan:
        case parse::TokenKind::MoreThan: {

            if (!(lhs.type == Type::Number && rhs.type == Type::Number)) {
                diagnostic::ReportError("Operator " + parse::OperatorToString(operator_) +
                                         " is not supported for types '" + TypeToString(lhs.type) + "' and '" +
                                         TypeToString(rhs.type) + "'.");
                return Error;
            }

            double lhs_num = *lhs.GetNum();
            double rhs_num = *rhs.GetNum();

            switch (operator_) {
                case parse::TokenKind::LessThan:
                    return lhs_num < rhs_num;
                case parse::TokenKind::LessEqualThan:
                    return lhs_num <= rhs_num;
                case parse::TokenKind::MoreThan:
                    return lhs_num > rhs_num;
                case parse::TokenKind::MoreEqualThan:
                    return lhs_num >= rhs_num;
                default:
                    throw std::runtime_error("Unknown comparison operator");
            }

            return false;
        }

        default:
            throw std::runtime_error("Unknown operator " + parse::TokenKindToString(operator_));
    }
}

Value Evaluator::EvaluateIdentifier(Identifier &node) {
    Symbol *symbol = symbols.LookupAllScopes(node.name);

    if (symbol) {
        return symbol->value;
    }

    diagnostic::ReportError("Unknown name '" + node.name + "'.");
    return Error;
}