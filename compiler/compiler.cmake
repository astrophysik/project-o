set(COMPILER_DIR ${CMAKE_CURRENT_LIST_DIR})

if(NOT TARGET zstd::libzstd_shared)
    find_library(ZSTD_LIBRARY NAMES zstd)
    if(ZSTD_LIBRARY)
        add_library(zstd::libzstd_shared SHARED IMPORTED)
        set_target_properties(zstd::libzstd_shared PROPERTIES
                IMPORTED_LOCATION ${ZSTD_LIBRARY})
    endif()
endif()

find_package(LLVM REQUIRED CONFIG)

set(COMPILER_ANALYSIS
        ${COMPILER_DIR}/analysis/print/details/ast-printer.cpp
        ${COMPILER_DIR}/analysis/print/details/codegen-ast-printer.cpp
        ${COMPILER_DIR}/analysis/semantic/error.cpp
        ${COMPILER_DIR}/analysis/semantic/phases/class-collector.cpp
        ${COMPILER_DIR}/analysis/semantic/phases/class-body-collector.cpp
        ${COMPILER_DIR}/analysis/semantic/phases/class-field-checker.cpp
        ${COMPILER_DIR}/analysis/semantic/phases/class-method-checker.cpp
        ${COMPILER_DIR}/analysis/semantic/phases/codegen-ast-collector.cpp
)

set(COMPILER_MAIN
        ${COMPILER_DIR}/compiler.cpp
)

set(COMPILER_COMPILATION_STRUCTURES
        ${COMPILER_DIR}/compilation-structures/type-table.cpp
        ${COMPILER_DIR}/compilation-structures/ast/parsing/ast.cpp
        ${COMPILER_DIR}/compilation-structures/ast/codegen/ast.cpp
)

set(COMPILER_PARSER
        ${COMPILER_DIR}/parser/parser.cpp
)

set(COMPILER_CODEGEN
        ${COMPILER_DIR}/codegen/llvm/llvm-codegen.cpp
)

set(COMPILER_SOURCES
        ${COMPILER_MAIN}
        ${COMPILER_PARSER}
        ${COMPILER_AST}
        ${COMPILER_COMPILATION_STRUCTURES}
        ${COMPILER_ANALYSIS}
        ${COMPILER_CODEGEN}
)

add_executable(compiler ${COMPILER_SOURCES})

target_compile_options(compiler PUBLIC -std=c++23)

target_include_directories(compiler PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/..
        ${LLVM_INCLUDE_DIRS}
)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(compiler PRIVATE ${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(LLVM_LIBS core support irreader native nativecodegen)
target_link_libraries(compiler PRIVATE ${LLVM_LIBS})
