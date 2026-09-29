CXX = g++
CXXFLAGS = -O2 -std=c++17 $(shell fltk-config --cxxflags --use-gl)
LDFLAGS = $(shell fltk-config --ldflags --use-gl) -lGL -lGLU -lfltk_gl -lfltk

TARGET = fgs_gravity
SRCS = main.cpp Simulation.cpp GravityWindow.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
