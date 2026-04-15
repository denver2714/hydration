CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
INCLUDES = -I.
LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl

TARGET = hydration
SRCDIR = src
SHADERDIR = shaders

SOURCES = main.cpp $(SRCDIR)/Shader.cpp $(SRCDIR)/Simulation.cpp $(SRCDIR)/Renderer.cpp
OBJECTS = $(SOURCES:.cpp=.o)

WEB_SOURCES = main_web.cpp $(SRCDIR)/Shader.cpp $(SRCDIR)/Simulation.cpp $(SRCDIR)/Renderer.cpp

.PHONY: all web clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

web:
	mkdir -p web/out
	emcc $(WEB_SOURCES) -std=c++17 -O2 -I. \
		-s USE_GLFW=3 \
		-s USE_WEBGL2=1 \
		-s FULL_ES3=1 \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s EXPORTED_FUNCTIONS='["_main","_web_reset","_web_toggle_gravity"]' \
		-s EXPORTED_RUNTIME_METHODS='["ccall"]' \
		--preload-file web/shaders@shaders \
		-o web/out/hydration.js
	cp web/shell.html web/out/index.html

clean:
	rm -f $(TARGET) $(OBJECTS)
	rm -rf web/out
