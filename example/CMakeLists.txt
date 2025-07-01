include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        docking
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_SOURCES
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

set(IMGUI_BACKEND_SOURCES
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC
  ${IMGUI_SOURCES}
  ${IMGUI_BACKEND_SOURCES}
)

target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${glfw_SOURCE_DIR}/include
)

add_executable(imgradient_example ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_link_libraries(imgradient_example PRIVATE imgui glfw imgradient)
