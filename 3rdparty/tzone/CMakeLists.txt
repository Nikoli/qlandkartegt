project(tzone)

# Required cmake version
cmake_minimum_required(VERSION 2.8.8)


set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)

set(SRCS
    zonefiles.cpp
    src/tzfile.cpp
    src/tzdata.cpp
)

if(WIN32)
    SET(SRCS ${SRCS}  src/tzsys_win.cpp)
else(WIN32)
    SET(SRCS ${SRCS}  src/tzsys_unix.cpp)
endif(WIN32)


set(HDRS
    include/tzfile.h
    include/tzdata.h
    src/tzsys.h
)

set(UIS

)

set(RCS


)

qt5_add_resources(RC_SRCS ${RCS})


include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/3rdparty/tzone/include
)

link_directories(
    ${CMAKE_BINARY_DIR}/lib
)

add_library(tzone
  ${SRCS}
  ${HDRS}
  ${UI_HDRS}
  ${RC_SRCS}
  ${MOC_SRCS}
  ${QM_FILES}
)


qt5_use_modules(tzone Core)


