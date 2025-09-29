CXX = g++
CXXFLAGS = -std=c++17 $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image
SRC = src/main.cpp src/bitmap_font.cpp src/game.cpp src/player.cpp src/enemy.cpp src/item.cpp src/asset_manager.cpp src/menu_scene.cpp src/scene_manager.cpp src/settings.cpp src/tmx_loader.cpp src/camera.cpp

all: game

game: $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f game
