add_executable(symbol_table_test symbol_table_test.cc)

target_link_libraries(symbol_table_test GTest::gtest_main ${llvm_libs} pthread sycompiler_base_static)

include(GoogleTest)
gtest_discover_tests(symbol_table_test)
