cmake_minimum_required(VERSION 3.10)

project(imgradient VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(IMGRADIENT_STANDALONE_PROJECT "Build imgradient example project" OFF)

if(IMGRADIENT_STANDALONE_PROJECT)
  set(IMGRADIENT_IMGUI_TARGET_NAME imgui)
endif()

if(NOT DEFINED IMGRADIENT_IMGUI_TARGET_NAME)
  message(FATAL_ERROR "IMGRADIENT_IMGUI_TARGET_NAME is not defined. Please define it before including this CMakeLists.txt.")
endif()

set(IMGRADIENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(IMGRADIENT_SOURCES ${IMGRADIENT_DIR}/imgradient.cpp)

set(IMGRADIENT_HEADERS
  ${IMGRADIENT_DIR}/imgradient.h
  ${IMGRADIENT_DIR}/imgradient_internal.h
)

add_library(imgradient STATIC ${IMGRADIENT_SOURCES} ${IMGRADIENT_HEADERS})

target_link_libraries(imgradient PRIVATE ${IMGRADIENT_IMGUI_TARGET_NAME})

target_include_directories(imgradient PUBLIC ${IMGRADIENT_DIR})

if(IMGRADIENT_STANDALONE_PROJECT)
  add_subdirectory(example)
endif()
