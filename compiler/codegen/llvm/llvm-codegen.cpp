#include "compiler/codegen/llvm/llvm-codegen.h"

#include <stdexcept>
#include <string>
#include <vector>

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include "compiler/common/variant-helper.h"

namespace codegen::llvm_ir {

bool llvm_codegen::is_builtin_class(const std::string& name) {
    return name == "Class" || name == "AnyValue" || name == "AnyRef"
        || name == "Integer" || name == "Real" || name == "Boolean" || name == "Unit";
}

llvm_codegen::llvm_codegen(const std::string& module_name, std::string entry_class_name)
    : module(std::make_unique<::llvm::Module>(module_name, context)),
      builder(context),
      entry_class_name(std::move(entry_class_name)) {
    // this ctor body only for setting default layout for target platform
    ::llvm::InitializeNativeTarget();
    ::llvm::InitializeNativeTargetAsmPrinter();
    ::llvm::InitializeNativeTargetAsmParser();

    auto triple = ::llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(triple);

    std::string err;
    const auto* target = ::llvm::TargetRegistry::lookupTarget(triple, err);
    if (!target) {
        throw std::runtime_error("failed to lookup target: " + err);
    }
    ::llvm::TargetOptions opt;
    target_machine.reset(target->createTargetMachine(triple, "generic", "", opt, std::nullopt));
    module->setDataLayout(target_machine->createDataLayout());
}

void llvm_codegen::emit(codegen::ast::program& program) {
    program.accept(*this);
}

std::string llvm_codegen::ir_to_string() const {
    std::string out;
    ::llvm::raw_string_ostream os(out);
    module->print(os, nullptr);
    return os.str();
}

bool llvm_codegen::write_ir_file(const std::string& path) const {
    std::error_code ec;
    ::llvm::raw_fd_ostream out(path, ec, ::llvm::sys::fs::OF_Text);
    if (ec) {
        return false;
    }
    module->print(out, nullptr);
    return true;
}

bool llvm_codegen::write_object_file(const std::string& path) {
    if (!target_machine) {
        return false;
    }
    std::error_code ec;
    ::llvm::raw_fd_ostream out(path, ec, ::llvm::sys::fs::OF_None);
    if (ec) {
        return false;
    }
    ::llvm::legacy::PassManager pm;
    if (target_machine->addPassesToEmitFile(pm, out, nullptr, ::llvm::CodeGenFileType::ObjectFile)) {
        return false;
    }
    pm.run(*module);
    out.flush();
    return true;
}

::llvm::Type* llvm_codegen::map_type(const codegen::ast::class_declaration* type) {
    if (!type) {
        return ::llvm::Type::getVoidTy(context);
    }
    if (type->name == "Integer") {
        return ::llvm::Type::getInt64Ty(context);
    }
    if (type->name == "Real") {
        return ::llvm::Type::getDoubleTy(context);
    }
    if (type->name == "Boolean") {
        return ::llvm::Type::getInt1Ty(context);
    }
    if (type->name == "Unit") {
        return ::llvm::Type::getVoidTy(context);
    }
    auto it = class_types.find(type);
    if (it == class_types.end()) {
        return ::llvm::PointerType::get(context, 0);
    }
    return ::llvm::PointerType::get(it->second, 0);
}

::llvm::StructType* llvm_codegen::declare_class_type(codegen::ast::class_declaration& cls) {
    if (auto it = class_types.find(&cls); it != class_types.end()) {
        return it->second;
    }
    auto* st = ::llvm::StructType::create(context, "class." + cls.name);
    class_types[&cls] = st;
    return st;
}

void llvm_codegen::define_class_layout(codegen::ast::class_declaration& cls) {
    if (is_builtin_class(cls.name)) {
        return;
    }
    auto* st = class_types.at(&cls);
    if (!st->isOpaque()) {
        return;
    }

    auto* ptr_ty = ::llvm::PointerType::get(context, 0);
    std::vector<::llvm::Type*> field_types;
    if (cls.base_class && !is_builtin_class(cls.base_class->name)) {
        field_types.push_back(class_types.at(cls.base_class));
    } else {
        field_types.push_back(ptr_ty);
    }
    for (auto& field : cls.fields) {
        field_types.push_back(map_type(field->type));
    }
    st->setBody(field_types, false);
}

std::string llvm_codegen::param_type_name(const codegen::ast::class_declaration* type) {
    return type ? type->name : "void";
}

std::vector<std::string> llvm_codegen::param_type_names_of(
    const std::vector<std::unique_ptr<codegen::ast::parameter_declaration>>& params) {
    std::vector<std::string> names;
    names.reserve(params.size());
    for (auto& p : params) {
        names.push_back(param_type_name(p->type));
    }
    return names;
}

std::string llvm_codegen::mangle_method(const codegen::ast::method_declaration& method) {
    std::string out = method.class_owner->name + "_" + method.name;
    for (auto& p : method.parameters) {
        out += "_" + param_type_name(p->type);
    }
    return out;
}

std::string llvm_codegen::mangle_constructor(const codegen::ast::constructor_declaration& ctor) {
    std::string out = ctor.class_owner->name + "_ctor";
    for (auto& p : ctor.parameters) {
        out += "_" + param_type_name(p->type);
    }
    return out;
}

void llvm_codegen::declare_method(codegen::ast::method_declaration& method) {
    std::vector<::llvm::Type*> param_types;
    param_types.push_back(::llvm::PointerType::get(context, 0));
    for (auto& p : method.parameters) {
        param_types.push_back(map_type(p->type));
    }
    auto* fn_type = ::llvm::FunctionType::get(map_type(method.return_type), param_types, false);
    auto* fn = ::llvm::Function::Create(fn_type, ::llvm::Function::ExternalLinkage, mangle_method(method), module.get());

    fn->arg_begin()->setName("this");
    auto arg_it = std::next(fn->arg_begin());
    for (auto& p : method.parameters) {
        arg_it->setName(p->name);
        ++arg_it;
    }
    method_functions[&method] = fn;
}

void llvm_codegen::declare_constructor(codegen::ast::constructor_declaration& ctor) {
    std::vector<::llvm::Type*> param_types;
    param_types.push_back(::llvm::PointerType::get(context, 0));
    for (auto& p : ctor.parameters) {
        param_types.push_back(map_type(p->type));
    }
    auto* fn_type = ::llvm::FunctionType::get(::llvm::Type::getVoidTy(context), param_types, false);
    auto* fn = ::llvm::Function::Create(fn_type, ::llvm::Function::ExternalLinkage, mangle_constructor(ctor), module.get());

    fn->arg_begin()->setName("this");
    auto arg_it = std::next(fn->arg_begin());
    for (auto& p : ctor.parameters) {
        arg_it->setName(p->name);
        ++arg_it;
    }
    constructor_functions[&ctor] = fn;
}

void llvm_codegen::build_vtable_for(codegen::ast::class_declaration& cls) {
    if (vtable_entries.count(&cls)) {
        return;
    }
    std::vector<vtable_entry> entries;
    if (cls.base_class && !is_builtin_class(cls.base_class->name)) {
        build_vtable_for(*cls.base_class);
        entries = vtable_entries.at(cls.base_class);
    }
    for (auto& m : cls.methods) {
        auto params = param_type_names_of(m->parameters);
        auto* fn = method_functions.at(m.get());
        bool overridden = false;
        for (auto& e : entries) {
            if (e.name == m->name && e.param_type_names == params) {
                e.function = fn;
                overridden = true;
                break;
            }
        }
        if (!overridden) {
            entries.push_back({m->name, std::move(params), fn});
        }
    }
    vtable_entries[&cls] = std::move(entries);
}

void llvm_codegen::emit_vtable_global(codegen::ast::class_declaration& cls) {
    const auto& entries = vtable_entries.at(&cls);
    if (entries.empty()) {
        return;
    }
    auto* ptr_ty = ::llvm::PointerType::get(context, 0);
    auto* arr_ty = ::llvm::ArrayType::get(ptr_ty, entries.size());
    std::vector<::llvm::Constant*> elements;
    elements.reserve(entries.size());
    for (auto& e : entries) {
        elements.push_back(e.function);
    }
    auto* init = ::llvm::ConstantArray::get(arr_ty, elements);
    auto* gv = new ::llvm::GlobalVariable(
        *module, arr_ty, true, ::llvm::GlobalValue::PrivateLinkage, init, "vtable." + cls.name);
    vtable_globals[&cls] = gv;
}

int llvm_codegen::method_vtable_slot(const codegen::ast::method_declaration& method) const {
    const auto& entries = vtable_entries.at(method.class_owner);
    auto params = param_type_names_of(method.parameters);
    for (size_t i = 0; i < entries.size(); ++i) {
        if (entries[i].name == method.name && entries[i].param_type_names == params) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

::llvm::AllocaInst* llvm_codegen::create_entry_alloca(::llvm::Type* type, const std::string& name) {
    auto& entry = current_function->getEntryBlock();
    ::llvm::IRBuilder<> tmp_builder(&entry, entry.begin());
    return tmp_builder.CreateAlloca(type, nullptr, name);
}

int llvm_codegen::field_index(const codegen::ast::field_declaration& field) const {
    auto* owner = field.class_owner;
    int base_offset = 1; // offset because of vtable
    for (size_t i = 0; i < owner->fields.size(); ++i) {
        if (owner->fields[i].get() == &field) {
            return base_offset + static_cast<int>(i);
        }
    }
    return -1;
}

::llvm::Value* llvm_codegen::emit_field_address(::llvm::Value* object, const codegen::ast::field_declaration& field) {
    auto* owner_struct = class_types.at(field.class_owner);
    return builder.CreateStructGEP(owner_struct, object, field_index(field), "field." + field.name);
}

::llvm::Function* llvm_codegen::get_or_declare_allocator() {
    if (auto* f = module->getFunction("GC_malloc")) {
        return f;
    }
    auto* fn_type = ::llvm::FunctionType::get(::llvm::PointerType::get(context, 0), {::llvm::Type::getInt64Ty(context)}, false);
    return ::llvm::Function::Create(fn_type, ::llvm::Function::ExternalLinkage, "GC_malloc", module.get());
}

::llvm::Value* llvm_codegen::eval(codegen::ast::expression& expr) {
    current_value = nullptr;
    expr.accept(*this);
    return current_value;
}

void llvm_codegen::emit_method_body(codegen::ast::method_declaration& method) {
    if (!method.body.has_value()) {
        return;
    }
    auto* fn = method_functions.at(&method);
    auto* entry = ::llvm::BasicBlock::Create(context, "entry", fn);
    builder.SetInsertPoint(entry);

    current_function = fn;
    current_this = fn->getArg(0);
    parameter_slots.clear();
    variable_slots.clear();

    auto arg_it = std::next(fn->arg_begin());
    for (auto& p : method.parameters) {
        auto* slot = create_entry_alloca(map_type(p->type), p->name);
        builder.CreateStore(&*arg_it, slot);
        parameter_slots[p.get()] = slot;
        ++arg_it;
    }

    auto* ret_ty = current_function->getReturnType();
    std::visit(overloaded{
                   [this](std::unique_ptr<codegen::ast::block>& b) { b->accept(*this); },
                   [this, ret_ty](std::unique_ptr<codegen::ast::expression>& e) {
                       auto* value = eval(*e);
                       if (ret_ty->isVoidTy()) {
                           builder.CreateRetVoid();
                       } else {
                           builder.CreateRet(value);
                       }
                   }},
               *method.body);

    if (!builder.GetInsertBlock()->getTerminator()) {
        if (ret_ty->isVoidTy()) {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(::llvm::Constant::getNullValue(ret_ty));
        }
    }

    current_function = nullptr;
    current_this = nullptr;
}

void llvm_codegen::emit_constructor_body(codegen::ast::constructor_declaration& ctor) {
    auto* fn = constructor_functions.at(&ctor);
    auto* entry = ::llvm::BasicBlock::Create(context, "entry", fn);
    builder.SetInsertPoint(entry);

    current_function = fn;
    current_this = fn->getArg(0);
    parameter_slots.clear();
    variable_slots.clear();

    if (auto it = vtable_globals.find(ctor.class_owner); it != vtable_globals.end()) {
        builder.CreateStore(it->second, current_this);
    }

    auto arg_it = std::next(fn->arg_begin());
    for (auto& p : ctor.parameters) {
        auto* slot = create_entry_alloca(map_type(p->type), p->name);
        builder.CreateStore(&*arg_it, slot);
        parameter_slots[p.get()] = slot;
        ++arg_it;
    }

    for (auto& field : ctor.class_owner->fields) {
        if (!field->initializer) {
            continue;
        }
        auto* value = eval(*field->initializer);
        auto* addr = emit_field_address(current_this, *field);
        builder.CreateStore(value, addr);
    }

    if (ctor.body) {
        ctor.body->accept(*this);
    }

    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateRetVoid();
    }

    current_function = nullptr;
    current_this = nullptr;
}

void llvm_codegen::emit_main(codegen::ast::program& program) {
    codegen::ast::class_declaration* entry_cls = nullptr;
    for (auto& cls : program.classes) {
        if (cls->name == entry_class_name) {
            entry_cls = cls.get();
            break;
        }
    }
    if (!entry_cls) {
        return;
    }
    codegen::ast::constructor_declaration* ctor = nullptr;
    for (auto& c : entry_cls->constructors) {
        if (c->parameters.empty()) {
            ctor = c.get();
            break;
        }
    }
    if (!ctor) {
        return;
    }

    auto* i32 = ::llvm::Type::getInt32Ty(context);
    auto* main_ty = ::llvm::FunctionType::get(i32, false);
    auto* main_fn = ::llvm::Function::Create(main_ty, ::llvm::Function::ExternalLinkage, "main", module.get());
    auto* bb = ::llvm::BasicBlock::Create(context, "entry", main_fn);
    builder.SetInsertPoint(bb);

    auto* struct_ty = class_types.at(entry_cls);
    auto* size = ::llvm::ConstantExpr::getSizeOf(struct_ty);
    auto* obj = builder.CreateCall(get_or_declare_allocator(), {size}, "main.obj");
    builder.CreateCall(constructor_functions.at(ctor), {obj});
    builder.CreateRet(::llvm::ConstantInt::get(i32, 0));
}

void llvm_codegen::visit(codegen::ast::program& node) {
    for (auto& cls : node.internal_classes) {
        declare_class_type(*cls);
    }
    for (auto& cls : node.classes) {
        declare_class_type(*cls);
    }
    for (auto& cls : node.classes) {
        define_class_layout(*cls);
    }

    for (auto& cls : node.classes) {
        for (auto& method : cls->methods) {
            declare_method(*method);
        }
        for (auto& ctor : cls->constructors) {
            declare_constructor(*ctor);
        }
    }

    for (auto& cls : node.classes) {
        build_vtable_for(*cls);
    }
    for (auto& cls : node.classes) {
        emit_vtable_global(*cls);
    }

    for (auto& cls : node.classes) {
        for (auto& method : cls->methods) {
            emit_method_body(*method);
        }
        for (auto& ctor : cls->constructors) {
            emit_constructor_body(*ctor);
        }
    }

    emit_main(node);
}

void llvm_codegen::visit(codegen::ast::block& node) {
    for (auto& item : node.items) {
        item->accept(*this);
        if (builder.GetInsertBlock() && builder.GetInsertBlock()->getTerminator()) {
            break;
        }
    }
}

void llvm_codegen::visit(codegen::ast::class_declaration&) {}
void llvm_codegen::visit(codegen::ast::field_declaration&) {}
void llvm_codegen::visit(codegen::ast::method_declaration&) {}
void llvm_codegen::visit(codegen::ast::constructor_declaration&) {}
void llvm_codegen::visit(codegen::ast::parameter_declaration&) {}

void llvm_codegen::visit(codegen::ast::variable_declaration& node) {
    auto* var_ty = map_type(node.type);
    if (var_ty->isVoidTy()) {
        if (node.initializer) {
            eval(*node.initializer);
        }
        return;
    }
    auto* slot = create_entry_alloca(var_ty, node.name);
    variable_slots[&node] = slot;
    if (node.initializer) {
        auto* value = eval(*node.initializer);
        if (value) {
            builder.CreateStore(value, slot);
        }
    }
}

void llvm_codegen::visit(codegen::ast::variable_assignment& node) {
    auto* value = eval(*node.value);
    ::llvm::Value* slot = std::visit(overloaded{
                                         [this](codegen::ast::variable_declaration* d) -> ::llvm::Value* { return variable_slots.at(d); },
                                         [this](codegen::ast::parameter_declaration* d) -> ::llvm::Value* { return parameter_slots.at(d); },
                                         [this](codegen::ast::field_declaration* d) -> ::llvm::Value* { return emit_field_address(current_this, *d); }},
                                     node.target);
    builder.CreateStore(value, slot);
}

void llvm_codegen::visit(codegen::ast::field_assignment& node) {
    auto* value = eval(*node.value);
    auto* object = eval(*node.target->object);
    auto* addr = emit_field_address(object, *node.target->member);
    builder.CreateStore(value, addr);
}

void llvm_codegen::visit(codegen::ast::while_statement& node) {
    auto* cond_block = ::llvm::BasicBlock::Create(context, "while.cond", current_function);
    auto* body_block = ::llvm::BasicBlock::Create(context, "while.body", current_function);
    auto* end_block = ::llvm::BasicBlock::Create(context, "while.end", current_function);

    builder.CreateBr(cond_block);
    builder.SetInsertPoint(cond_block);
    auto* cond = eval(*node.condition);
    builder.CreateCondBr(cond, body_block, end_block);

    builder.SetInsertPoint(body_block);
    node.body->accept(*this);
    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateBr(cond_block);
    }

    builder.SetInsertPoint(end_block);
}

void llvm_codegen::visit(codegen::ast::if_statement& node) {
    auto* cond = eval(*node.condition);
    auto* then_block = ::llvm::BasicBlock::Create(context, "if.then", current_function);
    auto* else_block = ::llvm::BasicBlock::Create(context, "if.else", current_function);
    auto* end_block = ::llvm::BasicBlock::Create(context, "if.end", current_function);

    builder.CreateCondBr(cond, then_block, node.false_branch ? else_block : end_block);

    builder.SetInsertPoint(then_block);
    node.true_branch->accept(*this);
    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateBr(end_block);
    }

