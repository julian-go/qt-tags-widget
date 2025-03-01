cmake_minimum_required(VERSION 3.16)
project(QTagEdit VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON) # Handle Qt MOC automatically

# Find required Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Define source files paths
set(QTAGEDIT_HEADERS
    include/QTagEdit/qtagedit.hpp
)

set(QTAGEDIT_SOURCES
    src/qtagedit.cpp
)

# Library target
add_library(QTagEdit ${QTAGEDIT_HEADERS} ${QTAGEDIT_SOURCES})
target_include_directories(QTagEdit 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/QTagEdit
)
target_link_libraries(QTagEdit PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)

# Example application
add_executable(QTagEditExample example/main.cpp)
target_link_libraries(QTagEditExample PRIVATE QTagEdit)
target_include_directories(QTagEditExample PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/QTagEdit
)

# Installation rules
install(TARGETS QTagEdit
    EXPORT QTagEditTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${QTAGEDIT_HEADERS}
    DESTINATION include/QTagEdit
)

# Export package configuration
install(EXPORT QTagEditTargets
    FILE QTagEditTargets.cmake
    NAMESPACE QTagEdit::
    DESTINATION lib/cmake/QTagEdit
)
