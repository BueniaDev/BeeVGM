cmake_minimum_required(VERSION 3.5)
project(beevgm)

# Require C++17 (and position independent code)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_WAV "Enables the Blythie VGM-to-WAV Converter." ON)
option(BUILD_PLAYER "Enables the Blythie VGM Player." ON)

set(BEEVGM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(GZIP_SOURCES
	em_inflate.c)

add_library(em_inflate STATIC ${GZIP_SOURCES})

set(BEEVGM_WAV_SOURCES
	vgm2wav.cpp)

set(BEEVGM_PLAYER_SOURCES
	player.cpp)

set(BEEVGM_HEADERS
	beevgm.h)

set(BEEVGM_SOURCES
	beevgm.cpp)

add_subdirectory(cores)
add_library(beevgm ${BEEVGM_SOURCES} ${BEEVGM_HEADERS})
target_include_directories(beevgm PUBLIC ${BEEVGM_INCLUDE_DIR})
target_link_libraries(beevgm PUBLIC emu_cores em_inflate)
add_library(libbeevgm ALIAS beevgm)

if (BUILD_WAV STREQUAL "ON")
    project(vgm2wav)
    add_executable(${PROJECT_NAME} ${BEEVGM_WAV_SOURCES})
    include_directories(${PROJECT_NAME} ${BEEVGM_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} libbeevgm)
endif()

if (BUILD_PLAYER STREQUAL "ON")
    project(vgmplayer)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSDL_MAIN_HANDLED")
    add_executable(${PROJECT_NAME} ${BEEVGM_PLAYER_SOURCES})
    include_directories(${PROJECT_NAME} ${BEEVGM_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} libbeevgm)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})

    if (TARGET SDL2::SDL2)
	target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
    else()
	target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
    endif()
endif()


if (WIN32)
    message(STATUS "Operating system is Windows.")
    if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	target_compile_options(beevgm PRIVATE -Wall)
	message(STATUS "Compiler is MinGW.")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
	message(WARNING "MSVC support is HIGHLY experimental, and may not even compile correctly, so be EXTREMELY careful here!")
	target_compile_options(beevgm PRIVATE /W4)
	message(STATUS "Compiler is MSVC.")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	message(WARNING "Clang support on Windows is HIGHLY experimental, and may not even compile correctly, so be EXTREMELY careful here!")
	target_compile_options(beevgm PRIVATE -Wall)
	message(STATUS "Compiler is Clang.")
    else()
	message(SEND_ERROR "Compiler not supported.")
	return()
    endif()
elseif(UNIX AND NOT APPLE)
    message(STATUS "Operating system is Linux.")
    if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	target_compile_options(beevgm PRIVATE -Wall)
	message(STATUS "Compiler is GCC.")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	target_compile_options(beevgm PRIVATE -Wall)
	message(STATUS "Compiler is Clang.")
    else()
	message(SEND_ERROR "Compiler not supported.")
	return()
    endif()
elseif(APPLE)
    message(STATUS "Operating system is Mac.")
    if (CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
	target_compile_options(beevgm PRIVATE -Wall)
	message(STATUS "Compiler is AppleClang.")
    else()
	message(SEND_ERROR "Compiler not supported.")
	return()
    endif()
else()
    message(SEND_ERROR "Operating system not supported.")
    return()
endif()
