include_directories(../..)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include(${CMAKE_CURRENT_SOURCE_DIR}/../LCC_demo_plugin_macro.cmake)

LCC_demo_plugin(IndoorGML_plugin IndoorGML2LCC)
target_link_libraries(IndoorGML_plugin PUBLIC CGAL::CGAL CGAL::CGAL_Qt5 Qt5::Gui Qt5::Widgets Qt5::OpenGL)
