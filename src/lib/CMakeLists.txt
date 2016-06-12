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
    core/executor.h
)

set(${PROJECT_NAME}_DATA_SRCS
    data/type_helper.h
    data/idatastore.h
    data/iitem.h
    data/item.h
    data/datastore.h
    data/imodel.h
    data/imutablemodel.h
    data/model.h
)

set(${PROJECT_NAME}_QT_SRCS
    qt/qobjectptr.h
    qt/iviewmodel.h
    qt/viewmodel.h
    qt/viewcontroller.cpp
    qt/viewmodelcontroller.h
    qt/iviewitem.h
    qt/viewitem.h
    qt/viewitemcontroller.h
)

set(${PROJECT_NAME}_ERROR_SRCS
    error/error.cpp
)

set(${PROJECT_NAME}_HTTP_SRCS
    http/httptypes.h
    http/httprequest.cpp
    http/httprequestfactory.cpp
)

set(${PROJECT_NAME}_JSON_SRCS
    json/jsontypes.h
    json/jsonrequestfactory.cpp
)

add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_CORE_SRCS}
    ${${PROJECT_NAME}_DATA_SRCS}
    ${${PROJECT_NAME}_QT_SRCS}
    ${${PROJECT_NAME}_ERROR_SRCS}
    ${${PROJECT_NAME}_HTTP_SRCS}
    ${${PROJECT_NAME}_JSON_SRCS}
)
qt5_use_modules(${PROJECT_NAME} Core Network Qml)
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Network
    Qt5::Qml
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}
)
