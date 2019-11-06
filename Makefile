CXX=g++
CXX_FLAGS=-std=c++17 -march=native -O3

include_guard: include_guard.cpp
	$(CXX) $(CXX_FLAGS) -o $@ $^

