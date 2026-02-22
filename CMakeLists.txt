cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

if(CMAKE_SYSTEM MATCHES Windows)
   message(STATUS "Target system is Windows")
   set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
elseif(CMAKE_HOST_SYSTEM MATCHES Linux)
    message(STATUS "Build host runs Linux")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)
endif()

set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.x
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML) #SFML connecting

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json) #JSON connecting

FetchContent_Declare(tgui URL https://github.com/texus/TGUI/archive/refs/tags/v1.12.0.tar.gz)
set(TGUI_BACKEND SFML_GRAPHICS)
set(TGUI_STATIC_LIBS TRUE)
FetchContent_MakeAvailable(tgui) #TGUI connecting

add_executable(main 
    src/main.cpp 
    src/entities/Player.cpp 
    src/ui/Menu.cpp 
    src/graphics/Trail.cpp
    src/systems/Platform.cpp
    src/graphics/Background.cpp
    src/systems/Ground.cpp
    src/graphics/Decoration.cpp
    src/core/Enemy.cpp
    src/entities/Bullet.cpp
    src/ui/AskDialogue.cpp
    src/ui/LoadScreen.cpp
    src/core/GameData.cpp
    src/ui/GameLoadingScreen.cpp
    src/entities/Skeleton.cpp
    src/core/EnemyManager.cpp
    src/graphics/Particle.cpp
    src/core/GameCamera.cpp
    src/systems/GameLevel.cpp
    src/entities/enemyPortal.cpp
    src/systems/Spawner.cpp
    src/entities/LevelPortal.cpp
    src/entities/Trader.cpp
    src/systems/Item.cpp
    src/ui/Shop.cpp
    src/ui/PlayerInventory.cpp
    src/ui/PlayerUI.cpp
	src/ui/ChooseDestinationMenu.cpp)

target_include_directories(main PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/src/funcs 
    ${CMAKE_SOURCE_DIR}/src/objects 
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/entities
    ${CMAKE_SOURCE_DIR}/src/graphics
    ${CMAKE_SOURCE_DIR}/src/systems
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/utils) #Project source files

# FFmpeg
target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/include)
target_link_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/ffmpeg/lib)

target_compile_features(main PRIVATE cxx_std_17)

target_link_libraries(main PRIVATE 
    SFML::Graphics 
    nlohmann_json::nlohmann_json
    avcodec avformat swscale avutil
    tgui
)


