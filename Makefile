CXX = g++
CXXFLAGS = -std=c++17 $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image
SRC = src/main.cpp \
      src/entities/entity.cpp src/entities/player.cpp src/entities/enemy.cpp src/entities/item.cpp src/entities/pet.cpp src/entities/projectile.cpp \
      src/scenes/game.cpp src/scenes/menu_scene.cpp src/scenes/player_select_scene.cpp src/scenes/scene_manager.cpp \
      src/systems/game_manager.cpp src/systems/asset_manager.cpp src/systems/settings.cpp \
      src/rendering/bitmap_font.cpp src/rendering/camera.cpp \
      src/utils/tmx_loader.cpp

all: game

game: $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f game
