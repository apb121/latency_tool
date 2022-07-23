latency_tool: latency_tool.o user_options.o cache_info.o class_parser.o binary_analyser.o
	g++ -g latency_tool.o user_options.o cache_info.o class_parser.o binary_analyser.o -o latency_tool

latency_tool.o: latency_tool.cpp
	g++ -g -c latency_tool.cpp -o latency_tool.o

user_options.o: user_options.cpp user_options.hpp
	g++ -g -c user_options.cpp -o user_options.o

cache_info.o: cache_info.cpp cache_info.hpp
	g++ -g -c cache_info.cpp -o cache_info.o

class_parser.o: class_parser.cpp class_parser.hpp
	g++ -g -c class_parser.cpp -o class_parser.o

binary_analyser.o: binary_analyser.cpp binary_analyser.hpp
	g++ -g -c binary_analyser.cpp -o binary_analyser.o

clean:
	rm -f latency_tool latency_tool.o user_options.o cache_info.o class_parser.o binary_analyser.o