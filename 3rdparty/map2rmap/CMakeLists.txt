project(map2rmap)

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
    CInputFile.cpp
)

set(HDRS
    main.h
    CInputFile.h
)

set(UIS

)

set(RCS


)

#qt5_wrap_cpp(MOC_SRCS ${HDRS})
#qt5_wrap_ui(UI_HDRS ${UIS})
qt5_add_resources(RC_SRCS ${RCS})

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GDAL_INCLUDE_DIRS}
    ${PROJ_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRS}
)
if(WIN32)
    include_directories(
        ${CMAKE_SOURCE_DIR}/Win32/
    )
endif(WIN32)

link_directories(
    ${CMAKE_BINARY_DIR}/lib
)

add_executable( map2rmap
    ${SRCS}
    ${HDRS}
    ${UI_HDRS}
    ${RC_SRCS}
    ${MOC_SRCS}
    ${QM_FILES}
)

qt5_use_modules(map2rmap Core)

if(APPLE)
set_target_properties( map2rmap PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${MAC_BUNDLE_PATH}/Contents/Resources
)
endif(APPLE)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE )
endif(MSVC)



target_link_libraries( map2rmap
    ${GDAL_LIBRARIES}
    ${PROJ_LIBRARIES}
    ${JPEG_LIBRARIES}
)

install(
    TARGETS
      map2rmap
    DESTINATION
      ${BIN_INSTALL_DIR}
)
