#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "compiler/compilation-structures/type-table.h"

namespace structures {

enum class symbol_kind {
    class_symbol,
    method_symbol,
    variable_symbol,
};

struct symbol {
    // todo add span
    std::string name;
    symbol_kind kind;

    symbol(std::string n, symbol_kind k)
        : name(std::move(n)),
          kind(k) {}

    virtual ~symbol() = default;
};

struct symbol_table {
    symbol_table(symbol_table* p)
        : parent(p){};

    symbol_table(const symbol_table&) = delete;
    symbol_table& operator=(const symbol_table&) = delete;
    symbol_table(symbol_table&&) noexcept = default;
    symbol_table& operator=(symbol_table&&) noexcept = default;

    void add(std::unique_ptr<symbol> symbol) {
        symbols[symbol->name] = std::move(symbol);
    }

    symbol* lookup(const std::string& name) const {
        if (auto it = symbols.find(name); it != symbols.end()) {
            return it->second.get();
        }
        if (parent != nullptr) {
            return parent->lookup(name);
        }
        return nullptr;
    }

    template<typename T>
    T* typed_lookup(const std::string& name) const {
        return dynamic_cast<T*>(lookup(name));
    }

    auto begin() noexcept {
        return symbols.begin();
    }

    auto end() noexcept {
        return symbols.end();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<symbol>> symbols{};
    symbol_table* parent;
};

struct variable_symbol : symbol {
    const structures::type* type;

    variable_symbol(std::string name, const structures::type* type)
        : symbol(std::move(name), symbol_kind::variable_symbol),
          type(type) {}
};

struct method_symbol : symbol {
    std::optional<const structures::type *> return_type;
    std::vector<const structures::type *> parameter_types;
    std::unique_ptr<symbol_table> method_scope;

    method_symbol(std::string name, symbol_table* parent_scope, std::optional<const structures::type *> ret_type, std::vector<const structures::type *> params_type)
        : symbol(std::move(name), symbol_kind::method_symbol),
          method_scope(std::make_unique<symbol_table>(parent_scope)),
          return_type(std::move(ret_type)),
          parameter_types(std::move(params_type)) {}
};

struct class_symbol : symbol {
    class_symbol* base_class = nullptr;
    std::unique_ptr<symbol_table> class_scope;
    std::vector<method_symbol *> methods;
    std::vector<variable_symbol *> fields;
    std::vector<std::unique_ptr<method_symbol>> constructors;

    class_symbol(std::string name, symbol_table* parent_scope, class_symbol* base_class)
        : symbol(std::move(name), symbol_kind::class_symbol),
          class_scope(std::make_unique<symbol_table>(parent_scope)),
          base_class(base_class) {}
};

} // namespace structures
