#include <filesystem>
#include <fstream>
#include <print>
#include <string>

#include "compiler/analysis/print/ast-print.h"
#include "compiler/analysis/print/codegen-ast-print.h"
#include "compiler/analysis/semantic/semantic-check.h"
#include "compiler/codegen/llvm/llvm-codegen.h"
#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::print("Usage: ./compiler <input_file>\n");
        return 1;
    }

    std::ifstream s(argv[1]);
    std::string file_content((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());

    try {
        auto tokens_res = lexer::tokenize_text(file_content);
        auto parser = parser::parser(tokens_res);
        auto parsing_ast = parser.parse();
        auto semantic_ast = analysis::semantic::check_program(parsing_ast, argv[1], file_content);

        codegen::llvm_ir::llvm_codegen ir_gen{std::string(argv[1])};
        ir_gen.emit(*semantic_ast);
        std::print("{}\n", ir_gen.ir_to_string());

        std::filesystem::path input_path(argv[1]);
        auto ir_path = input_path;
        ir_path.replace_extension(".ll");
        auto obj_path = input_path;
        obj_path.replace_extension(".o");

        if (!ir_gen.write_ir_file(ir_path.string())) {
            std::print(stderr, "Failed to write LLVM IR to {}\n", ir_path.string());
        }
        if (!ir_gen.write_object_file(obj_path.string())) {
            std::print(stderr, "Failed to write object file to {}\n", obj_path.string());
        }
    } catch (std::exception& e) {
        std::cerr << "Compilation error : \n" << e.what();
    }
}
