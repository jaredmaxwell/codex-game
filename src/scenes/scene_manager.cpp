#include "scene_manager.h"
#include <iostream>

SceneManager::SceneManager() {
    m_currentScene = SceneType::GAME;
    m_settings = new Settings();
}

SceneManager::~SceneManager() {
    if (m_gameScene) {
        delete m_gameScene;
    }
    if (m_menuScene) {
        delete m_menuScene;
    }
    if (m_playerSelectScene) {
        delete m_playerSelectScene;
    }
    if (m_settings) {
        delete m_settings;
    }
}

bool SceneManager::initialize(SDL_Renderer* renderer, SDL_Window* window) {
    m_renderer = renderer;
    m_window = window;
    
    // Load settings
    m_settings->loadFromFile();
    
    // Apply fullscreen setting on startup
    if (m_settings->isFullscreen()) {
        if (SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP) == 0) {
            std::cout << "Applied fullscreen setting on startup (desktop resolution)" << std::endl;
        } else {
            std::cerr << "Failed to apply fullscreen setting: " << SDL_GetError() << std::endl;
        }
    }
    
    // Initialize game scene
    m_gameScene = new GameScene();
    if (!m_gameScene->initialize(renderer)) {
        std::cerr << "Failed to initialize game scene" << std::endl;
        return false;
    }
    
    // Initialize menu scene
    m_menuScene = new MenuScene();
    if (!m_menuScene->initialize(renderer)) {
        std::cerr << "Failed to initialize menu scene" << std::endl;
        return false;
    }
    
    // Initialize player select scene
    m_playerSelectScene = new PlayerSelectScene();
    if (!m_playerSelectScene->initialize(renderer)) {
        std::cerr << "Failed to initialize player select scene" << std::endl;
        return false;
    }
    
    m_quit = false;
    m_currentScene = SceneType::PLAYER_SELECT;  // Start with player select
    
    return true;
}

void SceneManager::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_QUIT) {
        m_quit = true;
        return;
    }
    
    // Handle window resize events
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
        if (m_gameScene) {
            m_gameScene->handleWindowResize(event.window.data1, event.window.data2);
        }
        return;
    }
    
    // Handle F1 key to toggle menu (only from game scene)
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F1) {
        if (m_currentScene == SceneType::GAME) {
            switchToMenu();
        } else if (m_currentScene == SceneType::MENU) {
            switchToGame();
        }
        return;
    }
    
    // Handle ESC key to close menu or go back
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        if (m_currentScene == SceneType::MENU) {
            switchToGame();
        } else if (m_currentScene == SceneType::PLAYER_SELECT) {
            m_quit = true;  // Exit from player select
        } else {
            m_quit = true;
        }
        return;
    }
    
    // Route events to current scene
    if (m_currentScene == SceneType::GAME && m_gameScene) {
        m_gameScene->handleEvent(event);
        
        // Check if game wants to quit
        if (m_gameScene->shouldQuit()) {
            m_quit = true;
        }
    } else if (m_currentScene == SceneType::MENU && m_menuScene) {
        m_menuScene->handleEvent(event);
        
        // Check if menu wants to close
        if (m_menuScene->shouldClose()) {
            int selectedAction = m_menuScene->getSelectedAction();
            if (selectedAction == 1) { // Toggle Fullscreen
                toggleFullscreen();
            } else if (selectedAction == 2) { // Restart Game
                if (m_gameScene) {
                    m_gameScene->restart();
                }
            } else if (selectedAction == 3) { // Quit Game
                m_quit = true;
                std::cout << "Quit game selected - exiting application" << std::endl;
                return; // Exit immediately without closing menu
            }
            handleMenuAction();
        }
    } else if (m_currentScene == SceneType::PLAYER_SELECT && m_playerSelectScene) {
        m_playerSelectScene->handleEvent(event);
        
        // Check if player select wants to close
        if (m_playerSelectScene->shouldClose()) {
            m_selectedCharacterClass = m_playerSelectScene->getSelectedClass();
            std::cout << "Selected character class: " << static_cast<int>(m_selectedCharacterClass) << std::endl;
            switchToGame();
        }
    }
}

void SceneManager::update() {
    if (m_currentScene == SceneType::GAME && m_gameScene) {
        m_gameScene->update();
    } else if (m_currentScene == SceneType::MENU && m_menuScene) {
        m_menuScene->update();
    } else if (m_currentScene == SceneType::PLAYER_SELECT && m_playerSelectScene) {
        m_playerSelectScene->update();
    }
}

void SceneManager::render() {
    if (m_currentScene == SceneType::GAME && m_gameScene) {
        m_gameScene->render();
    } else if (m_currentScene == SceneType::MENU && m_menuScene) {
        m_menuScene->render();
    } else if (m_currentScene == SceneType::PLAYER_SELECT && m_playerSelectScene) {
        m_playerSelectScene->render();
    }
}

void SceneManager::switchToGame() {
    m_currentScene = SceneType::GAME;
    
    // Set the player's character class
    if (m_gameScene) {
        m_gameScene->setCharacterClass(m_selectedCharacterClass);
    }
    
    std::cout << "Switched to game scene" << std::endl;
}

void SceneManager::switchToMenu() {
    m_currentScene = SceneType::MENU;
    if (m_menuScene) {
        m_menuScene->reset();
    }
    std::cout << "Switched to menu scene" << std::endl;
}

void SceneManager::switchToPlayerSelect() {
    m_currentScene = SceneType::PLAYER_SELECT;
    if (m_playerSelectScene) {
        m_playerSelectScene->reset();
    }
    std::cout << "Switched to player select scene" << std::endl;
}

void SceneManager::handleMenuAction() {
    // Get the selected menu item from the menu scene
    // For now, we'll handle the fullscreen toggle here
    // In the future, we could get the selected item from the menu scene
    switchToGame();
}

void SceneManager::toggleFullscreen() {
    if (m_settings && m_window) {
        bool currentFullscreen = m_settings->isFullscreen();
        bool newFullscreen = !currentFullscreen;
        
        // Toggle SDL window fullscreen mode
        Uint32 flags = SDL_GetWindowFlags(m_window);
        bool isCurrentlyFullscreen = (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
        
        if (newFullscreen && !isCurrentlyFullscreen) {
            // Switch to fullscreen (desktop resolution)
            if (SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP) == 0) {
                std::cout << "Switched to fullscreen mode (desktop resolution)" << std::endl;
            } else {
                std::cerr << "Failed to switch to fullscreen: " << SDL_GetError() << std::endl;
                return;
            }
        } else if (!newFullscreen && isCurrentlyFullscreen) {
            // Switch to windowed mode
            if (SDL_SetWindowFullscreen(m_window, 0) == 0) {
                std::cout << "Switched to windowed mode" << std::endl;
            } else {
                std::cerr << "Failed to switch to windowed mode: " << SDL_GetError() << std::endl;
                return;
            }
        }
        
        // Update and save settings
        m_settings->setFullscreen(newFullscreen);
        m_settings->saveToFile();
        
        std::cout << "Fullscreen toggled to: " << (m_settings->isFullscreen() ? "ON" : "OFF") << std::endl;
    }
}
