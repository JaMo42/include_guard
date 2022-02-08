CXX=g++
CXX_FLAGS=-std=c++20 -march=native -mtune=native -O2

include_guard: include_guard.cpp
	$(CXX) $(CXX_FLAGS) -o $@ $^

