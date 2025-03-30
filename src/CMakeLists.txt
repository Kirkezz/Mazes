cmake_minimum_required(VERSION 3.18)
project(Mazes VERSION 1.0 LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets LinguistTools)
find_package(Boost 1.40 COMPONENTS program_options REQUIRED)
find_package(SFML COMPONENTS system window graphics REQUIRED)
include(FetchContent) # todo: find_package support
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/external)
FetchContent_Declare(stk
  GIT_REPOSITORY https://github.com/thestk/stk
  GIT_TAG        master
)
FetchContent_MakeAvailable(stk)

set(PROJECT_SOURCES
        main.cpp
        utility.h
        spacegui.cpp
        spacegui.h
        spacegui.ui
        Space.cpp
        SpaceSound.h
        SpaceRenderer.cpp
        PointAnimation.cpp
        Point2D.h
        Space.h
        SpaceRenderer.h
        PointAnimation.h
        SpaceProfiling.h
        Demo.h
)
qt_add_resources(PROJECT_SOURCES res/assets.qrc)
qt_add_resources(PROJECT_SOURCES res/translation.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Mazes
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(Mazes
        ${PROJECT_SOURCES}
    )
endif()
# qt_add_translations(Mazes TS_FILES Mazes_EN.ts Mazes_EN.ts)

target_link_libraries(Mazes PRIVATE Qt${QT_VERSION_MAJOR}::Widgets PUBLIC sfml-window sfml-graphics Boost::program_options stk)

set_target_properties(Mazes PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS Mazes
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Mazes)
endif()
