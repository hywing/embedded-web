cmake_minimum_required(VERSION 3.5)

project(websocket-client LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets WebSockets REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    main_window.cpp
    main_window.h
    main_window.ui
    connector.cpp
    connector.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::WebSockets)
