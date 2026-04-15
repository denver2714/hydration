CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
INCLUDES = -I.
LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl

TARGET = hydration
SRCDIR = src
SHADERDIR = shaders

SOURCES = main.cpp $(SRCDIR)/Shader.cpp $(SRCDIR)/Simulation.cpp $(SRCDIR)/Renderer.cpp
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
