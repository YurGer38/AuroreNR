cmake_minimum_required(VERSION 3.16)
project(AuroreNRQt VERSION 7.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets Gui Core REQUIRED PrintSupport)
# If you use Qt5, change to find_package(Qt5 COMPONENTS Widgets Gui Core REQUIRED)

qt_standard_project_setup()

set(SOURCES
    main.cpp
    MainWindow.cpp
    SimulationWindow.cpp
    qcustomplot.cpp
)

set(HEADERS
    MainWindow.h
    SimulationWindow.h
    qcustomplot.h
    version.h
)

qt_add_resources(RESOURCES resources.qrc)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    version.h
    SimulationWindow.h
    SimulationWindow.cpp
)

target_link_libraries(${PROJECT_NAME}
    Qt6::Widgets
    Qt6::Gui
    Qt6::Core
    Qt6::PrintSupport
)
