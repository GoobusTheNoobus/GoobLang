#include "symbol.h"
#include <cassert>

void ScopeStack::Define(Symbol symbol) {
    assert(!data.empty());

    data.back()[symbol.name] = std::move(symbol);
}

Symbol *ScopeStack::LookupTopScope(const std::string &name) {
    assert(!data.empty());

    return Lookup(data.back(), name);
}

Symbol *ScopeStack::LookupAllScopes(const std::string &name) {

    // We search top-down
    for (auto it = data.rbegin(); it != data.rend(); ++it) {
        Symbol *lookup = Lookup(*it, name);

        if (lookup)
            return lookup;
    }

    return nullptr;
}

Symbol *ScopeStack::Lookup(std::unordered_map<std::string, Symbol> &scope, const std::string &name) {

    auto it = scope.find(name);

    if (it == scope.end()) {
        return nullptr;
    }

    return &it->second;
}

void ScopeStack::PushScope() {
    data.push_back({});
}

void ScopeStack::PopScope() {
    data.pop_back();
}