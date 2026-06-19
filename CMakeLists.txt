cmake_minimum_required(VERSION 3.16)
project(appProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Qml)
find_package(Qt6 REQUIRED COMPONENTS Quick Qml QuickControls2)

if(POLICY QTP0001)
    qt_policy(SET QTP0001 OLD)
endif()

qt_add_executable(appProject
    main.cpp
    backend.h backend.cpp
    fileprocessor.h fileprocessor.cpp
)

qt_add_qml_module(appProject
    URI Project
    VERSION 1.0
    QML_FILES
        Main.qml
    SOURCES
        backend.cpp
        fileprocessor.cpp
        RESOURCES README.md
)

target_link_libraries(appProject PRIVATE Qt6::Quick Qt6::Qml)
target_link_libraries(appProject PRIVATE Qt6::Quick Qt6::Qml Qt6::QuickControls2)