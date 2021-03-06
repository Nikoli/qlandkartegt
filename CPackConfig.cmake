include(InstallRequiredSystemLibraries)

# For help take a look at:
# http://www.cmake.org/Wiki/CMake:CPackConfiguration

### general settings
string(TOLOWER ${APPLICATION_NAME} CPACK_PACKAGE_NAME)
set(CPACK_PACKAGE_VENDOR "QLandkarte")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GPS Map, Route, Waypoint and Tracking Tool")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

### versions
# set(CPACK_PACKAGE_VERSION_MAJOR "0")
# set(CPACK_PACKAGE_VERSION_MINOR "9")
# set(CPACK_PACKAGE_VERSION_PATCH "3")
set(CPACK_PACKAGE_VERSION "${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}.${APPLICATION_VERSION_PATCH}")

set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

### source package settings
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "~$;[.]swp$;/[.]svn/;/[.]git/;.gitignore;/build/;tags;cscope.*;[.]rej$;[.]orig$;svn-commit[.]")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

include(CPack)
