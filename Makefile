CXX = g++
CXXFLAGS = -g `pkg-config sdl3 --cflags --libs`

#Renderer.cpp init.cpp CPU.cpp `pkg-config sdl3 --cflags --libs`

build:
	$(CXX) -g -Wall -Wextra -pedantic Renderer.cpp init.cpp CPU.cpp `pkg-config sdl3 --cflags --libs` -o init

init: CPU.o init.cpp
	$(CXX) $(CXXFLAGS) -c CPU.o init.cpp -o init

CPU.o: Renderer.o CPU.cpp
	$(CXX) $(CXXFLAGS) -c Renderer.o CPU.cpp -o CPU.o

Renderer.o: Renderer.cpp
	$(CXX) $(CXXFLAGS) -c Renderer.cpp -o Renderer.o

clean:
	rm -f *.o init
	