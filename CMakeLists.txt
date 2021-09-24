cmake_minimum_required(VERSION 3.14)

project(LayoutWatcher LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	set(IS_TOPLEVEL_PROJECT TRUE)
else()
	set(IS_TOPLEVEL_PROJECT FALSE)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

find_package(sdbus-c++ REQUIRED)
find_package(X11)

option(LAYOUTWATCHER_LINK_X11 "Link library with X11. It's need for correctly work fallback, when main application doesn't linked with X11" ${X11_FOUND})
option(LAYOUTWATCHER_BUILD_SHARED "Build shared library instead of static" ${IS_TOPLEVEL_PROJECT})

file(GLOB ${PROJECT_NAME}_LIST
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fallbacks/*.cpp
)
file(GLOB ${PROJECT_NAME}_LIST_H
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/fallbacks/*.h
)

if (LAYOUTWATCHER_BUILD_SHARED)
	add_library(LayoutWatcher SHARED ${${PROJECT_NAME}_LIST} ${${PROJECT_NAME}_LIST_H})
else()
	add_library(LayoutWatcher STATIC ${${PROJECT_NAME}_LIST} ${${PROJECT_NAME}_LIST_H})
endif()

target_link_libraries(LayoutWatcher PRIVATE SDBusCpp::sdbus-c++)
if (LAYOUTWATCHER_LINK_X11 AND X11_FOUND)
	target_link_libraries(LayoutWatcher PRIVATE X11::Xkb)
endif()

target_compile_definitions(LayoutWatcher PRIVATE LAYOUTWATCHER_LIBRARY)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.h" DESTINATION include/LayoutWatcher)
