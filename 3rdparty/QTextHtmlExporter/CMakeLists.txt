project(QTextHtmlExporter)

# Required cmake version
cmake_minimum_required(VERSION 2.8.8)


set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)
find_package(Qt5Gui)


set(SRCS
    QTextHtmlExporter.cpp
)

set(HDRS
    QTextHtmlExporter.h
)

set(UIS

)

set(RCS


)

qt5_add_resources(RC_SRCS ${RCS})

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QT_QTXML_INCLUDE_DIR}
  ${QT_ZLIB_INCLUDE_DIR}
)

link_directories(
    ${CMAKE_BINARY_DIR}/lib
)

add_library(qtexthtmlexporter
  ${SRCS}
  ${HDRS}
  ${UI_HDRS}
  ${RC_SRCS}
  ${MOC_SRCS}
  ${QM_FILES}
)

qt5_use_modules(qtexthtmlexporter Core Gui)
