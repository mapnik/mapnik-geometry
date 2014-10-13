CXX := $(CXX)
COMMON_FLAGS = -Wall -Wsign-compare -Wsign-conversion -Wshadow -Wunused-parameter -pedantic -fvisibility-inlines-hidden -std=c++11
CXXFLAGS := $(CXXFLAGS)
LDFLAGS := $(LDFLAGS)

all: geometry_impl_test json_generator_test vertex_converters_test

geometry_impl_test:
	$(CXX) -o geometry_impl_test geometry_impl_test.cpp `mapnik-config --all-flags` $(CXXFLAGS) $(LDFLAGS) -L../src -lboost_timer -lboost_chrono

json_generator_test:
	$(CXX) -o json_generator_test json_generator_test.cpp `mapnik-config --all-flags` $(CXXFLAGS) $(LDFLAGS)-L../src -lboost_timer -lboost_chrono

vertex_converters_test:
	$(CXX) -o vertex_converters_test vertex_converters_test.cpp `mapnik-config --all-flags` $(CXXFLAGS) $(LDFLAGS) -L../src -lboost_timer -lboost_chrono

test:
	./json_generator_test
	./geometry_impl_test 100 20 600
	./vertex_converters_test '{"type": "Feature","geometry":{"type":"MultiPoint","coordinates": [[0,0],[1,1]]},"properties":{}}'

.PHONY: test