    builder.SetInsertPoint(else_block);
    if (node.false_branch) {
        node.false_branch->accept(*this);
        if (!builder.GetInsertBlock()->getTerminator()) {
            builder.CreateBr(end_block);
        }
    } else {
        builder.CreateBr(end_block);
    }

    builder.SetInsertPoint(end_block);
}

void llvm_codegen::visit(codegen::ast::return_statement& node) {
    if (current_function->getReturnType()->isVoidTy()) {
        if (node.value) {
            eval(*node.value);
        }
        builder.CreateRetVoid();
        return;
    }
    if (node.value) {
        auto* value = eval(*node.value);
        builder.CreateRet(value);
    } else {
        builder.CreateRetVoid();
    }
}

void llvm_codegen::visit(codegen::ast::literal_expression& node) {
    current_value = std::visit(overloaded{
                                   [this](int64_t v) -> ::llvm::Value* { return ::llvm::ConstantInt::get(::llvm::Type::getInt64Ty(context), v, true); },
                                   [this](double v) -> ::llvm::Value* { return ::llvm::ConstantFP::get(::llvm::Type::getDoubleTy(context), v); },
                                   [this](bool v) -> ::llvm::Value* { return ::llvm::ConstantInt::get(::llvm::Type::getInt1Ty(context), v ? 1 : 0); }},
                               node.value);
}

