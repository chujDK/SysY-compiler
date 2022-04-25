UNIT_TEST_DIR=./test/unit-test
TESTCASE_DIR=./test/testcase
TESTCASES=$(foreach x, $(TESTCASE_DIR), \
          $(wildcard \
		  $(addprefix ${x}/*,.sy)))
SRC_DIR=./src
SRC=$(foreach x, $(SRC_DIR), ${x}/*.cc)
LIB_DIR=./include
LIB=$(foreach x, $(LIB_DIR), ${x}/*.h)
BIN_DIR=./bin
CXXFLAGS=-std=c++2a -gdwarf-2 -ggdb -frtti# -fsanitize=address
DEFINE=-DDEBUG
LLVMFLAGS=`llvm-config --cxxflags --ldflags --libs --libfiles --system-libs`

all: syparser sylexer syinterpreter irtest
	@echo "making all..."

.PHONY:syparser
syparser:${BIN_DIR}/syparser

${BIN_DIR}/syparser: ${SRC} | ${LIB}
	@echo "making syparser..."
	clang++ -I $(LIB_DIR) -DPARSER $(DEFINE) $(CXXFLAGS) $(LLVMFLAGS) -o $@ $^

.PHONY:sylexer
sylexer:${BIN_DIR}/sylexer

${BIN_DIR}/sylexer: ${SRC} | ${LIB}
	@echo "making sylexer..."
	clang++ -I $(LIB_DIR) -DLEXER $(DEFINE) $(CXXFLAGS) $(LLVMFLAGS) -o $@ $^

.PHONY:syinterpreter
syinterpreter:${BIN_DIR}/syinterpreter

${BIN_DIR}/syinterpreter: ${SRC} | ${LIB}
	@echo "making syinterpreter..."
	clang++ -I $(LIB_DIR) -DINTERPRETER $(DEFINE) $(CXXFLAGS) $(LLVMFLAGS) -o $@ $^

.PHONY:irtest
irtest: ${BIN_DIR}/irtest

${BIN_DIR}/irtest: ${SRC} | ${LIB}
	@echo "making irtest..."
	clang++ -I $(LIB_DIR) -DCOMPILER $(DEFINE) $(CXXFLAGS) $(LLVMFLAGS) -o $@ $^

.PHONY:test-parser
test-parser: syparser
	@echo "begin lexer test.. testcases: ${TESTCASES}"
	@$(foreach x, ${TESTCASES}, echo "\033[35m${x}\033[0m"; echo "----- \033[32msrc\033[0m -----"; cat ${x}; \
	echo "----- \033[33mend of src\033[0m -----"; \
	echo "----- \033[32mAST\033[0m -----"; $(BIN_DIR)/$^ ${x}; \
	echo "----- \033[33mend of tokens\033[0m -----";)

.PHONY:test-lexer
test-lexer: sylexer
	@echo "begin lexer test.. testcases: ${TESTCASES}"
	@$(foreach x, ${TESTCASES}, echo "\033[35m${x}\033[0m"; echo "----- \033[32msrc\033[0m -----"; cat ${x}; \
	echo "----- \033[33mend of src\033[0m -----"; \
	echo "----- \033[32mtokens\033[0m -----"; $(BIN_DIR)/$^ ${x}; \
	echo "----- \033[33mend of tokens\033[0m -----";)

.PHONY:test-interpreter
test-interpreter: syinterpreter
	@echo "begin interpreter test.. testcases: ${TESTCASES}"
	@$(foreach x, ${TESTCASES}, echo "\033[35m${x}\033[0m"; echo "----- \033[32msrc\033[0m -----"; cat ${x}; \
	echo "----- \033[33mend of src\033[0m -----";)

# FIXME: this is buggy
.PHONY:btype
parser/btype: ${UNIT_TEST_DIR}/parser/btype/btype_test.cc ${SRC} | ${LIB}
	clang++ -I $(LIB_DIR) -DUNIT_TEST $(DEFINE) $(CXXFLAGS) -o $(BIN_DIR)/$@ $^
	$(BIN_DIR)/$@ > /tmp/_$@_output
	diff /tmp/_$@_output ${TESTFILE_DIR}/$@_output_expect
	if [ $$? -eq 0 ]; then
		echo "\033[1;32m[+]\033\0[m $@ \033[1;32msuccess!\033[0m"
	else
		echo "\033[1;31m[-]\033\0[m $@ \033[1;31mfailed!\033[0m"
	fi

#.ONESHELL:
.PHONY:UNIT_test
UNIT_test: ${SRC} ${TESTFILE_DIR}/UNIT_test.cc
	g++ -I $(LIB_DIR) -DUNIT_TEST $(DEFINE) $(CXXFLAGS) -o $(BIN_DIR)/$@ $^
	$(BIN_DIR)/$@ > /tmp/_$@_output
	diff /tmp/_$@_output ${TESTFILE_DIR}/$@_output_expect
	if [ $$? -eq 0 ]; then
		echo "\033[1;32m[+]\033\0[m $@ \033[1;32msuccess!\033[0m"
	else
		echo "\033[1;31m[-]\033\0[m $@ \033[1;31mfailed!\033[0m"
	fi

.PHONY:clean
clean:
	$(shell rm bin/*)
