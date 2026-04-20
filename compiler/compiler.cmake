set(COMPILER_DIR ${CMAKE_CURRENT_LIST_DIR})

set(COMPILER_ANALYSIS
        ${COMPILER_DIR}/analysis/print/details/ast-printer.cpp
        ${COMPILER_DIR}/analysis/semantic/phases/class-collector.cpp
        ${COMPILER_DIR}/analysis/semantic/phases/class-body-collector.cpp
        ${COMPILER_DIR}/analysis/semantic/phases/class-content-checker.cpp
)

set(COMPILER_MAIN
        ${COMPILER_DIR}/compiler.cpp
)

set(COMPILER_COMPILATION_STRUCTURES
        ${COMPILER_DIR}/compilation-structures/type-table.cpp
        ${COMPILER_DIR}/compilation-structures/ast.cpp
)

set(COMPILER_PARSER
        ${COMPILER_DIR}/parser/parser.cpp
)

set(COMPILER_SOURCES
        ${COMPILER_MAIN}
        ${COMPILER_PARSER}
        ${COMPILER_AST}
        ${COMPILER_COMPILATION_STRUCTURES}
        ${COMPILER_ANALYSIS}
)

add_executable(compiler ${COMPILER_SOURCES})

target_compile_options(compiler PUBLIC -stdlib=libc++)
target_compile_options(compiler PUBLIC -std=c++23)
target_link_options(compiler PUBLIC -stdlib=libc++)

target_include_directories(compiler PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/..
)
