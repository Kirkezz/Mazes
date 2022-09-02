cmake_minimum_required(VERSION 3.5)

project(SpaceFillingCurves LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SOURCES main.cpp Space.cpp SpaceRenderer.cpp PointAnimation.cpp)
set(HEADERS Point2D.h Space.h SpaceRenderer.h PointAnimation.h)

find_package(SFML COMPONENTS window graphics REQUIRED)

add_executable(SpaceFillingCurves ${SOURCES} ${HEADERS})

target_link_libraries(SpaceFillingCurves PUBLIC sfml-window sfml-graphics)
