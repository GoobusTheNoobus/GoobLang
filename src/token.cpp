#include "token.h"
#include <stdexcept>
#include <string>

namespace parse {

std::string TokenKindToString(TokenKind kind) {

    constexpr const char *MAP[] = {
        "Identifier",
        "Integer",
        "Float",
        "String",

        "Plus",
        "Minus",
        "Star",
        "Slash",

        "Equal",
        "LeftParen",
        "RightParen",

        "LeftBrace",
        "RightBrace",

        "AndAnd",
        "OrOr",
        "Bang",

        "LessThan",
        "LessEqualThan",
        "MoreThan",
        "MoreEqualThan",

        "EqualEqual",
        "BangEqual",

        "True",
        "False",
        "Let",

        "Semicolon",
        "Comma",
        "EndOfFile",
    };

    return MAP[static_cast<int>(kind)];
}

std::string OperatorToString(TokenKind kind) {
    switch (kind) {
        case TokenKind::AndAnd: return "&&";
        case TokenKind::OrOr: return "||";
        case TokenKind::Bang: return "!";
        case TokenKind::BangEqual: return "!=";
        case TokenKind::Equal: return "=";
        case TokenKind::EqualEqual: return "==";
        case TokenKind::LessEqualThan: return "<=";
        case TokenKind::LessThan: return "<";
        case TokenKind::MoreEqualThan: return ">=";
        case TokenKind::MoreThan: return ">";
        case TokenKind::Plus: return "+";
        case TokenKind::Minus: return "-";
        case TokenKind::Star: return "*";
        case TokenKind::Slash: return "/";
        default: throw std::runtime_error("Unknown operator " + TokenKindToString(kind));
    }
}

bool IsLogicalOp(TokenKind op) {
    return op == parse::TokenKind::AndAnd || op == parse::TokenKind::OrOr || op == TokenKind::Bang;
}

bool IsComparisonOp(TokenKind op) {
    return op == parse::TokenKind::EqualEqual || op == parse::TokenKind::BangEqual ||
           op == parse::TokenKind::LessThan || op == parse::TokenKind::MoreThan ||
           op == parse::TokenKind::LessEqualThan || op == parse::TokenKind::MoreEqualThan;
}

} // namespace parse