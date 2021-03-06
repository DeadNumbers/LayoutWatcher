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
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

find_package(sdbus-c++)
find_package(X11)

option(LAYOUTWATCHER_LINK_X11 "Link library with X11. It's need for correctly work fallback, when main application doesn't linked with X11" ${X11_FOUND})
option(LAYOUTWATCHER_BUNDLED_SDBUSCPP "Use bundled sdbus-c++" OFF)

file(GLOB ${PROJECT_NAME}_LIST
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/fallbacks/*.cpp
)
file(GLOB ${PROJECT_NAME}_LIST_H
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/fallbacks/*.h
)

add_library(LayoutWatcher ${${PROJECT_NAME}_LIST} ${${PROJECT_NAME}_LIST_H})

if (NOT sdbus-c++_FOUND OR LAYOUTWATCHER_BUNDLED_SDBUSCPP)
	if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/sdbus-cpp)
		execute_process(COMMAND git clone --recursive https://github.com/Kistler-Group/sdbus-cpp.git WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	endif()
	if (NOT BUILD_SHARED_LIBS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")
	endif()
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sdbus-cpp)
	target_include_directories(LayoutWatcher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sdbus-cpp/include/sdbus-c++)
	target_link_directories(LayoutWatcher PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/sdbus-cpp)
	target_link_libraries(LayoutWatcher PRIVATE sdbus-c++)
else()
	target_link_libraries(LayoutWatcher PRIVATE SDBusCpp::sdbus-c++)
endif()

if (LAYOUTWATCHER_LINK_X11 AND X11_FOUND)
	target_link_libraries(LayoutWatcher PRIVATE X11::Xkb)
endif()

target_compile_definitions(LayoutWatcher PRIVATE LAYOUTWATCHER_LIBRARY)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.h" DESTINATION include/LayoutWatcher)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}_global.h" DESTINATION include/LayoutWatcher)
