cmake_minimum_required(VERSION 3.0.0)

project(sycompiler)

set(CMAKE_CXX_STANDARD 17)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

aux_source_directory(./src/ DIR_SRC)
include_directories(./include)
add_definitions(-DLEXER)
add_executable(lexer ./test/lexer.cc ${DIR_SRC})
add_executable(parser ./test/parser.cc ${DIR_SRC})
add_executable(interpreter ./test/interpreter.cc ${DIR_SRC})
add_executable(compiler ./test/compiler.cc ${DIR_SRC})
llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(lexer ${llvm_libs})
target_link_libraries(parser ${llvm_libs})
target_link_libraries(interpreter ${llvm_libs})
target_link_libraries(compiler ${llvm_libs})