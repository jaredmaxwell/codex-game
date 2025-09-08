# Project Instructions

- Always run `make` and `./game` to validate the code compiles and runs after each change
- If we update the local makefile we need to make the equivalent changes to the cross compile build files. Makefile.linux, Makefile.macos, CMakeLists.txt, vcpkg.json

# Basic Architecture
This is a C game using SDL2 that cross compiles in GitHub actions for MacOS, Windows, Linux, and Emscripten. Local development is happening on Windows. Assets should always be dynamically loaded with simple draw rects or not drawn as fallbacks. 

