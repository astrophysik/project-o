#include "compiler/compilation-structures/type-table.h"

#include "compiler/compilation-structures/ast.h"

namespace structures {

type_table::type_table() {
    owned_types_.push_back(std::make_unique<primitive_type>(type_kind::Unit));
    unit_type_ = owned_types_.back().get();

    owned_types_.push_back(std::make_unique<primitive_type>(type_kind::Int));
    int_type_ = owned_types_.back().get();

    owned_types_.push_back(std::make_unique<primitive_type>(type_kind::Bool));
    bool_type_ = owned_types_.back().get();

    owned_types_.push_back(std::make_unique<primitive_type>(type_kind::Real));
    real_type_ = owned_types_.back().get();

    owned_types_.push_back(std::make_unique<primitive_type>(type_kind::Error));
    error_type_ = owned_types_.back().get();

    owned_types_.push_back(std::make_unique<primitive_type>(type_kind::Unknown));
    unknown_type_ = owned_types_.back().get();
}

const class_type* type_table::getClass(const std::string& name) const {
    auto it = class_types_.find(name);
    if (it != class_types_.end()) {
        return it->second;
    }
    return nullptr;
}

const class_type* type_table::addClass(const std::string& name, ast::class_declaration* decl) {
    if (class_types_.find(name) != class_types_.end()) {
        return nullptr;
    }

    auto class_type = std::make_unique<structures::class_type>(name, decl);
    const structures::class_type* ptr = class_type.get();
    owned_types_.push_back(std::move(class_type));
    class_types_[name] = ptr;
    return ptr;
}

const type* type_table::resolveType(const std::string& name) const {
    if (name == "Integer") {
        return int_type_;
    }
    if (name == "Bool") {
        return bool_type_;
    }
    if (name == "Unit") {
        return unit_type_;
    }
    if (name == "Real") {
        return real_type_;
    }

    auto it = class_types_.find(name);
    if (it != class_types_.end()) {
        return it->second;
    }

    return error_type_;
}

bool type_table::isPrimitiveTypeName(const std::string& name) {
    return name == "Integer" || name == "Bool" || name == "Void" || name == "Real";
}

bool type::isSubtype(const type* sub, const type* super) {
    if (sub == super) {
        return true;
    }
    if (sub->isError() || super->isError()) {
        return true;
    }

    if (auto* sub_cls = dynamic_cast<const class_type*>(sub)) {
        if (auto* super_cls = dynamic_cast<const class_type*>(super)) {
            ast::class_declaration* current = sub_cls->declaration;
            while (current && current->base_class) {
                if (*current->base_class == super_cls->name) {
                    return true;
                }
                break;
            }
        }
    }
    return false;
}

bool type::typesEqual(const type* t1, const type* t2) {
    return t1 == t2;
}

} // namespace structures
