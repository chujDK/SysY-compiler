TESTCASE_DIR=./test/testcase
TESTCASES=$(foreach x, $(TESTCASE_DIR), \
          $(wildcard \
		  $(addprefix ${x}/*,.sy)))
SRC_DIR=./
SRC=$(foreach x, $(SRC_DIR), ${x}/*.cc)

bin/sycompiler: ${SRC}
	@echo "making sycompiler..."
	g++ -ggdb -o $@ $^

test: bin/sycompiler
	@echo "begin lexer test.. testcases: ${TESTCASES}"
	@$(foreach x, ${TESTCASES}, echo ${x}; echo "----- \033[32msrc\033[0m -----"; cat ${x}; \
	echo "----- \033[33mend of src\033[0m -----"; \
	echo "----- \033[32mtokens\033[0m -----"; bin/sycompiler ${x}; \
	echo "----- \033[33mend of tokens\033[0m -----";)

.PHONY:clean
clean: bin/sycompiler
	$(shell rm $^)
