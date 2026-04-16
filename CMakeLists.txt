cmake_minimum_required(VERSION 3.28)
project(SFMLGameDemo LANGUAGES CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not supported. Configure with: cmake -S . -B build")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.x
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    tgui
    URL https://github.com/texus/TGUI/archive/refs/tags/v1.12.0.tar.gz
)
set(TGUI_BACKEND SFML_GRAPHICS)
set(TGUI_STATIC_LIBS TRUE)
set(TGUI_BUILD_GUI_BUILDER OFF)
FetchContent_MakeAvailable(tgui)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_DIR ${imgui_SOURCE_DIR} CACHE PATH "" FORCE)
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
set(IMGUI_SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(IMGUI_SFML_BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    imgui_sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG ab6b5cfeddb4eb980a42b526dc66901a0976ff6d
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(imgui_sfml)

FetchContent_Declare(
    selbaward
    GIT_REPOSITORY https://github.com/Hapaxia/SelbaWard.git
    GIT_TAG d4c18efdd8313a22eed668ba4d9174ce613f496f
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_GetProperties(selbaward)
if(NOT selbaward_POPULATED)
    cmake_policy(PUSH)
    cmake_policy(SET CMP0169 OLD)
    FetchContent_Populate(selbaward)
    cmake_policy(POP)
endif()

add_library(SelbaWard
    ${selbaward_SOURCE_DIR}/src/SelbaWard/ProgressBar.cpp
    ${selbaward_SOURCE_DIR}/src/SelbaWard/Ring.cpp
)
add_library(SelbaWard::SelbaWard ALIAS SelbaWard)
target_include_directories(SelbaWard PUBLIC ${selbaward_SOURCE_DIR}/src)
target_link_libraries(SelbaWard PUBLIC SFML::Graphics)
target_compile_features(SelbaWard PUBLIC cxx_std_17)

file(GLOB_RECURSE SFML_GAME_DEMO_ALL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE SFML_GAME_DEMO_ALL_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

set(SFML_GAME_DEMO_CORE_SOURCES ${SFML_GAME_DEMO_ALL_SOURCES})
set(SFML_GAME_DEMO_CORE_HEADERS ${SFML_GAME_DEMO_ALL_HEADERS})
list(REMOVE_ITEM SFML_GAME_DEMO_CORE_SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")
list(FILTER SFML_GAME_DEMO_CORE_SOURCES EXCLUDE REGEX ".*/src/editor/.*")
list(FILTER SFML_GAME_DEMO_CORE_HEADERS EXCLUDE REGEX ".*/src/editor/.*")

add_library(game_core STATIC
    ${SFML_GAME_DEMO_CORE_SOURCES}
    ${SFML_GAME_DEMO_CORE_HEADERS}
)

target_include_directories(game_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/entities
    ${CMAKE_SOURCE_DIR}/src/graphics
    ${CMAKE_SOURCE_DIR}/src/systems
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/utils
)

if(WIN32)
    target_include_directories(game_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/include)
    target_link_directories(game_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/lib)
endif()

target_link_libraries(game_core PUBLIC
    SFML::Graphics
    nlohmann_json::nlohmann_json
    avcodec
    avformat
    swscale
    avutil
    tgui
    ImGui-SFML::ImGui-SFML
    SelbaWard::SelbaWard
)

add_executable(main
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)
target_link_libraries(main PRIVATE game_core)

add_executable(level_editor
    ${CMAKE_SOURCE_DIR}/src/editor/LevelEditorMain.cpp
)
target_link_libraries(level_editor PRIVATE game_core)
target_compile_definitions(level_editor PRIVATE SFML_GAME_DEMO_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${SFML_GAME_DEMO_ALL_SOURCES} ${SFML_GAME_DEMO_ALL_HEADERS})

set(SFML_GAME_DEMO_RUNTIME_ASSETS
    data
    fonts
    images
)

add_custom_target(sync_runtime_assets ALL DEPENDS main level_editor)

foreach(asset_dir IN LISTS SFML_GAME_DEMO_RUNTIME_ASSETS)
    add_custom_command(TARGET sync_runtime_assets POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/${asset_dir}"
            "$<TARGET_FILE_DIR:main>/${asset_dir}"
    )
endforeach()

if(WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/libs/ffmpeg/bin")
    file(GLOB SFML_GAME_DEMO_FFMPEG_DLLS CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/libs/ffmpeg/bin/*.dll"
    )

    foreach(runtime_dll IN LISTS SFML_GAME_DEMO_FFMPEG_DLLS)
        foreach(runtime_target IN ITEMS main level_editor)
            add_custom_command(TARGET ${runtime_target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${runtime_dll}"
                    "$<TARGET_FILE_DIR:${runtime_target}>"
            )
        endforeach()
    endforeach()
endif()
