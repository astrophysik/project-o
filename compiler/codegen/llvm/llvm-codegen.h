#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Target/TargetMachine.h>

#include "compiler/compilation-structures/ast/codegen/ast.h"
#include "compiler/compilation-structures/ast/codegen/ast-visitor.h"

namespace codegen::llvm_ir {

class llvm_codegen : public codegen::ast::visitor {
public:
    explicit llvm_codegen(const std::string& module_name, std::string entry_class_name = "Main");
    ~llvm_codegen() override = default;

    void emit(codegen::ast::program& program);

    std::string ir_to_string() const;
    bool write_ir_file(const std::string& path) const;
    bool write_object_file(const std::string& path);

    void visit(codegen::ast::program& node) override;
    void visit(codegen::ast::block& node) override;
    void visit(codegen::ast::class_declaration& node) override;
    void visit(codegen::ast::field_declaration& node) override;
    void visit(codegen::ast::variable_declaration& node) override;
    void visit(codegen::ast::parameter_declaration& node) override;
    void visit(codegen::ast::method_declaration& node) override;
    void visit(codegen::ast::constructor_declaration& node) override;
    void visit(codegen::ast::variable_assignment& node) override;
    void visit(codegen::ast::field_assignment& node) override;
    void visit(codegen::ast::while_statement& node) override;
    void visit(codegen::ast::if_statement& node) override;
    void visit(codegen::ast::return_statement& node) override;
    void visit(codegen::ast::literal_expression& node) override;
    void visit(codegen::ast::this_expression& node) override;
    void visit(codegen::ast::identifier_expression& node) override;
    void visit(codegen::ast::method_call_expression& node) override;
    void visit(codegen::ast::constructor_call_expression& node) override;
    void visit(codegen::ast::member_expression& node) override;
    void visit(codegen::ast::grouping_expression& node) override;

private:
    struct vtable_entry {
        std::string name;
        std::vector<std::string> param_type_names;
        ::llvm::Function* function;
    };

    ::llvm::LLVMContext context;
    std::unique_ptr<::llvm::Module> module;
    ::llvm::IRBuilder<> builder;
    std::unique_ptr<::llvm::TargetMachine> target_machine;
    std::string entry_class_name;

    std::unordered_map<const codegen::ast::class_declaration*, ::llvm::StructType*> class_types;
    std::unordered_map<const codegen::ast::method_declaration*, ::llvm::Function*> method_functions;
    std::unordered_map<const codegen::ast::constructor_declaration*, ::llvm::Function*> constructor_functions;
    std::unordered_map<const codegen::ast::variable_declaration*, ::llvm::AllocaInst*> variable_slots;
    std::unordered_map<const codegen::ast::parameter_declaration*, ::llvm::AllocaInst*> parameter_slots;
    std::unordered_map<const codegen::ast::class_declaration*, std::vector<vtable_entry>> vtable_entries;
    std::unordered_map<const codegen::ast::class_declaration*, ::llvm::GlobalVariable*> vtable_globals;

    ::llvm::Value* current_value = nullptr;
    ::llvm::Value* current_this = nullptr;
    ::llvm::Function* current_function = nullptr;

    ::llvm::Type* map_type(const codegen::ast::class_declaration* type);
    ::llvm::StructType* declare_class_type(codegen::ast::class_declaration& cls);
    void define_class_layout(codegen::ast::class_declaration& cls);
    void declare_method(codegen::ast::method_declaration& method);
    void declare_constructor(codegen::ast::constructor_declaration& ctor);
    void emit_method_body(codegen::ast::method_declaration& method);
    void emit_constructor_body(codegen::ast::constructor_declaration& ctor);

    void build_vtable_for(codegen::ast::class_declaration& cls);
    void emit_vtable_global(codegen::ast::class_declaration& cls);
    int method_vtable_slot(const codegen::ast::method_declaration& method) const;

    void emit_main(codegen::ast::program& program);

    ::llvm::Value* eval(codegen::ast::expression& expr);
    ::llvm::AllocaInst* create_entry_alloca(::llvm::Type* type, const std::string& name);
    int field_index(const codegen::ast::field_declaration& field) const;
    ::llvm::Value* emit_field_address(::llvm::Value* object, const codegen::ast::field_declaration& field);
    ::llvm::Function* get_or_declare_allocator();

    ::llvm::Value* emit_builtin_method(const codegen::ast::method_call_expression& call,
                                       ::llvm::Value* receiver,
                                       const std::vector<::llvm::Value*>& args);
    ::llvm::Value* emit_builtin_constructor(const codegen::ast::constructor_call_expression& call,
                                            const std::vector<::llvm::Value*>& args);

    static bool is_builtin_class(const std::string& name);
    static std::string mangle_method(const codegen::ast::method_declaration& method);
    static std::string mangle_constructor(const codegen::ast::constructor_declaration& ctor);
    static std::string param_type_name(const codegen::ast::class_declaration* type);
    static std::vector<std::string> param_type_names_of(
        const std::vector<std::unique_ptr<codegen::ast::parameter_declaration>>& params);
};

} // namespace codegen::llvm_ir
