#include "value.h"
#include <cmath>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

std::string TypeToString(Type type) {
    switch (type) {
        case Type::Integer:
            return "int";
        case Type::Float:
            return "float";
        case Type::String:
            return "string";
        case Type::Boolean:
            return "boolean";
        case Type::Void:
            return "void";
        default:
            return "<error>";
    }
}

std::string Value::ToString() const {
    switch (type) {
        case Type::Float: {
            double num = *GetFloat();
            if (std::isnan(num)) 
                return "NaN";

            if (std::isinf(num))
                return num > 0 ? "Inf" : "-Inf";

            return std::to_string(num);
        }

        case Type::Integer: {
            return std::to_string(*GetInteger());
        }

        case Type::String: {
            return *GetStr();
        }

        case Type::Boolean: {
            return *GetBool() ? "true" : "false";
        }

        default: {
            throw std::runtime_error("Unprintable type id=" + std::to_string(static_cast<int>(type)));
        }
    }
}

bool Value::IsTruthy() const {
    switch (type) {
        case Type::Float: {
            double num = *GetFloat();

            return num != 0.0 && !std::isnan(num);
        }

        case Type::Integer: {
            int32_t num = *GetInteger();
            return num != 0;
        }

        case Type::String: {
            return !GetStr()->empty();
        }

        case Type::Boolean: {
            return *GetBool();
        }

        case Type::Void: {
            return false; // void is always false
        }

        default: {
            throw std::runtime_error("Unknown truthiness type id=" + std::to_string(static_cast<int>(type)));
        }
    }
}

std::optional<double> Value::GetFloat() const {
    if (type != Type::Float)
        return std::nullopt;

    const double *pfloat = std::get_if<double>(&data);

    if (!pfloat) {
        throw std::runtime_error("Value type does not match variant value. The "
                                 "type is float, but the variant isnt");
    }

    return *pfloat;
}

std::optional<int32_t> Value::GetInteger() const {
    if (type != Type::Integer)
        return std::nullopt;

    const int32_t *pint = std::get_if<int32_t>(&data);
    
    if (!pint) {
        throw std::runtime_error("Value type does not match variant value. The "
                                 "type is int, but the variant isnt");
    }

    return *pint;
}

std::optional<std::string> Value::GetStr() const {
    if (type != Type::String)
        return std::nullopt;

    const std::string *pstr = std::get_if<std::string>(&data);

    if (!pstr) {
        throw std::runtime_error("Value type does not match variant value. The "
                                 "type is string, but the variant isnt");
    }

    return *pstr;
}

std::optional<bool> Value::GetBool() const {
    if (type != Type::Boolean)
        return std::nullopt;

    const bool *pbool = std::get_if<bool>(&data);

    if (!pbool) {
        throw std::runtime_error("Value type does not match variant value. The "
                                 "type is boolean, but the variant isnt");
    }

    return *pbool;
}