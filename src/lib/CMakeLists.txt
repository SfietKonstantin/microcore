project(microcore)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_EXTRA_WARNINGS_FLAGS} ${CMAKE_CXX_COVERAGE_FLAGS}")

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)

set(${PROJECT_NAME}_CORE_SRCS
    core/globals.h
    core/ijob.h
    core/ijobfactory.h
    core/pipe.h
)

set(${PROJECT_NAME}_HTTP_SRCS
   http/httprequest.cpp
   http/httprequestfactory.cpp
)

set(${PROJECT_NAME}_JSON_SRCS
)

add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_CORE_SRCS}
    ${${PROJECT_NAME}_HTTP_SRCS}
)
qt5_use_modules(${PROJECT_NAME} Core Network)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}
)
