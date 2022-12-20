cmake_minimum_required(VERSION 3.14)

project(sycompiler)

set(CMAKE_CXX_STANDARD 17)

# google test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

enable_testing()

# set the flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb -Og -DDEBUG -Wall")

aux_source_directory(${CMAKE_SOURCE_DIR}/src/ DIR_SRC)
include_directories(${CMAKE_SOURCE_DIR}/include)
add_library(sycompiler_base_static STATIC ${DIR_SRC})
add_executable(lexer ./test/lexer.cc)
add_executable(parser ./test/parser.cc)
add_executable(interpreter ./test/interpreter.cc)
add_executable(compiler ./test/compiler.cc)
llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(lexer ${llvm_libs} sycompiler_base_static)
target_link_libraries(parser ${llvm_libs} sycompiler_base_static)
target_link_libraries(interpreter ${llvm_libs} sycompiler_base_static)
target_link_libraries(compiler ${llvm_libs} sycompiler_base_static)

add_subdirectory(test)