void llvm_codegen::visit(codegen::ast::this_expression&) {
    current_value = current_this;
}

void llvm_codegen::visit(codegen::ast::identifier_expression& node) {
    current_value = std::visit(overloaded{
                                   [this](codegen::ast::variable_declaration* d) -> ::llvm::Value* {
                                       auto* slot = variable_slots.at(d);
                                       return builder.CreateLoad(map_type(d->type), slot, d->name);
                                   },
                                   [this](codegen::ast::parameter_declaration* d) -> ::llvm::Value* {
                                       auto* slot = parameter_slots.at(d);
                                       return builder.CreateLoad(map_type(d->type), slot, d->name);
                                   },
                                   [this](codegen::ast::field_declaration* d) -> ::llvm::Value* {
                                       auto* addr = emit_field_address(current_this, *d);
                                       return builder.CreateLoad(map_type(d->type), addr, d->name);
                                   }},
                               node.target);
}

void llvm_codegen::visit(codegen::ast::member_expression& node) {
    auto* object = eval(*node.object);
    auto* addr = emit_field_address(object, *node.member);
    current_value = builder.CreateLoad(map_type(node.member->type), addr, node.member->name);
}

void llvm_codegen::visit(codegen::ast::grouping_expression& node) {
    current_value = eval(*node.inner);
}

