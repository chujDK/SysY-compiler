find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(symbol_table_test symbol_table_test.cc)

target_link_libraries(symbol_table_test ${GTEST_BOTH_LIBRARIES} ${llvm_libs} pthread sycompiler_base_static)

gtest_discover_tests(symbol_table_test)