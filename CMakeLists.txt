cmake_minimum_required(VERSION 3.14)

project(LayoutWatcher LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus REQUIRED)

add_library(LayoutWatcher SHARED
  LayoutWatcher_global.h
  LayoutWatcher.cpp
  LayoutWatcher.h
  FallbackX11.cpp
  FallbackX11.h
)

target_link_libraries(LayoutWatcher PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::DBus)

target_compile_definitions(LayoutWatcher PRIVATE LAYOUTWATCHER_LIBRARY)
