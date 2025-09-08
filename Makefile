CXX = g++
CXXFLAGS = -std=c++17 $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image
SRC = src/main.cpp src/bitmap_font.cpp src/game.cpp

all: game

game: $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f game
