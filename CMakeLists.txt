CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(autojsoncxx)

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
