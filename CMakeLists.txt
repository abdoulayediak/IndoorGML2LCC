include_directories(../..)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../)
include(${CMAKE_CURRENT_SOURCE_DIR}/../LCC_demo_plugin_macro.cmake)

LCC_demo_plugin(IndoorGML_plugin IndoorGML2LCC)
target_link_libraries(IndoorGML_plugin PUBLIC LCC_demo_framework CGAL::CGAL CGAL::CGAL_Qt5 Qt5::Gui Qt5::Widgets Qt5::OpenGL)
