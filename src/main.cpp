#if defined(__EMSCRIPTEN__) || defined(_WIN32)
#include <SDL.h>
#include <SDL_image.h>
#elif defined(__APPLE__)
#include <SDL2/SDL.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif
#include <iostream>
#include "game.h"

// Platform-specific main function handling
#ifdef __EMSCRIPTEN__
#define SDL_MAIN_HANDLED
#include <emscripten.h>
#endif

#if defined(__EMSCRIPTEN__) || defined(__APPLE__) || defined(__linux__)
#define SDL_MAIN_HANDLED
#endif

// Global variables for SDL
SDL_Renderer* g_renderer = nullptr;
SDL_Window* g_window = nullptr;
GameScene* g_gameScene = nullptr;

// Game loop function for Emscripten
void gameLoop() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        g_gameScene->handleEvent(e);
    }
    
    g_gameScene->update();
    g_gameScene->render();
    
    // Check if we should quit
    if (g_gameScene->shouldQuit()) {
        #ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
        #endif
    }
}

// SDL initialization function
bool initializeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        // Don't return here - we can still run without images
    } else {
        std::cout << "SDL_image initialized successfully" << std::endl;
    }

    g_window = SDL_CreateWindow("Simple SDL Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      800, 600, SDL_WINDOW_SHOWN);
    if (!g_window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return false;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) {
        std::cout << "Accelerated renderer failed, trying software renderer..." << std::endl;
        g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!g_renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

// Cleanup function
void cleanup() {
    if (g_gameScene) {
        delete g_gameScene;
    }
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    IMG_Quit();
    SDL_Quit();
}

int SDL_main(int argc, char* argv[]) {
    // Initialize SDL
    if (!initializeSDL()) {
        return 1;
    }
    
    // Create and initialize game scene
    g_gameScene = new GameScene();
    if (!g_gameScene->initialize(g_renderer)) {
        std::cerr << "Failed to initialize game scene" << std::endl;
        cleanup();
        return 1;
    }

    #ifdef __EMSCRIPTEN__
    // Set up the main loop for Emscripten
    emscripten_set_main_loop(gameLoop, 0, 1);
    #else
    // Standard desktop game loop
    while (!g_gameScene->shouldQuit()) {
        gameLoop();
        SDL_Delay(16);
    }
    #endif

    // Cleanup
    cleanup();
    return 0;
}

// Platform-specific wrapper main function
#if defined(__EMSCRIPTEN__) || defined(__APPLE__) || defined(__linux__)
int main(int argc, char* argv[]) {
    return SDL_main(argc, argv);
}
#endif