void llvm_codegen::visit(codegen::ast::method_call_expression& node) {
    auto* receiver = eval(*node.object);
    std::vector<::llvm::Value*> args;
    args.reserve(node.arguments.size());
    for (auto& arg : node.arguments) {
        args.push_back(eval(*arg));
    }

    if (is_builtin_class(node.method->class_owner->name)) {
        current_value = emit_builtin_method(node, receiver, args);
        return;
    }

    std::vector<::llvm::Value*> call_args;
    call_args.reserve(args.size() + 1);
    call_args.push_back(receiver);
    for (auto* a : args) {
        call_args.push_back(a);
    }

    auto* ptr_ty = ::llvm::PointerType::get(context, 0);
    auto* vtable = builder.CreateLoad(ptr_ty, receiver, "vtable");
    int slot = method_vtable_slot(*node.method);
    auto* slot_ptr = builder.CreateInBoundsGEP(
        ptr_ty, vtable, ::llvm::ConstantInt::get(::llvm::Type::getInt32Ty(context), slot), "vslot");
    auto* fn_ptr = builder.CreateLoad(ptr_ty, slot_ptr, "vfn");
    auto* fn_type = method_functions.at(node.method)->getFunctionType();
    current_value = builder.CreateCall(fn_type, fn_ptr, call_args);
}

