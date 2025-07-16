cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.x
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(main 
    src/main.cpp 
    src/objects/Player.cpp 
    src/objects/Menu.cpp 
    src/objects/Trail.cpp
    src/objects/Platform.cpp
    src/objects/Background.cpp
    src/objects/Ground.cpp
    src/objects/Decoration.cpp
    src/objects/Enemy.cpp
    src/objects/Bullet.cpp)

target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/funcs ${CMAKE_SOURCE_DIR}/src/objects)
target_compile_features(main PRIVATE cxx_std_17)
target_link_libraries(main PRIVATE SFML::Graphics)



 