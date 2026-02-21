set(COMPILER_DIR ${CMAKE_CURRENT_LIST_DIR})

set(COMPILER_MAIN
    ${COMPILER_DIR}/compiler.cpp
)

set(COMPILER_SOURCES
    ${COMPILER_MAIN}
)

add_executable(compiler ${COMPILER_SOURCES})

target_compile_options(compiler PUBLIC -stdlib=libc++)
target_compile_options(compiler PUBLIC -std=c++23)
target_link_options(compiler PUBLIC -stdlib=libc++)

target_include_directories(compiler PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/..
)
