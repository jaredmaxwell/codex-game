cmake_minimum_required(VERSION 3.15)
project(codex_game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2 packages (skip for Emscripten as it has built-in SDL2)
if(NOT EMSCRIPTEN)
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_ttf CONFIG REQUIRED)
    find_package(SDL2_image CONFIG REQUIRED)
endif()

# Create executable
add_executable(game src/main.cpp)

# Link libraries - SDL2main must be linked first
if(EMSCRIPTEN)
    # Emscripten handles SDL2 linking automatically via emcc flags
    target_link_libraries(game)
else()
    target_link_libraries(game 
        SDL2::SDL2main 
        SDL2::SDL2 
        SDL2_ttf::SDL2_ttf 
        SDL2_image::SDL2_image
    )
endif()

# Copy assets to build directory (skip for Emscripten as it's handled later)
if(NOT EMSCRIPTEN)
    file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Platform-specific settings
if(WIN32)
    # Copy SDL2 DLLs to output directory
    if(TARGET SDL2::SDL2)
        get_target_property(SDL2_DLL SDL2::SDL2 IMPORTED_LOCATION)
        if(SDL2_DLL)
            get_filename_component(SDL2_DLL_DIR ${SDL2_DLL} DIRECTORY)
            file(GLOB SDL2_DLLS "${SDL2_DLL_DIR}/*.dll")
            foreach(DLL ${SDL2_DLLS})
                add_custom_command(TARGET game POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${DLL} $<TARGET_FILE_DIR:game>)
            endforeach()
        endif()
    endif()
elseif(EMSCRIPTEN)
    # WebAssembly specific settings
    set_target_properties(game PROPERTIES
        SUFFIX ".html"
        COMPILE_FLAGS "-s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_SDL_IMAGE=2"
        LINK_FLAGS "-s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]' -s EXPORTED_FUNCTIONS='[\"_main\"]' -s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]' -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=64MB -s MAXIMUM_MEMORY=256MB -s DISABLE_EXCEPTION_CATCHING=0 -s GL_PREINITIALIZED_CONTEXT=0 -s OFFSCREEN_FRAMEBUFFER=1 --shell-file ${CMAKE_SOURCE_DIR}/web/index.html"
    )
    
    # Rename game.html to index.html after build
    add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename game.html index.html
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    # Copy assets to build directory for web
    file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
endif()