void llvm_codegen::visit(codegen::ast::constructor_call_expression& node) {
    std::vector<::llvm::Value*> args;
    args.reserve(node.arguments.size());
    for (auto& arg : node.arguments) {
        args.push_back(eval(*arg));
    }

    if (is_builtin_class(node.constructor->class_owner->name)) {
        current_value = emit_builtin_constructor(node, args);
        return;
    }

    auto* struct_ty = class_types.at(node.constructor->class_owner);
    auto* size = ::llvm::ConstantExpr::getSizeOf(struct_ty);
    auto* obj = builder.CreateCall(get_or_declare_allocator(), {size}, "obj");

    std::vector<::llvm::Value*> call_args;
    call_args.reserve(args.size() + 1);
    call_args.push_back(obj);
    for (auto* a : args) {
        call_args.push_back(a);
    }
    builder.CreateCall(constructor_functions.at(node.constructor), call_args);
    current_value = obj;
}

::llvm::Value* llvm_codegen::emit_builtin_constructor(const codegen::ast::constructor_call_expression& node,
                                                     const std::vector<::llvm::Value*>& args) {
    const auto& cls_name = node.constructor->class_owner->name;
    if (cls_name == "Unit") {
        return nullptr;
    }
    if (args.empty()) {
        return ::llvm::Constant::getNullValue(map_type(node.constructor->class_owner));
    }
    auto* value = args[0];
    const auto& src_name = node.constructor->parameters[0]->type->name;

    if (cls_name == "Integer" && src_name == "Real") {
        return builder.CreateFPToSI(value, ::llvm::Type::getInt64Ty(context), "to.int");
    }
    if (cls_name == "Real" && src_name == "Integer") {
        return builder.CreateSIToFP(value, ::llvm::Type::getDoubleTy(context), "to.real");
    }
    return value;
}

