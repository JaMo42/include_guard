CXX=g++
CXX_FLAGS=-std=c++11 -march=native -O3

include_guard: include_guard.cpp
	$(CXX) $(CXX_FLAGS) -o $@ $^

