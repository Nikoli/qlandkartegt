project(qtsoap)

# Required cmake version
cmake_minimum_required(VERSION 2.6.0)


set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)

set(SRCS
    qtsoap.cpp
)

set(HDRS
    qtsoap.h
)

set(UIS

)

set(RCS


)



qt5_add_resources(RC_SRCS ${RCS})



include_directories(

)

link_directories(
    ${CMAKE_BINARY_DIR}/lib
)

add_library(qtsoap
  ${SRCS}
  ${HDRS}
  ${UI_HDRS}
  ${RC_SRCS}
  ${MOC_SRCS}
  ${QM_FILES}
)

qt5_use_modules(qtsoap Core)
