cmake_minimum_required(VERSION 3.15)
project(codex_game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2 packages
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)

# Create executable
add_executable(game src/main.cpp)

# Link libraries
target_link_libraries(game 
    SDL2::SDL2main 
    SDL2::SDL2 
    SDL2_ttf::SDL2_ttf 
    SDL2_image::SDL2_image
)

# Copy assets to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

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
endif()
