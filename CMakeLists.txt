cmake_minimum_required(VERSION 3.15)
project(codex_game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2 packages (skip for Emscripten as it has built-in SDL2)
if(NOT EMSCRIPTEN)
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_image CONFIG REQUIRED)
endif()

# Create executable
add_executable(game src/main.cpp src/bitmap_font.cpp src/game.cpp src/player.cpp src/enemy.cpp src/item.cpp src/asset_manager.cpp src/menu_scene.cpp src/scene_manager.cpp src/settings.cpp src/tmx_loader.cpp src/camera.cpp)

# Link libraries - SDL2main must be linked first
if(EMSCRIPTEN)
    # Emscripten handles SDL2 linking automatically via emcc flags
    target_link_libraries(game)
else()
    target_link_libraries(game 
        SDL2::SDL2main 
        SDL2::SDL2 
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
elseif(APPLE)
    # macOS specific settings
    option(STATIC_SDL2 "Link SDL2 statically on macOS" ON)
    
    if(STATIC_SDL2)
        # Static linking - find static libraries
        find_library(SDL2_STATIC_LIB SDL2 PATHS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES lib)
        find_library(SDL2_IMAGE_STATIC_LIB SDL2_image PATHS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES lib)
        
        if(SDL2_STATIC_LIB AND SDL2_IMAGE_STATIC_LIB)
            # Override the imported targets to use static libraries
            set_target_properties(SDL2::SDL2 PROPERTIES
                IMPORTED_LOCATION ${SDL2_STATIC_LIB}
                INTERFACE_LINK_LIBRARIES "-framework Cocoa -framework IOKit -framework CoreVideo -framework CoreAudio -framework AudioToolbox -framework ForceFeedback"
            )
            set_target_properties(SDL2_image::SDL2_image PROPERTIES
                IMPORTED_LOCATION ${SDL2_IMAGE_STATIC_LIB}
            )
            message(STATUS "Using static SDL2 libraries on macOS")
        else()
            message(WARNING "Static SDL2 libraries not found, falling back to dynamic linking")
        endif()
    else()
        # Dynamic linking - bundle dylib files
        if(TARGET SDL2::SDL2)
            get_target_property(SDL2_DYLIB SDL2::SDL2 IMPORTED_LOCATION)
            if(SDL2_DYLIB)
                get_filename_component(SDL2_DYLIB_DIR ${SDL2_DYLIB} DIRECTORY)
                file(GLOB SDL2_DYLIBS "${SDL2_DYLIB_DIR}/*.dylib")
                foreach(DYLIB ${SDL2_DYLIBS})
                    add_custom_command(TARGET game POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${DYLIB} $<TARGET_FILE_DIR:game>)
                endforeach()
                
                # Fix library paths in the executable
                add_custom_command(TARGET game POST_BUILD
                    COMMAND install_name_tool -change 
                    ${SDL2_DYLIB} @executable_path/$(notdir ${SDL2_DYLIB}) 
                    $<TARGET_FILE:game>
                    COMMENT "Fixing SDL2 library paths in executable")
            endif()
        endif()
    endif()
elseif(EMSCRIPTEN)
    # WebAssembly specific settings - simplified following standard patterns
    set_target_properties(game PROPERTIES
        SUFFIX ".html"
        COMPILE_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]'"
        LINK_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]' -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=64MB -s MAXIMUM_MEMORY=256MB --preload-file assets@/assets --shell-file ${CMAKE_SOURCE_DIR}/web/index.html"
    )
    
    # Rename game.html to index.html after build
    add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename game.html index.html
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    # Copy assets to build directory for web
    file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
endif()
