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

file(GLOB_RECURSE SFML_GAME_DEMO_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE SFML_GAME_DEMO_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

add_executable(main
    ${SFML_GAME_DEMO_SOURCES}
    ${SFML_GAME_DEMO_HEADERS}
)

target_include_directories(main PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/entities
    ${CMAKE_SOURCE_DIR}/src/graphics
    ${CMAKE_SOURCE_DIR}/src/systems
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/utils
)

if(WIN32)
    target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/include)
    target_link_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/lib)
endif()

target_link_libraries(main PRIVATE
    SFML::Graphics
    nlohmann_json::nlohmann_json
    avcodec
    avformat
    swscale
    avutil
    tgui
)

source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${SFML_GAME_DEMO_SOURCES} ${SFML_GAME_DEMO_HEADERS})

set(SFML_GAME_DEMO_RUNTIME_ASSETS
    data
    fonts
    images
)

foreach(asset_dir IN LISTS SFML_GAME_DEMO_RUNTIME_ASSETS)
    add_custom_command(TARGET main POST_BUILD
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
        add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${runtime_dll}"
                "$<TARGET_FILE_DIR:main>"
        )
    endforeach()
endif()
