#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <expected>

#include "compiler/compilation-structures/ast-forward-declarations.h"

namespace structures {

class class_symbol;

class type;
class type_table;

enum class type_kind {
    Error,
    Unit,
    Int,
    Bool,
    Real,
    Class,
    Unknown,
};

class type {
public:
    struct infer_context {
        structures::type_table * type_table;
        structures::class_symbol * class_symbol;
    };

    const type_kind kind;

    explicit type(type_kind k) : kind(k) {}
    virtual ~type() = default;
    
    virtual std::string toString() {return  "";}

    static bool isSubtype(const type* sub, const type* super);
    static bool typesEqual(const type* t1, const type* t2);
    static std::expected<const type *, std::string> inferExpressionType(const ast::expression * expression, infer_context context);

    
    bool isError() const { return kind == type_kind::Error; }

};

class primitive_type : public type {
public:
    explicit primitive_type(type_kind k) : type(k) {}

    std::string toString() const {
        switch (kind) {
            case type_kind::Unit:
                return "Void";
            case type_kind::Int:
                return "Int";
            case type_kind::Bool:
                return "Bool";
            case type_kind::Real:
                return "Real";
            case type_kind::Error:
                return "<error>";
            default:
                return "<unknown>";
        }
    }
};

class class_type : public type {
public:
    std::string name;
    ast::class_declaration* declaration;

    class_type(std::string n, ast::class_declaration* decl)
        : type(type_kind::Class), name(std::move(n)), declaration(decl) {}

    std::string toString() const { return name; }
};

class type_table {
public:
    type_table();
    ~type_table() = default;

    type_table(const type_table&) = delete;
    type_table& operator=(const type_table&) = delete;
    type_table(type_table&&) = delete;
    type_table& operator=(type_table&&) = delete;

    const type* getUnit() const { return unit_type_; }
    const type* getInt() const { return int_type_; }
    const type* getBool() const { return bool_type_; }
    const type* getReal() const { return real_type_; }
    const type* getError() const { return error_type_; }
    const type* getUnknown() const {return unknown_type_;}
    
    const class_type* getClass(const std::string& name) const;
    
    const class_type* addClass(const std::string& name,
                                            ast::class_declaration* decl = nullptr);
    
    const type* resolveType(const std::string& name) const;
    
    static bool isPrimitiveTypeName(const std::string& name);

private:
    const type* unit_type_;
    const type* int_type_;
    const type* bool_type_;
    const type* real_type_;
    const type* error_type_;
    const type* unknown_type_;

    std::vector<std::unique_ptr<type>> owned_types_;

    std::unordered_map<std::string, const class_type*> class_types_;
};




}  // namespace structures
