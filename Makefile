TESTFILE_DIR=./test/testfile
TESTCASE_DIR=./test/testcase
TESTCASES=$(foreach x, $(TESTCASE_DIR), \
          $(wildcard \
		  $(addprefix ${x}/*,.sy)))
SRC_DIR=./src
SRC=$(foreach x, $(SRC_DIR), ${x}/*.cc)
LIB_DIR=./lib
LIB=$(foreach x, $(LIB_DIR), ${x}/*.h)
BIN_DIR=./bin
CXXFLAGS=-ggdb #-fsanitize=address
DEFINE=-DDEBUG 
LLVMFLAGS=`llvm-config --cxxflags --ldflags --libs --libfiles --system-libs`

syparser: ${SRC}
	@echo "making syparser..."
	g++ -I $(LIB_DIR) -DPARSER $(DEFINE) $(CXXFLAGS) $(LLVMFLAGS) -o $(BIN_DIR)/$@ $^

sylexer: ${SRC}
	@echo "making sylexer..."
	clang++ -I $(LIB_DIR) -DLEXER $(DEFINE) $(CXXFLAGS) $(LLVMFLAGS) -o $(BIN_DIR)/$@ $^

syinterpreter: ${SRC}
	@echo "making syinterpreter..."
	clang++ -I $(LIB_DIR) -DINTERPRETER $(DEFINE) $(CXXFLAGS) $(LLVMFLAGS) -o $(BIN_DIR)/$@ $^

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
clean: bin/*
	$(shell rm $^)

all: syparser sylexer
	@echo "making all done"
