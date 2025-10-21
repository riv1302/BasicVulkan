CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

build/VulkanTest: src/main.cpp
	mkdir -p build
	g++ $(CFLAGS) -o build/VulkanTest src/main.cpp $(LDFLAGS)

.PHONY: test clean

test: build/VulkanTest
	./build/VulkanTest

clean:
	rm -f build/*