CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

build/VulkanTest: src/*.cpp src/*.hpp
	mkdir -p build
	g++ $(CFLAGS) -o build/VulkanTest src/*.cpp $(LDFLAGS)

.PHONY: test clean

test: build/VulkanTest src/shaders/shader.vert src/shaders/shader.frag ./compile.sh
	mkdir -p build/shaders
	./compile.sh
	./build/VulkanTest

clean:
	rm -fr build/*