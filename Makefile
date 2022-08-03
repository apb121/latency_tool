latency_tool: latency_tool.o user_options.o cache_info.o data_analyser.o binary_analyser.o
	g++ -g -std=c++2a latency_tool.o user_options.o cache_info.o data_analyser.o binary_analyser.o -o latency_tool

latency_tool.o: latency_tool.cpp
	g++ -g -c -std=c++2a latency_tool.cpp -o latency_tool.o

user_options.o: user_options.cpp user_options.hpp
	g++ -g -c -std=c++2a user_options.cpp -o user_options.o

cache_info.o: cache_info.cpp cache_info.hpp
	g++ -g -c -std=c++2a cache_info.cpp -o cache_info.o

data_analyser.o: data_analyser.cpp data_analyser.hpp
	g++ -g -c -std=c++2a data_analyser.cpp -o data_analyser.o

binary_analyser.o: binary_analyser.cpp binary_analyser.hpp
	g++ -g -c -std=c++2a binary_analyser.cpp -o binary_analyser.o

clean:
	rm -f latency_tool latency_tool.o user_options.o cache_info.o data_analyser.o binary_analyser.o