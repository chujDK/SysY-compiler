bin/sycompiler: syparse.cc main.cc sydebug.cc
	g++ -ggdb -o $@ $^

clean: bin/sycompiler
	$(shell rm $^)
