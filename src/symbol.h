#pragma once
#include "value.h"
#include <string>
#include <unordered_map>
#include <vector>

struct Symbol {
    std::string name;
    Value value;
};

class ScopeStack {
public:
    void Define(Symbol symbol);
    Symbol *LookupTopScope(const std::string& name);
    Symbol *LookupAllScopes(const std::string& name);

    void PushScope();
    void PopScope();

private:
    std::vector<std::unordered_map<std::string, Symbol>> data;

    Symbol *Lookup(std::unordered_map<std::string, Symbol>& scope, const std::string& name);
};