TESTCASE_DIR=./test/testcase
TESTCASES=$(foreach x, $(TESTCASE_DIR), \
          $(wildcard \
		  $(addprefix ${x}/*,.sy)))
SRC_DIR=./src
SRC=$(foreach x, $(SRC_DIR), ${x}/*.cc)
LIB_DIR=./lib
BIN_DIR=./bin

syparser: ${SRC}
	@echo "making syparser..."
	g++ -I $(LIB_DIR) -DPARSER -ggdb -o $(BIN_DIR)/$@ $^

sylexer: ${SRC}
	@echo "making sylexer..."
	g++ -I $(LIB_DIR) -DLEXER -ggdb -o $(BIN_DIR)/$@ $^

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

.PHONY:clean
clean: bin/*
	$(shell rm $^)

all: syparser sylexer
	@echo "making all done"
