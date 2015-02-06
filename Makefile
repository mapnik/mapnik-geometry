CXX := $(CXX)
COMMON_FLAGS = -Wall -Wsign-compare -Wsign-conversion -Wshadow -Wunused-parameter -pedantic -fvisibility-inlines-hidden
CXXFLAGS := $(CXXFLAGS)
LDFLAGS := $(LDFLAGS)

all: geometry_impl_test json_generator_test vertex_converters_test geometry_adapters

geometry_adapters: geometry_adapters.cpp geometry_adapters.hpp
	$(CXX) -o geometry_adapters geometry_adapters.cpp -F/ -framework CoreFoundation -g `mapnik-config --all-flags` $(COMMON_FLAGS) $(CXXFLAGS) $(LDFLAGS) -L../src

geometry_impl_test: geometry_impl_test.cpp geometry_impl.hpp
	$(CXX) -o geometry_impl_test geometry_impl_test.cpp -F/ -framework CoreFoundation -g `mapnik-config --all-flags` $(COMMON_FLAGS) $(CXXFLAGS) $(LDFLAGS) -L../src

json_generator_test: json_generator_test.cpp
	$(CXX) -o json_generator_test json_generator_test.cpp -F/ -framework CoreFoundation -g `mapnik-config --all-flags` $(COMMON_FLAGS) $(CXXFLAGS) $(LDFLAGS)-L../src

vertex_converters_test: vertex_converters_test.cpp
	$(CXX) -o vertex_converters_test vertex_converters_test.cpp -F/ -framework CoreFoundation -g `mapnik-config --all-flags` $(COMMON_FLAGS) $(CXXFLAGS) $(LDFLAGS) -L../src

test:
	./json_generator_test
	./geometry_impl_test 100 20 600
	./vertex_converters_test '{"type": "Feature","geometry":{"type":"MultiPoint","coordinates": [[0,0],[1,1]]},"properties":{}}'

clean:
	rm -f ./json_generator_test
	rm -f ./geometry_impl_test
	rm -f ./vertex_converters_test
	rm -f ./geometry_adapters

.PHONY: test clean
