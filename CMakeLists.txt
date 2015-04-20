CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(autojsoncxx)

# Perform tests
enable_testing()
ADD_SUBDIRECTORY(test)

# Header installation
INSTALL(
    DIRECTORY include/
    DESTINATION include
    COMPONENT dev
    )

INSTALL(
    PROGRAMS ${PROJECT_SOURCE_DIR}/autojsoncxx.py
    DESTINATION bin
    COMPONENT dev
    )

INSTALL(
    PROGRAMS ${PROJECT_SOURCE_DIR}/code_template
    DESTINATION share
    COMPONENT dev
    )

# Simplified debianization
if (UNIX)

    # Common information
    SET(CPACK_COMPONENTS_ALL dev)
    SET(CPACK_GENERATOR "TGZ;DEB")
    SET(CPACK_SOURCE_TGZ "ON")
    SET(CPACK_DEB_COMPONENT_INSTALL 1)

    SET(CPACK_DEBIAN_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "A header-only library and a code generator to automagically translate between JSON and C++ types")
    SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)


    # Debian-aware information
    EXEC_PROGRAM("/bin/date +%Y%m%d" OUTPUT_VARIABLE DATE)
    SET(CPACK_DEBIAN_PACKAGE_VERSION "~git${DATE}-1")
    SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Vitaly Isaev <vitalyisaev2@gmail.com>")
    SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")
    SET(CPACK_PACKAGE_FILE_NAME 
        "${CPACK_DEBIAN_PACKAGE_NAME}${CPACK_DEBIAN_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
    SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/netheril96/autojsoncxx")

endif(UNIX)
 
INCLUDE(CPack)
