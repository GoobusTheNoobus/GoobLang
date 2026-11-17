#include "ast.h"
#include "token.h"
#include <sstream>
#include <string>

std::string Identifier::ToString(int indent) const {
    return std::string(indent, ' ') + "Identifier(" + name + ")\n";
}

std::string String::ToString(int indent) const {
    return std::string(indent, ' ') + "String('" + value + "')\n";
}

std::string Number::ToString(int indent) const {
    return std::string(indent, ' ') + "Number(" + std::to_string(value) + ")\n";
}

std::string Boolean::ToString(int indent) const {
    return std::string(indent, ' ') + "Boolean(" + (value ? "true" : "false") +
         ")\n";
}

std::string BinaryOperation::ToString(int indent) const {
    return std::string(indent, ' ') + "BinaryOperation(" +
         parse::TokenKindToString(operator_) + ")\n" +
         operand1->ToString(indent + 2) + operand2->ToString(indent + 2);
}

std::string UnaryOperation::ToString(int indent) const {
    return std::string(indent, ' ') + "UnaryOperation(" +
         parse::TokenKindToString(operator_) + ")\n" +
         operand->ToString(indent + 2);
}

std::string VarDeclaration::ToString(int indent) const {
    return std::string(indent, ' ') + "VarDeclaration(" + name + ")\n" +
        value->ToString(indent + 2);
}

std::string VarReassignment::ToString(int indent) const {
    return std::string(indent, ' ') + "VarReassignment(" + name + ")\n" + 
        value->ToString(indent + 2);
}

std::string Block::ToString(int indent) const {
    std::ostringstream out;
    out << "Block()\n";

    for (const auto &node: code) {
        out << node->ToString(indent + 2);
    } 

    return out.str();
}

std::string Invalid::ToString(int indent) const {
    return std::string(indent, ' ') + "Invalid()\n";
}

std::ostream &operator<<(std::ostream &out, const SyntaxTree &tree) {
    for (const auto &node : tree) {
        out << node->ToString(0);
    }

    return out;
}