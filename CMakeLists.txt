cmake_minimum_required(VERSION 3.1)

project(3d_scene LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Quick REQUIRED)
find_package(assimp REQUIRED)
find_package(OpenGL REQUIRED)
add_executable(${PROJECT_NAME}
    "main.cpp"
    "qml.qrc"
    "scene_gui.h" "scene_gui.cpp"
    "scene_render_dynamic.h" "scene_render_dynamic.cpp"
    "geometry.h" "geometry.cpp"
    "navagator_mouse_controller.h" "navagator_mouse_controller.cpp"
    "ray_tracer.h" "ray_tracer.cpp"
    "utils.h")
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Quick ${ASSIMP_LIBRARIES} ${OPENGL_LIBRARIES})
