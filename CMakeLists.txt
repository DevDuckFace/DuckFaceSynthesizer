cmake_minimum_required(VERSION 3.16)
project(DuckFaceSynth VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- raylib (fetched automatically) ----
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
# CMake 4.x removed compat with very old cmake_minimum_required values
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)
FetchContent_MakeAvailable(raylib)

add_executable(DuckFaceSynth WIN32
    src/main.cpp
)
# Keep console off on Windows, normal elsewhere
if(NOT WIN32)
    set_target_properties(DuckFaceSynth PROPERTIES WIN32_EXECUTABLE OFF)
endif()

target_link_libraries(DuckFaceSynth PRIVATE raylib)

if(WIN32)
    target_link_libraries(DuckFaceSynth PRIVATE winmm)
    # entry point for WIN32 apps built with main()
    if(MSVC)
        set_target_properties(DuckFaceSynth PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
    endif()
endif()

install(TARGETS DuckFaceSynth RUNTIME DESTINATION .)
