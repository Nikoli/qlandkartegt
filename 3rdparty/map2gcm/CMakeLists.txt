project(map2gcm)

# Required cmake version
cmake_minimum_required(VERSION 2.8.8)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)
find_package(GDAL REQUIRED)
find_package(PROJ REQUIRED)
find_package(JPEG REQUIRED)


set(SRCS
    main.cpp
)

set(HDRS

)

set(UIS

)

set(RCS


)

#qt5_wrap_cpp(MOC_SRCS ${HDRS})
qt5_add_resources(RC_SRCS ${RCS})

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GDAL_INCLUDE_DIRS}
    ${PROJ_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/3rdparty/QZip
)
if(WIN32)
    include_directories(
        ${CMAKE_SOURCE_DIR}/Win32/
    )
endif(WIN32)

link_directories(
    ${CMAKE_BINARY_DIR}/lib
)

add_executable( map2gcm
    ${SRCS}
    ${HDRS}
    ${UI_HDRS}
    ${RC_SRCS}
    ${MOC_SRCS}
    ${QM_FILES}
)

qt5_use_modules(map2gcm Core)

if(APPLE)
set_target_properties( map2gcm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${MAC_BUNDLE_PATH}/Contents/Resources
)
endif(APPLE)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE )
endif(MSVC)


if(WIN32)
target_link_libraries( map2gcm
    ${GDAL_LIBRARIES}
    ${PROJ_LIBRARIES}
    ${JPEG_LIBRARIES}
    qzip
)
else(WIN32)
target_link_libraries( map2gcm
    ${GDAL_LIBRARIES}
    ${PROJ_LIBRARIES}
    ${JPEG_LIBRARIES}
    qzip
)
endif(WIN32)

install(
    TARGETS
      map2gcm
    DESTINATION
      ${BIN_INSTALL_DIR}
)
