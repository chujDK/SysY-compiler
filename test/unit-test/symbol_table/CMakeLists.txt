find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(symbol_table_test symbol_table_test.cc ${DIR_SRC})

target_link_libraries(symbol_table_test ${GTEST_LIBRARY} ${llvm_libs} pthread)

gtest_discover_tests(symbol_table_test)