::llvm::Value* llvm_codegen::emit_builtin_method(const codegen::ast::method_call_expression& node,
                                                 ::llvm::Value* receiver,
                                                 const std::vector<::llvm::Value*>& args) {
    const auto& cls = node.method->class_owner->name;
    const auto& name = node.method->name;
    auto* i64 = ::llvm::Type::getInt64Ty(context);
    auto* f64 = ::llvm::Type::getDoubleTy(context);
    auto* i1 = ::llvm::Type::getInt1Ty(context);

    auto to_real = [&](::llvm::Value* v) { return v->getType()->isIntegerTy() ? builder.CreateSIToFP(v, f64) : v; };

    bool param_is_real = !node.method->parameters.empty() && node.method->parameters[0]->type->name == "Real";
    bool returns_real = node.method->return_type && node.method->return_type->name == "Real";
    bool uses_fp = (cls == "Real") || param_is_real || returns_real;

    if (cls == "Integer" || cls == "Real") {
        if (name == "UnaryMinus") {
            return uses_fp ? builder.CreateFNeg(to_real(receiver)) : builder.CreateNeg(receiver);
        }
        if (name == "toReal") {
            return builder.CreateSIToFP(receiver, f64);
        }
        if (name == "toInteger") {
            return builder.CreateFPToSI(receiver, i64);
        }
        if (name == "toBoolean") {
            return builder.CreateICmpNE(receiver, ::llvm::ConstantInt::get(i64, 0));
        }

        ::llvm::Value* lhs = receiver;
        ::llvm::Value* rhs = args[0];
        if (uses_fp) {
            lhs = to_real(lhs);
            rhs = to_real(rhs);
        }
        if (name == "Plus") {
            return uses_fp ? builder.CreateFAdd(lhs, rhs) : builder.CreateAdd(lhs, rhs);
        }
        if (name == "Minus") {
            return uses_fp ? builder.CreateFSub(lhs, rhs) : builder.CreateSub(lhs, rhs);
        }
        if (name == "Mult") {
            return uses_fp ? builder.CreateFMul(lhs, rhs) : builder.CreateMul(lhs, rhs);
        }
        if (name == "Div") {
            return uses_fp ? builder.CreateFDiv(lhs, rhs) : builder.CreateSDiv(lhs, rhs);
        }
        if (name == "Rem") {
            return uses_fp ? builder.CreateFRem(lhs, rhs) : builder.CreateSRem(lhs, rhs);
        }
        if (name == "Less") {
            return uses_fp ? builder.CreateFCmpOLT(lhs, rhs) : builder.CreateICmpSLT(lhs, rhs);
        }
        if (name == "LessEqual") {
            return uses_fp ? builder.CreateFCmpOLE(lhs, rhs) : builder.CreateICmpSLE(lhs, rhs);
        }
        if (name == "Greater") {
            return uses_fp ? builder.CreateFCmpOGT(lhs, rhs) : builder.CreateICmpSGT(lhs, rhs);
        }
        if (name == "GreaterEqual") {
            return uses_fp ? builder.CreateFCmpOGE(lhs, rhs) : builder.CreateICmpSGE(lhs, rhs);
        }
        if (name == "Equal") {
            return uses_fp ? builder.CreateFCmpOEQ(lhs, rhs) : builder.CreateICmpEQ(lhs, rhs);
        }
    }

    if (cls == "Boolean") {
        if (name == "Not") {
            return builder.CreateXor(receiver, ::llvm::ConstantInt::get(i1, 1));
        }
        if (name == "toInteger") {
            return builder.CreateZExt(receiver, i64);
        }
        if (name == "And") {
            return builder.CreateAnd(receiver, args[0]);
        }
        if (name == "Or") {
            return builder.CreateOr(receiver, args[0]);
        }
        if (name == "Xor") {
            return builder.CreateXor(receiver, args[0]);
        }
    }

    return ::llvm::Constant::getNullValue(map_type(node.method->return_type));
}

} // namespace codegen::llvm_ir
