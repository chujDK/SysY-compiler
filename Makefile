bin/sycompiler: syparse.cc main.cc
	g++ -ggdb -o $@ $^
